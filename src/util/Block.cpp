//
// Created by zzyyy on 2017/4/15.
//

#include "../include/Block.h"

namespace sshkv{

	Block::Block(){
		valid_data_size = 0;
		state = FREE;
		number = block_total++;
	}


	Block::~Block(){}

	void Block::Reclaim(uint64_t value_size){
		valid_data_size -= value_size;
		if(state == VALID && value_size != 0) state = DIRTY;
		if(valid_data_size == 0) state = FREE;
	}

	void Block::SetState(BLOCK_STATE state_){
		state = state_;
	}

	void Block::SetSize(uint64_t size){
		valid_data_size = size;
	}

	uint64_t Block::block_total = 0;
}//namespace sshkv
