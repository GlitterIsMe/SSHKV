//
// Created by zzyyy on 2017/4/18.
//

#ifndef SSHKV_IOREQUEST_H
#define SSHKV_IOREQUEST_H

#include "slice.h"

namespace  sshkv{

	enum IO_TYPE{KV_READ, KV_WRITE, KV_DELETE};

	struct io_request{

		IO_TYPE type;
		Slice key;
		void(*cb_func)(void*);
		void* cb_arg;

		const Slice const_value;
		//Slice value;
		std::string* read_value;
		//Slice value;

		bool completion;
		//bool heap_alocated;

		io_request(IO_TYPE type_, const Slice& key_, const Slice& value_, void(*func)(void*), void* arg);

		io_request(IO_TYPE type_, const Slice& key_, std::string* value_, void(*func)(void*), void* arg);

		static io_request* NewWriteIORequest(const Slice& key, const Slice& value, void(*cb_func)(void*), void* cb_arg);

		static io_request* NewReadIORequest(const Slice& key, std::string* value, void(*cb_func)(void*), void* cb_arg);

		static io_request* NewDeleteIORequest(const Slice& key, void(*cb_func)(void*), void* cb_arg);
	};

}//namespace sshkv
#endif //SSHKV_IOREQUEST_H
