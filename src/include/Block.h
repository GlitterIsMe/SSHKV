//
// Created by zzyyy on 2017/4/15.
//

#ifndef SSHKV_BLOCK_H
#define SSHKV_BLOCK_H

#include <map>
#include "status.h"
#include <stdint.h>

namespace sshkv{


	class Block{
	public:
		Block();
		//Block(uint64_t LBA_, uint64_t size_);

		~Block();

		//invalid some space
		void Reclaim(uint64_t value_size);

		//return the total valid data size in this block
		inline uint64_t ValidData(void){
			return valid_data_size;
		}

		//return the state of this block
		inline BLOCK_STATE BlockState(){
			return state;
		}

		//Change the state of a block
		void SetState(BLOCK_STATE state_);

		//set the size of the block;
		void SetSize(uint64_t);

		inline uint64_t GetNumber(){
			return number;
		}

		static void ResetTotal(){
			block_total = 0;
		}

	private:
		uint64_t valid_data_size;
		BLOCK_STATE state;

		uint64_t number;
		static uint64_t block_total;


	};//class Block
}// namespace sshkv

#endif //SSHKV_BLOCK_H
