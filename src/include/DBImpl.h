//
// Created by zzyyy on 2017/4/16.
//

#ifndef SSHKV_DBIMPL_H
#define SSHKV_DBIMPL_H

#include <thread>
#include <string>
#include <vector>
#include "DB.h"
#include "status.h"
#include "Options.h"
#include "IORequest.h"

namespace sshkv{

	class SubSystem;
	class SCMKeyTable;
	class SCMKey;
	class BGThread;

	class DBImpl:public DB{
	public:

		//DBImpl();
		DBImpl(const std::string db_name, const StartupOptions& options_);

		virtual ~DBImpl();

		static STATUS Open(const std::string& name, DB** dbptr);

		virtual STATUS Put(const Slice& key, const Slice& value, void(*cb_func)(void*), void* cb_arg);

		virtual STATUS Delete(const Slice& key, void(*cb_func)(void*), void* cb_arg);

		virtual STATUS Get(const Slice& key, std::string* value, void(*cb_func)(void*), void* cb_arg);

	private:

		void Submit(io_request* req, size_t subsystem_no);

		uint64_t total_size();

		size_t subsystem_num;
		std::string db_name;
		std::string DISK;
		SCMKey* table;
		uint64_t max_LBA_size;
		uint64_t Physical_size;
		size_t max_SCM_size;
		//BGThread* bg_thread;
		//SubSystem* subsystem_list;
		std::vector<SubSystem*> subsystem_list;

		int fd;
	};

}

#endif //SSHKV_DBIMPL_H
