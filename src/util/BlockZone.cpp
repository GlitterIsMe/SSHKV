//
// Created by zzyyy on 2017/4/15.
//
#include <vector>
#include <assert.h>
#include <stdio.h>
#include <cstdlib>
#include "../include/BlockZone.h"
#include "../include/debug.h"

namespace sshkv{

	BlockZone::BlockZone(uint64_t block_num_, uint64_t block_size_/*, uint64_t LBA_*/)
			:  block_num(block_num_),
			   block_size(block_size_),
			   //start_LBA(LBA_),
			   valid_block_num(0),
               free_block_num(block_num),
			   block_now(0),
			   valid_data(0),
			   invalid_data(0),
			   in_GC(false){
		block_list_ = new Block[block_num];
		//Block::block_total = 0;
		//Block::ResetTotal();
	}

	BlockZone::~BlockZone(){
		delete[] block_list_;
	}

	STATUS BlockZone::AllocBlock(uint64_t& LBA){
		//printf("alloc block from zone\n");
		uint64_t pos = 0;
		for(int i = 0; i < block_num; i++){
			pos = block_now % block_num;
			//printf("block : %d is %d\n", pos, block_list_[pos].BlockState());
			if(block_list_[pos].BlockState() == FREE){
				SetState(pos, VALID);
				valid_data += block_size;
				block_now = (pos + 1) % block_num;
				valid_block_num++;
                free_block_num--;//free block drop one
				LBA = BlockOffset(pos);
				return SUCCESS;
			}
			block_now++;
		}
		return FULL;
	}

    STATUS BlockZone::AllocBlock(uint64_t LBA, bool flag) {
        uint64_t block_num_ = LBA / block_size;
        SetState(block_num_, VALID);
        valid_data += block_size;
        valid_block_num++;
        free_block_num--;
        DBGprint("free block %llu\n", free_block_num);
        return SUCCESS;
    }

	void BlockZone::FreeBlock(uint64_t LBA_){
		//assert(LBA_ <= block_num * block_size + 1);
        if (LBA_ > block_num * block_size){
            printf("%llu > %llu failed\n", LBA_, block_num * block_size);
            exit(-1);
        }
		uint64_t block_num_ = LBA_ / block_size;
		if(valid_block_num > 0){
			valid_block_num--;
		}
        free_block_num++;//free block num up one
        //printf("block %d has %llu bytes valid data and free\n", block_num_, block_list_[block_num_].ValidData());
		valid_data -= block_list_[block_num_].ValidData();

        //printf("%llu invalid data : %llu free %llu\n",block_num_, invalid_data, (block_size - block_list_[block_num_].ValidData()));

		//invalid_data -= (block_size - block_list_[block_num_].ValidData());
		SetState(block_num_, FREE);
	}

	void BlockZone::Reclaim(uint64_t LBA_, uint64_t value_size){
		assert(LBA_ + value_size <= block_num * block_size);
		uint64_t block_num_ = LBA_ / block_size;
		block_list_[block_num_].Reclaim(value_size);
        //printf("block %d has %llu bytes valid data and add %llu\n", block_num_, block_list_[block_num_].ValidData(), value_size);
		//invalid_data += value_size;
		if(block_list_[block_num_].BlockState() == FREE){
            //printf("%llu invalid data : %llu reclaim %llu\n",block_num_, invalid_data, block_size);
			//invalid_data -= block_size;
			valid_block_num--;
            free_block_num++;
		}
		valid_data -= value_size;
	}

	void BlockZone::SetState(uint64_t block_, BLOCK_STATE state_){
		switch (state_){
			case FREE:
				SetSize(block_, 0);
				break;

			case VALID:
				SetSize(block_, block_size);
				break;

			case DIRTY:;

			default:;

		}
		block_list_[block_].SetState(state_);
	}

	void BlockZone::GetBlockforGC(std::vector<Block*> &list) {
		for(uint64_t i = 0; i < block_num; i++){
			if (block_list_[i].BlockState() == DIRTY){
				list.push_back(&block_list_[i]);
			}
		}
	}

	void BlockZone::SetSize(uint64_t block_, uint64_t size){
		block_list_[block_].SetSize(size);
	}

	uint64_t BlockZone::FreeBlockNum() {
		uint64_t count = 0;
		for(uint64_t i = 0; i < block_num; i++){
			if (block_list_[i].BlockState() == FREE) count++;
		}
		return count;
	}

    uint64_t BlockZone::InvalidData() {
        DBGprint("free block num %llu   valid_data num %llu\n", free_block_num, valid_data);
        return block_num * block_size - free_block_num * block_size - valid_data;
    }

}//namespace sshkv

