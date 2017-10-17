//
// Created by zzyyy on 2017/4/16.
//


#include <string>
#include <fcntl.h>
#include <stdio.h>
#include "../include/Options.h"
#include "../include/DBImpl.h"
#include "../include/Hash.h"
#include "../include/SCMKeyTable.h"
#include "../include/SubSystem.h"
#include "../include/BGThread.h"
#include "../include/Options.h"
#include "../include/DB.h"


namespace sshkv{

	DBImpl::DBImpl(const std::string db_name_, const StartupOptions& options_)
			:   DB(),
			    subsystem_num(options_.max_subsystem_num),
				db_name(db_name_),
	            DISK(options_.DISK),
	            table(new SCMKey[options_.max_SCM_size]),
	            max_LBA_size(options_.max_LBA_size),
	            max_SCM_size(options_.max_SCM_size)
	            //bg_thread(new BGThread)
				{
					PrepareCryptTable();
					//printf("max LBA size : %lu\n", max_LBA_size);
					uint64_t LBA_size_per_system = max_LBA_size / subsystem_num;
					size_t SCM_size_per_system = max_SCM_size / subsystem_num;
					fd = open(DISK.c_str(), O_RDWR);
					if(fd == -1){
						printf("disk opend failed\n");
					}
					for(size_t i = 0; i < subsystem_num; i++){
						Options opt;
						uint64_t LBA_start = i * LBA_size_per_system;
						size_t SCM_start = i * SCM_size_per_system;
						opt.max_LBA = LBA_size_per_system;
						//printf("LBA_start : %lu, LBA_size : %lu\n", LBA_start, LBA_size_per_system);
						SubSystem* ss = new SubSystem(LBA_start, LBA_size_per_system, (table + i * SCM_size_per_system), SCM_start, SCM_size_per_system, fd, opt);
						//SubSystem* ss1 = new SubSystem(LBA_start, LBA_size_per_system, table+i*SCM_size_per_system, SCM_start, SCM_size_per_system, fd, opt);
						subsystem_list.push_back(ss);
					}

	}

	DB::~DB() {}

	DBImpl::~DBImpl() {
		for(size_t i = 0; i < subsystem_num; i++){
			subsystem_list[i]->ShutDown();
		}
		for(size_t i = 0; i < subsystem_num; i++){
			delete subsystem_list[i];
		}
		delete[] table;
		//delete bg_thread;
	}

	void DBImpl::Submit(io_request *req, size_t subsystem_no) {
		//printf("submit to subsystem %ld\n", subsystem_no);
		subsystem_list[subsystem_no]->Submit(req);
	}

	STATUS DBImpl::Put(const Slice &key, const Slice &value, void(*cb_func)(void*), void* cb_arg) {
		//if (total_size() > Physical_size){ return FULL;}
		size_t HASH = hash(key, 0);
		size_t subsystem_no = HASH % subsystem_num;
		io_request* request = io_request::NewWriteIORequest(key, value, cb_func, cb_arg);
		Submit(request, subsystem_no);
		return SUCCESS;
	}

	STATUS DBImpl::Get(const Slice &key, std::string *value, void(*cb_func)(void*), void* cb_arg) {
		size_t HASH = hash(key, 0);
		size_t subsystem_no = HASH % subsystem_num;
		io_request* request = io_request::NewReadIORequest(key, value, cb_func, cb_arg);
		Submit(request, subsystem_no);
		return SUCCESS;
	}

	STATUS  DBImpl::Delete(const Slice &key, void(*cb_func)(void*), void* cb_arg) {
		size_t HASH = hash(key, 0);
		size_t subsystem_no = HASH % subsystem_num;
		io_request* request = io_request::NewDeleteIORequest(key, cb_func, cb_arg);
		Submit(request, subsystem_no);
		return SUCCESS;
	}

	STATUS DBImpl::Open(const std::string &name, DB **dbptr) {
		StartupOptions opt;
		(*dbptr) = new DBImpl(name, opt);
		if((*dbptr)!= nullptr){
			return SUCCESS;
		}else return FAILED;
	}

	uint64_t DBImpl::total_size() {
		uint64_t size = 0;
		for(int i = 0; i < subsystem_num; i++){
			size += subsystem_list[i]->real_data_size();
		}
		return size;
	}


}//namespace sshkv

