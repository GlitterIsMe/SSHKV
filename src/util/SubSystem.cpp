//
// Created by zzyyy on 2017/4/19.
//

#include <cstdio>
#include <fcntl.h>
#include <zconf.h>
#include <sys/time.h>
#include <atomic>
#include "../include/SubSystem.h"
#include "../include/SCMKeyTable.h"
#include "../include/SSDAllocator.h"
#include "../include/Keymap.h"
#include "../include/WriteBuffer.h"
#include "../include/DataWriter.h"
#include "../include/DataReader.h"
#include "../include/BGThread.h"



namespace sshkv{

    struct GCStatus{
        uint64_t total_read;
        uint64_t total_write;
        uint64_t relaim_space;
        uint64_t start_micro;
        uint64_t end_micro;
        static uint64_t GC_times;

        GCStatus() {
            total_read = total_write = 0;
            relaim_space = 0;
            start_micro = end_micro =0;
            GC_times++;
        }

        void start_time(){
            struct timeval tv;
            gettimeofday(&tv, nullptr);
            start_micro = static_cast<uint64_t>(tv.tv_sec) * 1000000 + tv.tv_usec;
        }

        void end_time(){
            struct timeval tv;
            gettimeofday(&tv, nullptr);
            end_micro = static_cast<uint64_t>(tv.tv_sec) * 1000000 + tv.tv_usec;
        }
    };

    uint64_t GCStatus::GC_times = 0;

	struct GCUnit{
		BlockZone* zone;
		DataReader* reader;
		DataWriter* writer;
		WriteBuffer* buffer, *pending_buffer;
		std::vector<Block*> block_list;
        std::vector<int> pending_keymap;
        Block* block_now;
        bool been_alloc;
        //bool done;
        GCStatus status;
        //int pending_zone;

        GCUnit();
	};

    GCUnit::GCUnit() {
        zone = nullptr;
        reader = nullptr;
        writer = nullptr;
        buffer = nullptr;
        pending_buffer = nullptr;
        block_list.clear();
        block_now = nullptr;
        been_alloc = false;
       // pending_zone = 0;
        //done = false;
    }



    int SubSystem::number = 0;

	STATUS SubSystem::DoWrite(io_request *request) {
		//printf("%d Dowrite\n", no);
		size_t hash_addr;
		STATUS s;
		if(key_table->GetKey(request->key, hash_addr) == SUCCESS){
            //FILE* fp = fopen("/home/zzyyyww/GraduationProject/update.txt", "a");
            //fprintf(fp,"%s = %lu\n", request->key.data(), hash_addr);
            //fclose(fp);
			//Find in SCM, Update data of the key
			//printf("%s hash addr = %ld\n",request->key.data(), hash_addr);
			s = Update(hash_addr, request->const_value);
			if (s == SUCCESS){
				//request->cb_func(request->cb_arg);
				request->completion = true;
			}
		} else {
			//printf("%d not found and write\n", no);
			//the key is not in the SCM, write it
			size_t hash_addr;
            //FILE* fp = fopen("/home/zzyyyww/GraduationProject/key_hash.txt", "a");
            //fprintf(fp,"%s = ", request->key.data());
            //fclose(fp);
			key_table->AllocKey(request->key, hash_addr, request->const_value.size());
			//printf("%s hash addr = %ld\n",request->key.data(), hash_addr);
            //printf("end AllocKey\n");
            //fprintf(fp," %lu\n", hash_addr);
            //fclose(fp);

			s = Write(hash_addr, request->const_value);
            //printf("%d end Write\n", no);
			if (s == SUCCESS){
				//request->cb_func(request->cb_arg);
				request->completion = true;
			}
			//printf("end do write\n");
		}
		//printf("end write\n");
		//printf("excute cb function : %s\n", request->key.data());
		(*(request->cb_func))(request);
        //printf("end cd call\n");
		return s;
	}

	STATUS SubSystem::DoRead(io_request *request) {
		size_t hash_addr;
		//not found
		if(key_table->GetKey(request->key, hash_addr) == NOTFOUND){
            //FILE* fp = fopen("/home/zzyyyww/GraduationProject/not_found_key.txt", "a");
            //fprintf(fp,"%s\n", request->key.data());
            //fclose(fp);
			request->completion = false;
			request->cb_func(request);
			return NOTFOUND;
		}else{
			SCMKey& key = (*key_table)[hash_addr];
			if(key.in_buffer){
				//found in buffer
				std::string result;
				STATUS s = active_buffer->Get(hash_addr, request->read_value);
				if(s == SUCCESS){
					//request->u.read_value = Slice(result);
					request->completion = true;
					//return s;
				}
				(*(request->cb_func))(request);
				return s;
			}else{
				//found in ssd
				ReadOptions read_options;
				read_options.size = key.value_size;
				read_options.LBA = key.LBA;
				DataReader* reader = DataReader::NewDataReader(DISK);
				uint64_t read_size;
				char* data;
				STATUS s = reader->DoRead(read_options, &data, read_size);
				delete reader;
				if(s == SUCCESS){
					request->read_value->assign(data);
					delete[] data;
					request->completion = true;
					//return s;
				}
				(*(request->cb_func))(request);
				return s;
			}
		}
	}

	STATUS SubSystem::DoDelete(io_request *request) {
		size_t hash_addr;
		if(key_table->GetKey(request->key, hash_addr) == NOTFOUND){
			return NOTFOUND;
		}else{
			SCMKey key = (*key_table)[hash_addr];
			key.valid = false;
			if(key.in_buffer){
				//found in buffer
				std::string result;
				STATUS s = active_buffer->Delete(hash_addr);
				if (s == SUCCESS) {
					request->completion = true;
				}
				(*(request->cb_func))(request);
				return s;
			}else{
				//found in ssd
				keymap->DeleteKey(hash_addr, LBAtoBlockno(key.LBA));
				allocator->Reclaim(key.LBA, key.value_size);
				(*(request->cb_func))(request);
				request->completion = true;
				return SUCCESS;
			}
		}
	}

	STATUS SubSystem::Update(size_t hash_addr, const Slice &value) {
		SCMKey& key = (*key_table)[hash_addr];
        //bool in_buffer = (*key_table)[hash_addr].in_buffer;
		//find in buffer
		if (key.in_buffer){
			//keymap->DeleteKey(hash_addr, LBAtoBlockno(active_buffer->StartLBA()));
			//keymap->AddKey(hash_addr, value.size(), LBAtoBlockno(active_buffer->StartLBA()));
			return active_buffer->Update(hash_addr, value);
		}else{
			//the value is on the SSD, delete old data in keymap and do write operation
            //printf("uodate in ssd\n");
			uint64_t addr = key.LBA;
			keymap->DeleteKey(hash_addr, LBAtoBlockno(addr));
			allocator->Reclaim(addr, key.value_size);
			return Write(hash_addr, value);
		}

	}

	STATUS SubSystem::Write(size_t hash_addr, const Slice& value) {
		//printf("Write\n");
		if (active_buffer == NULL){
			active_buffer = NewWriteBuffer(options.max_block_size);
		}
        if (active_buffer->DataSize() + value.size() > options.max_block_size && bgWriteWorking){
            while(bgWriteWorking){}
        }
        if(active_buffer->DataSize() + value.size() > options.max_block_size && !bgWriteWorking){
			mutable_buffer = active_buffer;
			active_buffer = NewWriteBuffer(options.max_block_size);
            //printf("end New buffer\n");
            bgWriteWorking = true;
			bg_thread->Schedule(&SubSystem::BGWork_W, this);
            //DoPersist();
		}
		//Do not update the key map here, do it at Persist
		//keymap->AddKey(hash_addr, value.size(), LBAtoBlockno(active_buffer->StartLBA()));
		//(*key_table)[hash_addr].in_buffer = true;
		//printf("end Write\n");
		return active_buffer->Put(hash_addr, value);
	}

    uint64_t SubSystem::LBAtoBlockno(uint64_t LBA_) {
		return (LBA_ - LBA_start) / options.max_block_size;
	}

	void SubSystem::BGWork_W(void* arg){

		reinterpret_cast<SubSystem*>(arg)->DoPersist();

	}

	void SubSystem::DoPersist() {
        //printf("subsystem do persist\n");
        if(mutable_buffer == nullptr){
            //printf("null buffer\n");
            return;
        }
		std::map<size_t , uint64_t> key;
		char* data = nullptr;
        //int fd = open("/home/zzyyyww/GraduationProject/writelog", O_RDWR);
        //write down data
        WriteBuffer* immbuffer = mutable_buffer;
		uint64_t size = immbuffer->Persist(&data, key);
        //write(fd, data, size);
		DataWriter* writer = NewDataWriter(DISK, immbuffer->StartLBA(), Slice(data, options.max_block_size));
		STATUS s = writer->DoWrite();
        //reclaim space unused
		allocator->Reclaim(immbuffer->StartLBA() + size - 1, options.max_block_size - size);
        //update the keymap
		if (s){
			for(std::map<size_t , uint64_t >::iterator iter = key.begin(); iter != key.end(); iter++){
				//update keymap here
				keymap->AddKey(iter->first, (*key_table)[iter->first].value_size, LBAtoBlockno(immbuffer->StartLBA()));
				//update the LBA
                char key[100];
                sprintf(key, "%16d\n", iter->first);
               // write(fd, key, 100);

				(*key_table)[iter->first].LBA = iter->second;
				(*key_table)[iter->first].in_buffer = false;
			}
            //printf("end update keymap\n");
			delete immbuffer;
			mutable_buffer = nullptr;
			delete writer;
		}
        //close(fd);
		MaybeScheduleGC();
        bgWriteWorking = false;
        //printf("end persist\n");
	}

	WriteBuffer* SubSystem::NewWriteBuffer(uint64_t buffer_size) {
		uint64_t LBA;
		STATUS s = allocator->AllocBlock(LBA);
		if (s == SUCCESS){
			return new WriteBuffer(buffer_size, LBA);
		}else{
            printf("failed to new a buffer status : %d\n", s);
			//MaybeScheduleGC();
			return nullptr;
		}

	}

    WriteBuffer* SubSystem::NewWriteBuffer(GCUnit* unit, uint64_t buffer_size) {
        uint64_t LBA;
        //STATUS s = unit->zone->AllocBlock(LBA);
        //printf("alloc form block %d directly\n", unit->block_now->GetNumber());
        LBA = unit->block_now->GetNumber() * options.max_block_size + LBA_start;
        //unit->zone->FreeBlock(LBA);
        allocator->FreeBlock(LBA);
        printf("invalid data size %llu\n", unit->zone->InvalidData());
        unit->zone->AllocBlock(LBA - LBA_start, true);
	    //unit->block_now->SetState(VALID);
        unit->been_alloc = true;
        //printf("new buffer at %llu\n", LBA);
        return new WriteBuffer(buffer_size, LBA);
        /*if (s == SUCCESS){
            return new WriteBuffer(buffer_size, LBA);
        }else{
            printf("failed to new a buffer from this zone status : %d\n", s);
            return nullptr;
        }*/

    }

	DataWriter* SubSystem::NewDataWriter(int fd_, uint64_t LBA_, const Slice &data_) {
		return new DataWriter(fd_, LBA_, data_);
	}

	void SubSystem::MaybeScheduleGC() {

		//to ask ssd allocator is there need for GC
		//when the space was used over 80%, we active the GC thread
		bool need_gc = allocator->MaybeNeedGC();
		if (need_gc && !GCworking){
			//send task to background thread
            printf("need GC\n");
			bg_thread->Schedule(&SubSystem::BGWork_GC, this);
		}
	}

	void SubSystem::BGWork_GC(void* subsystem_){
		reinterpret_cast<SubSystem*>(subsystem_)->PrepareforGC();
	}

	void SubSystem::PrepareforGC(){
		GCUnit* unit = new GCUnit;
		unit->zone = allocator->FindZoneforGC();
		unit->zone->GetBlockforGC(unit->block_list);

		DoGarbageCollection(unit);
        printf("invalid data size %llu\n", unit->zone->InvalidData());
	FILE* fp = fopen("/home/zhangyiwen/SSHKV/GC.log", "a");
        printf("GC reads %.2f MBs\n", (double)unit->status.total_read / (1024 * 1024));
        printf("GC writes %.2f MBs\n", (double)unit->status.total_write / (1024 * 1024));
        printf("GC %llu times\n", unit->status.GC_times);
        printf("GC processing time : %llu micros\n", unit->status.end_micro - unit->status.start_micro);
        getchar();
	delete unit;
	}

	void SubSystem::DoGarbageCollection(GCUnit* unit) {
        GCworking = true;
        unit->status.start_time();
		//GCUnit* unit = reinterpret_cast<GCUnit*>(unit_);
        //printf("now processing GC\n");
        //STATUS  s;
        printf("GC block num : %d\n", unit->block_list.size());
        //for(size_t i = 0; i < unit->block_list.size(); i++){
            //printf("block : %d---valid data size : %llu---state : %d\n", unit->block_list[i]->GetNumber(), unit->block_list[i]->ValidData(), unit->block_list[i]->BlockState());
        //}

        unit->reader = DataReader::NewDataReader(DISK);


        uint64_t size = 0;


		for(std::vector<Block*>::iterator iter = unit->block_list.begin(); iter != unit->block_list.end(); iter++){
			//get all key of this block
			std::vector<size_t> valid_key;
            valid_key.clear();
            unit->block_now = (*iter);
            unit->been_alloc = false;

			keymap->GetValidKey(valid_key, (*iter)->GetNumber());
			std::vector<size_t>::iterator iiter = valid_key.begin();
			//find these key in scm and mark as in-buffer and read value to memory
			//write into buffer
			for(; iiter!= valid_key.end(); iiter++){
				SCMKey& tmp = (*key_table)[*iiter];

                char* reasult;
				//tmp.in_buffer = true;

                ReadOptions readop;
                readop.LBA = tmp.LBA;
                readop.size = tmp.value_size;

                unit->reader->DoRead(readop, &reasult, size);
                unit->status.total_read += size;

				if (unit->buffer == nullptr){
                    //printf("nullptr alloc\n");
					unit->buffer = NewWriteBuffer(unit, options.max_block_size);
				}else if (unit->buffer->DataSize() + tmp.value_size > unit->buffer->BufferSize()){
					//start a new buffer
					//write data to SSD
                    //while(unit->pending_buffer != nullptr) {/*wait for write*/}
					unit->pending_buffer = unit->buffer;
					//unit->buffer = NewWriteBuffer(options.max_block_size);
                    //printf("full alloc\n");
                    unit->buffer = NewWriteBuffer(unit, options.max_block_size);
                    /*if(unit->buffer == nullptr){
                        unit->buffer = NewWriteBuffer(options.max_block_size);
                    }*/
					char* data = nullptr;
					std::map<size_t , uint64_t > key_data;
					uint64_t real_size = unit->pending_buffer->Persist(&data, key_data);
					unit->writer = NewDataWriter(DISK, unit->pending_buffer->StartLBA(), Slice(data, unit->pending_buffer->BufferSize()));
                    STATUS s = unit->writer->DoWrite();

                    unit->status.total_write += unit->pending_buffer->DataSize();

					allocator->Reclaim(unit->pending_buffer->StartLBA() + real_size - 1, options.max_block_size - real_size);
					if (s){
                        //DoCleanUp(unit);
                        BlockUnit new_unit;
						for(std::map<size_t , uint64_t >::iterator iiiter = key_data.begin(); iiiter != key_data.end(); iiiter++){
                            //printf("add %lu to block %d\n", iiiter->first, LBAtoBlockno(unit->pending_buffer->StartLBA()));
                            //keymap->AddKey(iiiter->first, (*key_table)[iiiter->first].value_size, LBAtoBlockno(unit->pending_buffer->StartLBA()));
							//update the LBA
							(*key_table)[iiiter->first].LBA = iiiter->second;
                            //contian the old value
                            keymap->AddKey(iiiter->first, (*key_table)[iiiter->first].value_size, LBAtoBlockno(unit->pending_buffer->StartLBA()));
                            new_unit.AddKey(iiiter->first, (*key_table)[iiiter->first].value_size);
						}
                        DoCleanUp(unit);
                        keymap->ChangeUnit(new_unit, LBAtoBlockno(unit->pending_buffer->StartLBA()));

					}
					delete unit->writer;
					delete unit->pending_buffer;

					unit->writer = nullptr;
					unit->pending_buffer = nullptr;
				}

				unit->buffer->Put(*iiter, Slice(reasult, size));
			}
            //printf("end a block\n");
            unit->pending_keymap.push_back((*iter)->GetNumber());
            if(!unit->been_alloc){
                allocator->FreeBlock((*iter)->GetNumber() * options.max_block_size + LBA_start);
                //unit->zone->FreeBlock((*iter)->GetNumber() * options.max_block_size);
                //printf("free block %d\n", (*iter)->GetNumber());
                //printf("block state %d\n", (*iter)->BlockState());
                //mark this block as FREE
            }
		}
        //printf("close to end\n");
		if (unit->buffer != nullptr && unit->buffer->DataSize() > 0){
			char* data = nullptr;
			std::map<size_t , uint64_t > key_data;
			uint64_t real_size = unit->buffer->Persist(&data, key_data);
			unit->writer = NewDataWriter(DISK, unit->buffer->StartLBA(), Slice(data, unit->buffer->BufferSize()));

			STATUS s = unit->writer->DoWrite();
            unit->status.total_write += unit->buffer->DataSize();

			allocator->Reclaim(unit->buffer->StartLBA() + real_size - 1, options.max_block_size - real_size);
			if (s){
                //DoCleanUp(unit);
                BlockUnit new_unit;
                for(std::map<size_t , uint64_t >::iterator iiiter = key_data.begin(); iiiter != key_data.end(); iiiter++){
                    //printf("add %lu to block %d\n", iiiter->first, LBAtoBlockno(unit->buffer->StartLBA()));
                    //keymap->AddKey(iiiter->first, (*key_table)[iiiter->first].value_size, LBAtoBlockno(unit->pending_buffer->StartLBA()));
                    //update the LBA
                    (*key_table)[iiiter->first].LBA = iiiter->second;
                    //contian the old value
                    keymap->AddKey(iiiter->first, (*key_table)[iiiter->first].value_size, LBAtoBlockno(unit->buffer->StartLBA()));
                    new_unit.AddKey(iiiter->first, (*key_table)[iiiter->first].value_size);
                }
                DoCleanUp(unit);
                keymap->ChangeUnit(new_unit, LBAtoBlockno(unit->buffer->StartLBA()));

			}
            delete unit->writer;
            unit->writer = nullptr;

		}else if(unit->buffer != nullptr && unit->buffer->DataSize() == 0){
            allocator->FreeBlock(unit->buffer->StartLBA());
            keymap->CleanUp(LBAtoBlockno(unit->buffer->StartLBA()));
        }
        delete unit->buffer;
        if(unit->writer != nullptr) delete unit->writer;
        //if(unit->reader != nullptr) delete unit->reader;
        delete unit->reader;

        //unlock this zone
        unit->zone->GC_unlock();

        //printf("end GC\n");
        unit->status.end_micro;
        GCworking = false;

	}

    void SubSystem::DoCleanUp(GCUnit *unit) {
        for(int i = 0; i < unit->pending_keymap.size(); i++){
            //printf("clean up block %d\n", unit->pending_keymap[i]);
            keymap->CleanUp(unit->pending_keymap[i]);
        }
        unit->pending_keymap.clear();
    }

	void SubSystem::Run() {
        //bool is_lock = false;
		run = true;
		while(run){

            std::unique_lock<std::mutex> lk(mu);
			//cv.wait(lk, Scheduled());
            while(request_queue.empty()){
		system_busy = false;
                if(!run) return ;
		//printf("wait for notifying\n");
                cv.wait(lk);
            }
            //printf("has been notified and to execute\n");
			/*while(request_queue.empty()){
                //printf("idle : %d\n", request_queue.size());
				system_busy = false;
				if (!run) return ;
			}*/
            //mu.lock();
			system_busy = true;
			io_request* request= request_queue.front();
			request_queue.pop();
			//printf("subsystem %d queue req num %d, pop %s\n", no, request_queue.size(), request->key.data());
			lk.unlock();
            //printf("start exe\n");
			ExecuteRequest(request);
            //printf("%d end exe\n", no);

		}
	}

	void SubSystem::RunWrapper(void* arg){
		reinterpret_cast<SubSystem*>(arg)->Run();
	}

	void SubSystem::ShutDown() {
		cv.notify_one();
		while(system_busy){}
		run = false;
	}

	STATUS SubSystem::ExecuteRequest(io_request *request) {
		//printf("execute request %s\n", request->key.data());
		STATUS s;
		switch(request->type) {
			case KV_READ:
				s = DoRead(request);
				break;

			case KV_WRITE:
				s = DoWrite(request);
				break;

			case KV_DELETE:
				s = DoDelete(request);
				break;
		}
        //printf("end execute\n");
		return s;
	}

	void SubSystem::Submit(io_request *request) {

		//mu.lock();
		std::lock_guard<std::mutex> lk(mu);
		if (!run){
			run = true;
			main_thread = new std::thread(std::bind(&SubSystem::RunWrapper, this));
		}
		request_queue.push(request);
		//mu.unlock();
		cv.notify_one();
        //printf("notify thread to execute\n");
	}

	uint64_t SubSystem::real_data_size() {
		return allocator->real_data_size();
	}

	SubSystem::SubSystem(uint64_t LBA_start_, uint64_t LBA_size_, SCMKey* table, size_t SCM_start_, size_t SCM_size_, int fd_, const Options& options_)
			:   options(options_),
			    LBA_start(LBA_start_),
			    SCM_start(SCM_start_),
			    LBA_size(LBA_size_),
			    SCM_size(SCM_size_),
				key_table(new SCMKeyTable(SCM_size, SCM_start, table)),
				allocator(new SSDAllocator(options, LBA_start_)),
				keymap(new Keymap(options.max_LBA / options.max_block_size)),
				bg_thread(new BGThread),
				active_buffer(nullptr),
				mutable_buffer(nullptr),
                main_thread(nullptr),
	            DISK(fd_),
	            system_busy(false),
	            run(false),
                GCworking(false),
                bgWriteWorking(false),
                //flag(ATOMIC_FLAG_INIT),
                no(number++)
				{
					/*Options opt;
					opt.max_LBA = LBA_size;
					allocator = new  SSDAllocator(opt,)*/
					printf("SCM_size : %lu\n", SCM_size_);
				//DISK = fd_;
	}

	SubSystem::~SubSystem() {
        delete bg_thread;
        while(system_busy){}
		if (main_thread->joinable()){
			main_thread->join();
		}
		delete main_thread;
		delete key_table;
		delete allocator;
		delete keymap;
		if (active_buffer != nullptr){
			delete active_buffer;
		}
		if (mutable_buffer != nullptr){
			delete mutable_buffer;
		}
	}


}//namespace sshkv
