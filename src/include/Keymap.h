//
// Created by zzyyy on 2017/4/16.
//

#ifndef SSHKV_KEYMAP_H
#define SSHKV_KEYMAP_H

#include <map>
#include <stdint.h>

namespace sshkv{

	class BlockUnit{
	public:

		BlockUnit();

		~BlockUnit();

		//Add a key to this block
		void AddKey(size_t hash_addr, uint64_t value_size);

		//Delete a key from this block
		uint64_t DeleteKey(size_t hash_addr);

		//Return valid key num of this block
		int KeyNum(void);

		void GetValidKey(std::vector<size_t>& list);

		void CleanUp();

		BlockUnit& operator=(const BlockUnit& new_unit);

	private:
		std::map<size_t, uint64_t> key_;
		int key_num;
	};

	class Keymap{
	public:
		Keymap(uint64_t block_num_);

		~Keymap();
		//Add a key to block[block_no]
		void AddKey(size_t hash_addr, uint64_t value_size, uint64_t block_no);

		//Delete a key from block[block_no]
		uint64_t DeleteKey(size_t hash_addr, uint64_t block_no);

		void CleanUp(uint64_t block_no);

		//Return the valid key num of block[block_no]
		int ValidKeyNum(uint64_t block_no);

		void GetValidKey(std::vector<size_t>& list, uint64_t block_no);

		void ChangeUnit(BlockUnit new_, uint64_t block_num);

	private:
		BlockUnit* maplist;
	};
}//namespace sshkv

#endif //SSHKV_KEYMAP_H
