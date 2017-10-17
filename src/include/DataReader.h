//
// Created by zzyyy on 2017/4/17.
//

#ifndef SSHKV_DATAREADER_H
#define SSHKV_DATAREADER_H

#include <cstdint>
#include "Options.h"
#include "status.h"

namespace sshkv{

	class DataReader{
	public:
		DataReader(int fd_);
		~DataReader();

		STATUS DoRead(const ReadOptions& options_, char** result, uint64_t& size);

		static DataReader* NewDataReader(int fd_){
			return new DataReader(fd_);
		}

	private:
		//ReadOptions options;
		int DISK;
		//uint64_t LBA;
	};
}//namespace sshkv
#endif //SSHKV_DATAREADER_H
