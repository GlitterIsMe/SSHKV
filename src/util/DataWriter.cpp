//
// Created by zzyyy on 2017/4/18.
//

#include "../include/DataWriter.h"
#include <unistd.h>

namespace sshkv{
	DataWriter::DataWriter(int fd_, uint64_t LBA_, const Slice& data_)
			:   DISK(fd_),
	            LBA(LBA_),
	            data(data_) {

	}

	DataWriter::~DataWriter() {
		delete[] data.data();
	}

	STATUS DataWriter::DoWrite() {
		ssize_t r = pwrite(DISK, data.data(), data.size(), LBA);
		if (r == -1) {
		    return FAILED;
		}
		else{
			fsync(DISK);
			return SUCCESS;
		}
	}
}
