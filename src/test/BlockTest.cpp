//
// Created by zzyyy on 2017/4/17.
//

#include "Block.h"
#include <iostream>

using namespace sshkv;

const static uint64_t block_size = 1024 * 1024;

int main(){
	Block block;
	block.SetSize(block_size);
	block.SetState(FREE);
	std::cout<<"block valid data size : "<<block.ValidData()<<std::endl;
	std::cout<<"block state : "<<block.BlockState()<<std::endl;

	block.Reclaim(5 * 1024);
	std::cout<<"block valid data size : "<<block.ValidData()<<std::endl;
	std::cout<<"block state : "<<block.BlockState()<<std::endl;

	block.Reclaim(1019 * 1024);
	std::cout<<"block valid data size : "<<block.ValidData()<<std::endl;
	std::cout<<"block state : "<<block.BlockState()<<std::endl;

	return 0;
}

