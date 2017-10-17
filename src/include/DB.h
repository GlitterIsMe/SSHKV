//
// Created by zzyyy on 2017/4/16.
//

#ifndef SSHKV_DB_H
#define SSHKV_DB_H

#include "status.h"
#include "slice.h"

namespace sshkv{
	class DB{
	public:

		DB() { }
		virtual ~DB();

		static STATUS Open(const std::string& name, DB** dbptr);

		virtual STATUS Put(const Slice& key, const Slice& value, void(*cb_func)(void*), void* cb_arg) = 0;

		virtual STATUS Delete(const Slice& key, void(*cb_func)(void*), void* cb_arg) = 0;

		virtual STATUS Get(const Slice& key, std::string* value, void(*cb_func)(void*), void* cb_arg) = 0;

		//virtual Iterator* NewIterator(const ReadOptions& options) = 0;

	private:
		// No copying allowed
		DB(const DB&);
		void operator=(const DB&);
	};
}

#endif //SSHKV_DB_H
