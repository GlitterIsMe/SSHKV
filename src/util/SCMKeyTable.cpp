//
// Created by zzyyy on 2017/4/18.
//
#include <cstdio>
#include "../include/SCMKeyTable.h"
#include "../include/Hash.h"


namespace sshkv {

	/*static uint64_t Max_Key_Num(){
		return 20000000;
	}*/

    /*
     struct SCMKey{
		char* key;
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
	};
     */

	SCMKey::SCMKey() {
        //key = new char[100];
        memset(key, 0, 100);
        key_size = 0;
        value_size = 0;
        LBA = 0;

        valid = false;
		in_buffer = false;
		next_hash = 0;

		CHECK_A = CHECK_B = 0;
	}

	SCMKey::~SCMKey(){
		//printf("delete %s\n", key);
		//delete[] key;
	}

	SCMKey& SCMKey::operator=(const SCMKey &key_) {
		//do deep copy
		//SCMKey left_key;
		key_size = key_.key_size;
		value_size = key_.value_size;
		valid = key_.valid;
		CHECK_A = key_.CHECK_A;
		CHECK_B = key_.CHECK_B;
		LBA = key_.LBA;
		next_hash = key_.next_hash;
		in_buffer = key_.in_buffer;
		memcpy(key, key_.key, key_size);
		return *this;
	}

	SCMKeyTable::SCMKeyTable(size_t size, size_t base, SCMKey* table_)
			:   MAX_KEY_NUM(size),
			    BASE(base),
				current_(MAX_KEY_NUM / 2),
				direct_(current_),
				valid_key_num(0),
				table(table_){
		//printf("size:%lu  base:%lu\n", size, base);
		//printf("MAX_KEY_NUM:%lu  BASE:%lu  current:%lu  direct:%lu\n", MAX_KEY_NUM, BASE, current_, direct_);
	}

	SCMKeyTable::~SCMKeyTable(){
	}

	size_t SCMKeyTable::RealTableAddr(size_t hash){
		return hash % direct_;
	}

	STATUS SCMKeyTable::AllocKey(const Slice &key, size_t& result, uint64_t value_size) {
		size_t hash_off, hash_a, hash_b;
		hash_off = hash(key.data(), 0);
		hash_a = hash(key.data(), 1);
		hash_b = hash(key.data(), 2);
		size_t hash_addr = RealTableAddr(hash_off);

		if(table[hash_addr].valid){
			size_t last_hash = FindLastHash(hash_addr);
			hash_addr = AllocInClash();
			table[last_hash].next_hash = hash_addr + BASE;
		}

		//table[hash_addr].key = key;
		//char* key_data = new char[key.size()];
        //save the key data
		memcpy(table[hash_addr].key, key.data(), key.size());
        //save key size
        table[hash_addr].key_size = key.size();
		table[hash_addr].value_size = value_size;
		table[hash_addr].valid = true;
		table[hash_addr].in_buffer = true;
		table[hash_addr].CHECK_A = hash_a;
		table[hash_addr].CHECK_B = hash_b;

		result = hash_addr + BASE;
		valid_key_num++;
		return SUCCESS;
	}

	STATUS SCMKeyTable::GetKey(const Slice &key, size_t& result) {
        //printf("gey key\n");
		size_t hash_off, hash_a, hash_b;
		hash_off = hash(key.data(), 0);
		hash_a = hash(key.data(), 1);
		hash_b = hash(key.data(), 2);
		size_t hash_addr = RealTableAddr(hash_off);
        //printf("saved key hash a = %lu, hash b = %lu\n", hash(table[hash_addr].key, 1), hash(table[hash_addr].key, 2));



		//printf("check A = %lu, hash A = %lu\nchech B = %lu, hash B = %lu\n", table[hash_addr].CHECK_A, hash_a, table[hash_addr].CHECK_B, hash_b);
		//if(table[hash_addr].valid){
			//printf("valid\n");
		//}//else printf("invlaid\n");
		//if(saved_key == key){
		//	printf("%s same\n", key.data());
		//}else printf("%s %ld not same %s %ld\n", table[hash_addr].key, table[hash_addr].key_size, key.data(), key.size());

		do{
            Slice saved_key = Slice(table[hash_addr].key, table[hash_addr].key_size);

			if (table[hash_addr].CHECK_A == hash_a && table[hash_addr].CHECK_B == hash_b && saved_key == key && table[hash_addr].valid){
				result = hash_addr + BASE;
                //printf("end get key find\n");
				return SUCCESS;
			}
			else{
				size_t next = table[hash_addr].next_hash;

                //FILE* fp = fopen("/home/zzyyyww/GraduationProject/hashchain","a");
                //fprintf(fp, "%lu ---> %lu\n",hash_addr + BASE, next);
                //fprintf(fp, "check A = %lu, hash A = %lu\nchech B = %lu, hash B = %lu\n", table[hash_addr].CHECK_A, hash_a, table[hash_addr].CHECK_B, hash_b);
                //if(table[hash_addr].valid){
                //    fprintf(fp, "valid\n");
                //}else fprintf(fp, "invlaid\n");
                //if(saved_key == key){
                //	fprintf(fp, "%s same\n", key.data());
                //}else fprintf(fp, "%s %ld not same %s %ld\n", saved_key.data(), saved_key.size(), key.data(), key.size());

                hash_addr = (next == 0 ? next : next - BASE);
                //fprintf(fp, "next : %lu \n", hash_addr);
                //if(hash_addr != 0){
                //    fprintf(fp, "%s\n", table[hash_addr].key);
                //}
                //fclose(fp);
			}
		}while(hash_addr != 0);
        //printf("end get key not find\n");
		return NOTFOUND;
	}

	STATUS SCMKeyTable::DeleteKey(const Slice &key) {
		size_t hash_addr;
        size_t local_addr;
		if (GetKey(key, hash_addr) != NOTFOUND){
            local_addr = hash_addr - BASE;
            if(local_addr < direct_){
                table[local_addr].valid = false;
                table[local_addr].key_size = 0;
            }else{
                size_t last = FindLastHash(local_addr);
                table[last].next_hash = table[local_addr].next_hash;
                table[local_addr].next_hash = 0;
                table[local_addr].valid = false;
                table[local_addr].key_size = 0;
            }
		}
		else{
			return NOTFOUND;
		}
	}

	size_t SCMKeyTable::FindLastHash(size_t start) {
		size_t hash_addr = start;
		while(table[hash_addr].next_hash != 0){
			hash_addr = table[hash_addr].next_hash - BASE;
		};
		return hash_addr;
	}

	STATUS SCMKeyTable::SetLBA(size_t hash_addr, uint64_t LBA_) {
		table[hash_addr - BASE].LBA = LBA_;
	}

	SCMKey& SCMKeyTable::operator[](size_t hash_addr) {
		return table[hash_addr - BASE];
	}

    size_t SCMKeyTable::AllocInClash() {
        while(table[current_].valid){
            current_++;
            if(current_ == MAX_KEY_NUM) current_ = direct_;
        }
        return current_;
    }
}//namespace sshkv
