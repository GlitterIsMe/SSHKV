//
// Created by zzyyy on 2017/4/18.
//
#include <sys/types.h>
#include <zconf.h>
#include "../include/DataReader.h"


namespace sshkv{
	DataReader::DataReader(int fd_) : DISK(fd_) {
	}

	DataReader::~DataReader() {}

	STATUS DataReader::DoRead(const ReadOptions &options, char **result, uint64_t &size) {
		ssize_t r = 0;
		*result = new char[options.size];
		r = pread(DISK, *result, options.size, options.LBA);
		if (r == -1){
			return FAILED;
		}
		else{
			size = r;
			return SUCCESS;
		}
	}


}
