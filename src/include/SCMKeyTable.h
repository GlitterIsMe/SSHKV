//
// Created by zzyyy on 2017/4/17.
//

#ifndef SSHKV_SCMKEYTABLE_H
#define SSHKV_SCMKEYTABLE_H

#include "slice.h"
#include "status.h"
#include <stdint.h>

namespace sshkv{

	struct SCMKey{
		char key[100];
        size_t key_size;
		uint64_t value_size;
        uint64_t LBA;

		bool valid;
		bool in_buffer;
        size_t next_hash;

		size_t CHECK_A;
		size_t CHECK_B;

		SCMKey();
		~SCMKey();

        SCMKey& operator=(const SCMKey& key);
	};


	class SCMKeyTable{
	public:
		SCMKeyTable(size_t size, size_t base, SCMKey* table_);
		~SCMKeyTable();

		//store a key to SCM and return the hash address
		STATUS AllocKey(const Slice& key, size_t& result, uint64_t value_size);

		//search a key in the hash table and return the hash address
		STATUS GetKey(const Slice& key, size_t& result);

		//mark a key as invalid
		STATUS DeleteKey(const Slice& key);

		//Set the LBA of a key
		STATUS SetLBA(size_t hash_addr, uint64_t LBA_);

		SCMKey& operator[](size_t hash_addr);


	private:
		const size_t MAX_KEY_NUM;
		const size_t BASE;

		size_t current_;
		size_t direct_;

		size_t valid_key_num;

		SCMKey* table;

		size_t RealTableAddr(size_t hash);

		size_t FindLastHash(size_t start);

        size_t AllocInClash();
	};

}//namespace sshkv

#endif //SSHKV_SCMKEYTABLE_H
