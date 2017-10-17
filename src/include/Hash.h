//
// Created by zzyyy on 2017/4/24.
//

#ifndef SSHKV_HASH_H
#define SSHKV_HASH_H

#include <cstdio>
#include "slice.h"

namespace sshkv{

	static size_t cryptTable[0x500];
	extern size_t hash(const Slice &key, size_t hash_type);
	extern void PrepareCryptTable();

}//namespace sshkv

#endif //SSHKV_HASH_H
