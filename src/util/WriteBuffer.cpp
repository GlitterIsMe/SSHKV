//
// Created by zzyyy on 2017/4/17.
//
#include <map>
#include "../include/WriteBuffer.h"

#include <string.h>

namespace sshkv{

	WriteBuffer::WriteBuffer(uint64_t size_, uint64_t LBA_)
			:   data_size(0),
				buffer_size(size_),
				LBA(LBA_){
		buffer.clear();
	}

	WriteBuffer::~WriteBuffer() {

	}

	STATUS WriteBuffer::Put(int hash_addr, const Slice &value) {
		buffer.insert(std::map<int, Slice>::value_type(hash_addr, value));
		data_size += value.size();
		assert(data_size < buffer_size);
		return SUCCESS;
	}

	STATUS WriteBuffer::Get(int hash_addr, std::string *result) {
		if(buffer.find(hash_addr) == buffer.end()) return FAILED;
		else{
			result->assign(buffer[hash_addr].data(), buffer[hash_addr].size());
			return SUCCESS;
		}
	}

	STATUS WriteBuffer::Delete(int hash_addr) {
		if(buffer.find(hash_addr) == buffer.end()) return FAILED;
		else{
			data_size -= buffer[hash_addr].size();
            delete[] buffer[hash_addr].data();
			buffer.erase(hash_addr);
			return SUCCESS;
		}
	}

	STATUS WriteBuffer::Update(int hash_addr, const Slice &value) {
		if (buffer.find(hash_addr) == buffer.end()){
            return FAILED;
		} else {
			data_size -= buffer[hash_addr].size();
            delete[] buffer[hash_addr].data();
			buffer[hash_addr] = value;
			data_size += value.size();
			assert(data_size < buffer_size);
			return SUCCESS;
		}
	}

	uint64_t WriteBuffer::Persist(char **result, std::map<size_t , uint64_t>& addr_map) {

        //printf("do persist\n");
		uint64_t addr = LBA;
		uint64_t real_size = 0;
		*result = new char[buffer_size];
		memset(*result, 0, buffer_size);
		char* src = *result;
		std::map<size_t, Slice>::const_iterator iter = buffer.begin();
		for(; iter != buffer.end(); iter++){

			addr_map.insert(std::map<size_t , uint64_t >::value_type(iter->first, addr));
			memcpy(src, iter->second.data(), iter->second.size());
            delete[] iter->second.data();

			src += iter->second.size();
			addr += iter->second.size();
			real_size += iter->second.size();
			//record the key hash address and the LBA
		}
		return real_size;
	}

}

