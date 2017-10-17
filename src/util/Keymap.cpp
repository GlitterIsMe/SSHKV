//
// Created by zzyyy on 2017/4/16.
//
#include <vector>
#include "../include/Keymap.h"

namespace sshkv{

	BlockUnit::BlockUnit():key_num(0){
		key_.clear();
	}

	BlockUnit::~BlockUnit() {}

	void BlockUnit::AddKey(size_t hash_addr, uint64_t value_size){
		key_.insert(std::map<size_t, uint64_t>::value_type(hash_addr, value_size));
		key_num++;
	}

	uint64_t BlockUnit::DeleteKey(size_t hash_addr){
		uint64_t size = key_[hash_addr];
		key_.erase(hash_addr);
		key_num--;
		return size;
	}

	int BlockUnit::KeyNum(){
		return key_num;
	}

	void BlockUnit::CleanUp() {
		key_.clear();
		key_num = 0;
	}

	BlockUnit& BlockUnit::operator=(const BlockUnit &new_unit) {
		key_ = new_unit.key_;
		key_num = new_unit.key_num;
		return *this;
	}

	Keymap::Keymap(uint64_t block_num_):maplist(new BlockUnit[block_num_]){

	}

	Keymap::~Keymap(){
		delete[] maplist;
	}

	void Keymap::AddKey(size_t hash_addr, uint64_t value_size, uint64_t block_no){
		maplist[block_no].AddKey(hash_addr, value_size);
	}

	uint64_t Keymap::DeleteKey(size_t hash_addr, uint64_t block_no){
		return maplist[block_no].DeleteKey(hash_addr);
	}

	int Keymap::ValidKeyNum(uint64_t block_no){
		return maplist[block_no].KeyNum();
	}

	void BlockUnit::GetValidKey(std::vector<size_t>& list) {
		std::map<size_t , uint64_t >::iterator iter = key_.begin();
		for(; iter != key_.end(); iter++){
			list.push_back(iter->first);
		}
	}

	void Keymap::GetValidKey(std::vector<size_t>& list, uint64_t block_no) {
		maplist[block_no].GetValidKey(list);
	}

	void Keymap::CleanUp(uint64_t block_no) {
		maplist[block_no].CleanUp();
	}

	void Keymap::ChangeUnit(BlockUnit new_, uint64_t block_num){
		//BlockUnit* old_ = maplist[block_num];
		maplist[block_num] = new_;
		//delete old_;
	}
}//namespace sshkv

