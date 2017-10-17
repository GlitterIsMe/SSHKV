//
// Created by zzyyy on 2017/4/17.
//

#ifndef SSHKV_WRITEBUFFER_H
#define SSHKV_WRITEBUFFER_H

#define NDEBUG

#include "slice.h"
#include <stdint.h>
#include <map>
#include <string>
#include <assert.h>
#include "status.h"

namespace sshkv{

	class WriteBuffer{
	public:

		WriteBuffer(uint64_t size_, uint64_t LBA_);

		~WriteBuffer();

		STATUS Put(int hash_addr, const Slice& value);

		STATUS Get(int hash_addr, std::string* result);

		STATUS Delete(int hash_addr);

		STATUS Update(int hash_addr, const Slice& value);

		uint64_t Persist(char **result, std::map<size_t, uint64_t>& addr_map);

		inline uint64_t BufferSize(){
			return buffer_size;
		}

		inline uint64_t DataSize(){
			return data_size;
		}

		inline uint64_t StartLBA(){
			return LBA;
		}

	private:
		std::map<size_t, Slice> buffer;
		uint64_t buffer_size;
		uint64_t LBA;
		uint64_t data_size;

	};

}//namespace sshkv

#endif //SSHKV_WRITEBUFFER_H
