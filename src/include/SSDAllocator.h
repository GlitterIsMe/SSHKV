//
// Created by zzyyy on 2017/4/16.
//

#ifndef SSHKV_SSDALLOCATOR_H
#define SSHKV_SSDALLOCATOR_H

#include "BlockZone.h"
#include "status.h"
#include "Options.h"
#include <stdint.h>
#include <vector>

namespace sshkv{
	class SSDAllocator{

	public:
		SSDAllocator(const Options& options, uint64_t start_LBA_);
		~SSDAllocator();

		//allocate a block and return the LBA of it
		STATUS AllocBlock(uint64_t& LBA_);

		//free a block space
		void FreeBlock(uint64_t LBA_);

		//invalid some space when delete a key-value
		void Reclaim(uint64_t LBA_, uint64_t size);

		//DB calls this function to find a zone for garbage collection
		BlockZone* FindZoneforGC();

		bool MaybeNeedGC();

		uint64_t real_data_size();

	private:

		//get the valid data size of zone[zone_no]
		uint64_t ValidDataSize(uint64_t zone_no);

		//move to the next zone for allocation
		STATUS NextZoneforAlloc();

		bool ZoneNeedGC(int zone);

		//allocate memory for zonelist
		void InitZone(uint64_t zone_num_);

		//calculate which zone the LBA belongs to
		uint64_t LBAtoZone(uint64_t LBA_);

		//transfer the LBA to zone block address
		uint64_t ZoneBlockAddr(uint64_t LBA_, uint64_t zone_no);

		//transfer the zone block address to LBA
		uint64_t toGlobalLBA(uint64_t localLBA, uint64_t zone_no);

		std::vector<BlockZone*> zonelist;
		const Options& options;
		uint64_t capacity;
		uint64_t block_size;
		uint64_t zone_num;
		uint64_t zone_now;
		uint64_t start_LBA;

	};
}//namespace sshkv

#endif //SSHKV_SSDALLOCATOR_H


