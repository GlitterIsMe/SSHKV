//
// Created by zzyyy on 2017/4/15.
//

#ifndef SSHKV_BLOCKZONE_H
#define SSHKV_BLOCKZONE_H

#include "Block.h"
#include "status.h"
#include <stdint.h>
#include <vector>

namespace sshkv{
	class BlockZone{
	public:
		BlockZone(uint64_t block_num_, uint64_t block_size_/*, uint64_t LBA_*/);
		~BlockZone();

		//Allocate a block from this zone
		STATUS AllocBlock(uint64_t& LBA_);

        STATUS AllocBlock(uint64_t LBA, bool flag);

		//Free a block in this zone
		void FreeBlock(uint64_t LBA_);

		//reclaim some space
		void Reclaim(uint64_t LBA_, uint64_t value_size);

		//return the free space size of the zone
		inline uint64_t UsableSize(){
			return block_size * block_num - valid_data;
		}

		//return the free block num of this zone
		inline uint64_t UsableBlockNum(){
			return block_num - valid_block_num;
		}

		void GetBlockforGC(std::vector<Block*>& list);

		uint64_t FreeBlockNum();

		inline void GC_lock(){
			in_GC = true;
		}

		inline void GC_unlock(){
			in_GC = false;
		}

        inline bool ProcessingGC(){
            return in_GC;
        }
	
	    uint64_t InvalidData();
	private:
		//set the state of a block and change total valid data size
		void SetState(uint64_t block_, BLOCK_STATE state_);

		//set the size of a block
		void SetSize(uint64_t block_, uint64_t size);

		//return the LBA of a block
		inline uint64_t BlockOffset(uint64_t block_no){
			return block_no * block_size;
		}

		Block* block_list_;

		uint64_t block_num;
		uint64_t block_size;
		//uint64_t start_LBA;

		uint64_t valid_block_num;
        uint64_t free_block_num;
		uint64_t block_now;

		uint64_t valid_data;
		uint64_t invalid_data;

		bool in_GC;


	};//class BlockZone
}//namespace sshkv

#endif //SSHKV_BLOCKZONE_H
