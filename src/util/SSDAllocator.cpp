//
// Created by zzyyy on 2017/4/16.
//
#include <cstdio>
#include "../include/SSDAllocator.h"

namespace sshkv{

	/*static uint64_t Max_Logical_Space(const Options* options){
		return options->max_LBA;
	}

	static uint64_t Max_Block_size(const Options* options){
		return options->max_block_size;
	}

	static uint64_t Max_Zone_Size(const Options* options){
		return options->max_zone_size;
	}*/

	static uint64_t Max_Zone_Num(const Options& options){
		//return Max_Logical_Space(options) / Max_Zone_Size(options);
		//printf("max_LBA = %llu, max_zone_size = %llu, zone num = %llu\n",options.max_LBA, options.max_zone_size,  options.max_LBA / options.max_zone_size);
		return options.max_LBA / options.max_zone_size;
	}

	static uint64_t Max_Block_Num(const Options& options){
		//return Max_Zone_Size(options) / Max_Block_size(options);
		return options.max_zone_size / options.max_block_size;
	}

	SSDAllocator::SSDAllocator(const Options& options_, uint64_t start_LBA_)
			:   options(options_),
			    capacity(options_.max_LBA),
			    block_size(options_.max_block_size),
			    zone_num(Max_Zone_Num(options_)),
			    zone_now(0),
				start_LBA(start_LBA_){
        //printf("initialize SSDAllocator\n");
		InitZone(zone_num);
		Block::ResetTotal();
	}

	SSDAllocator::~SSDAllocator(){
		for(int i = 0; i < zone_num; i++){
			delete zonelist[i];
		}
	}

	void SSDAllocator::InitZone(uint64_t zone_num_){
		uint64_t start_LBA = 0;
		for(int i = 0; i < zone_num_; i++){
			BlockZone* tmp = nullptr;
			tmp = new BlockZone(Max_Block_Num(options), block_size);
			zonelist.push_back(tmp);
			start_LBA += options.max_zone_size;
		}

	}

	STATUS SSDAllocator::AllocBlock(uint64_t& LBA_){
		uint64_t localLBA;
		STATUS  s;
		if(zonelist[zone_now]->UsableBlockNum() == 0) {
			s = NextZoneforAlloc();
			if (s != SUCCESS) return s;
		}
		s = zonelist[zone_now]->AllocBlock(localLBA);
		if( s == SUCCESS) {
			LBA_ = toGlobalLBA(localLBA, zone_now);
			return SUCCESS;
		}
		return FULL;
	}

	STATUS SSDAllocator::NextZoneforAlloc(){
		uint64_t zone_next = zone_now + 1;
		while(true){
			for(uint64_t i = 0; i < zone_num; i++){
				uint64_t zone_no = zone_next % zone_num;
				if(zonelist[zone_no]->UsableBlockNum() != 0 && !zonelist[zone_no]->ProcessingGC()){
					//while(zonelist[zone_no]->ProcessingGC()){/*wait*/}
					zone_now = zone_no;
					return SUCCESS;
				}
				zone_next++;
			}
		}
		//return FULL;
	}

	void SSDAllocator::FreeBlock(uint64_t LBA_){
        uint64_t local_LBA = LBA_ - start_LBA;
		uint64_t zone_no = LBAtoZone(local_LBA);
		zonelist[zone_no]->FreeBlock(ZoneBlockAddr(local_LBA, zone_no));
	}

	void SSDAllocator::Reclaim(uint64_t LBA_, uint64_t size){
        uint64_t local_LBA = LBA_ - start_LBA;
		uint64_t zone_no = LBAtoZone(local_LBA);
		zonelist[zone_no]->Reclaim(ZoneBlockAddr(local_LBA, zone_no), size);
	}

	bool SSDAllocator::ZoneNeedGC(int zone) {
        BlockZone* p= zonelist[zone];
		if (zone == zone_now){
			return false;
		}else if(p->UsableBlockNum() == Max_Block_Num(options) || p->InvalidData() == 0){
            //this zone has not been used
			//there is no invalid data in this zone
            return false;
        }
        return true;
    }

	BlockZone* SSDAllocator::FindZoneforGC(){
		if (zone_num == 1){
			zonelist[zone_now]->GC_lock();//pending = zone_now;
			return zonelist[zone_now];
		}
		uint64_t max_size = 0;
		int pending_zone_no = 0;
		for(int i = 0; i < zone_num; i++){
			if(zonelist[i]->InvalidData() > max_size && ZoneNeedGC(i)){
				max_size = zonelist[i]->InvalidData();
				pending_zone_no = i;
			}
		}
        zonelist[pending_zone_no]->GC_lock();
		printf("return zone %d for GC\n", pending_zone_no);
		//getchar();
        //pending = pending_zone_no;
		return zonelist[pending_zone_no];
	}

	inline uint64_t SSDAllocator::LBAtoZone(uint64_t LBA_){
		return LBA_ / options.max_zone_size;
	}

	uint64_t SSDAllocator::ZoneBlockAddr(uint64_t LBA_, uint64_t zone_no){
		return LBA_ - zone_no * options.max_zone_size;
	}

	uint64_t SSDAllocator::toGlobalLBA(uint64_t localLBA, uint64_t zone_no){
		return localLBA + zone_no * (options.max_zone_size) + start_LBA;
	}

	bool SSDAllocator::MaybeNeedGC() {
		uint64_t total_free_size = 0;
		bool has_dirty = false;
		for(size_t i = 0; i < zone_num; i++){
			BlockZone* p = zonelist[i];
			total_free_size += p->FreeBlockNum() * options.max_block_size;
			printf("Zone %d invalid data %llu\n", i, p->InvalidData());
			if(p->InvalidData() != 0) has_dirty = true;
		}
		double usage_ratio = 1 - (double)total_free_size / capacity;
		printf("free : %llu, capacity %llu , %.2lf\n", total_free_size, capacity, usage_ratio);
		if (usage_ratio > 0.8 && has_dirty) return true;
		else return false;
	}

	uint64_t SSDAllocator::real_data_size() {
        uint64_t total_size = 0;
        for(int i = 0; i < zone_num; i++){
            total_size += zonelist[i]->UsableSize();
        }
        return total_size;
    }




}//namespace sshkv

