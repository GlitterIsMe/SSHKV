//
// Created by zzyyy on 2017/4/24.
//
#include "../include/slice.h"
#include "../include/Hash.h"

namespace sshkv{

	extern size_t cryptTable[0x500];

	void PrepareCryptTable() {
		size_t seed = 0x00100001, index1 = 0, index2 = 0, i;
		for(index1 = 0; index1 < 0x100; index1++){
			for(index2 = index1, i = 0; i < 5; i++, index2 += 0x100){
				size_t temp1, temp2;

				seed = (seed * 125 + 3) % 0x2AAAAB;
				temp1 = (seed & 0xFFFF) << 0x10;

				seed = (seed * 125 + 3) % 0x2AAAAB;
				temp2 = (seed & 0xFFFF);

				cryptTable[index2] = (temp1 | temp2);
			}
		}
	}

	size_t hash(const Slice &key, size_t hash_type) {
		const char* key_tmp = key.data();
		size_t seed1 = 0x7FED7FED;
		size_t seed2 = 0x7EEEEEEE;
		int ch;

		while(*key_tmp != 0){
			ch = toupper(*key_tmp++);

			seed1 = cryptTable[(hash_type << 8) + ch]^(seed1 + seed2);
			seed2 = ch + seed1 + seed2 + (seed2 << 5) + 3;
		}

		return seed1;

	}
}//namespace sshkv

