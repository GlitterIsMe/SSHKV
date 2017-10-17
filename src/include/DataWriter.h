//
// Created by zzyyy on 2017/4/17.
//

#ifndef SSHKV_DATAWRITER_H
#define SSHKV_DATAWRITER_H

#include <stdint.h>
#include "status.h"
#include "slice.h"

namespace sshkv{

	class DataWriter{
	public:
		DataWriter(int fd_, uint64_t LBA_, const Slice& data_);
		~DataWriter();
		STATUS DoWrite();

	private:
		uint64_t LBA;
		int DISK;
		Slice data;
	};

}//namespace sshkv
#endif //SSHKV_DATAWRITER_H
