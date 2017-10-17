//
// Created by zzyyy on 2017/4/26.
//

#include <IORequest.h>

namespace sshkv{
	io_request::io_request(IO_TYPE type_, const Slice& key_, const Slice& value_, void(*func)(void*), void* arg): const_value(value_){
			type = type_;
			key = key_;
			//const_value = value_;
			completion = false;
			cb_func = func;
			cb_arg = arg;
	}

	io_request::io_request(IO_TYPE type_, const Slice& key_, std::string* value_, void(*func)(void*), void* arg){
			type = type_;
			key = key_;
			read_value = value_;
			completion = false;
			cb_func = func;
			cb_arg = arg;
	}

	io_request* io_request::NewWriteIORequest(const Slice& key, const Slice& value, void(*cb_func)(void*), void* cb_arg){
		return new io_request(KV_WRITE, key, value, cb_func, cb_arg);
	}

	io_request* io_request::NewReadIORequest(const Slice& key, std::string* value, void(*cb_func)(void*), void* cb_arg){
		return new io_request(KV_READ, key, value, cb_func, cb_arg);
	}

	io_request* io_request::NewDeleteIORequest(const Slice& key, void(*cb_func)(void*), void* cb_arg){
		return new io_request(KV_DELETE, key, Slice(), cb_func, cb_arg);
	}
}
