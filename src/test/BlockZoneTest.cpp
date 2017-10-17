//
// Created by zzyyy on 2017/4/24.
//
#include <iostream>
#include "../include/BlockZone.h"

using namespace sshkv;

static uint64_t block_num = 20;
static uint64_t block_size = 1024*1024;

void StatusOut(STATUS s){
	switch (s){
		case ERROR:
			std::cout<<"ERROR"<<std::endl;
			break;
		case SUCCESS:
			std::cout<<"SUCCESS"<<std::endl;
			break;
		case FAILED:
			std::cout<<"FAILED"<<std::endl;
			break;
		case FULL:
			std::cout<<"FULL"<<std::endl;
			break;
		case NOTFOUND:
			std::cout<<"NOTFOUND"<<std::endl;
			break;
	}
}

int main(){
	BlockZone zone(block_num, block_size);
	uint64_t LBA;
	std::cout<<"usable block num "<<zone.UsableBlockNum()<<std::endl;
	std::cout<<"usabel size "<<zone.UsableSize() / 1024 / 1024<<"M"<<std::endl;

	for(int i = 0; i < 20; i++){
		//uint64_t LBA;
		StatusOut(zone.AllocBlock(LBA));
		std::cout<<"LBA : "<<LBA<<std::endl;
	}


	std::cout<<"usable block num "<<zone.UsableBlockNum()<<std::endl;
	std::cout<<"usabel size "<<zone.UsableSize() / 1024 / 1024<<"M"<<std::endl;

	StatusOut(zone.AllocBlock(LBA));


	zone.FreeBlock(2097152);

	//uint64_t LBA;
	StatusOut(zone.AllocBlock(LBA));
	std::cout<<"LBA : "<<LBA<<std::endl;

	std::cout<<"usable block num "<<zone.UsableBlockNum()<<std::endl;
	std::cout<<"usabel size "<<zone.UsableSize() / 1024 / 1024<<"M"<<std::endl;

	StatusOut(zone.AllocBlock(LBA));
	std::cout<<"usable block num "<<zone.UsableBlockNum()<<std::endl;
	std::cout<<"usabel size "<<zone.UsableSize() / 1024 / 1024<<"M"<<std::endl;

	zone.Reclaim(3145728, 2*4*1024);
	zone.Reclaim(4194304, 3*4*1024);
	std::cout<<"after reclaim"<<std::endl;
	std::cout<<"usable block num "<<zone.UsableBlockNum()<<std::endl;
	std::cout<<"usabel size "<<zone.UsableSize()<<"bytes"<<std::endl;

	std::vector<Block*> list;
	zone.GetBlockforGC(list);

	for(std::vector<Block*>::iterator iter = list.begin(); iter!= list.end(); iter++){
		std::cout<<(*iter)->BlockState()<<"  "<<(*iter)->GetNumber()<<"  "<<(*iter)->ValidData()<<std::endl;
	}
	return 0;
}


