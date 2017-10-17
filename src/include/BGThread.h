//
// Created by zzyyy on 2017/4/23.
//

#ifndef SSHKV_BGTHREAD_H
#define SSHKV_BGTHREAD_H

#include <queue>
#include <mutex>
#include <thread>
#include <atomic>
#include <condition_variable>

namespace sshkv{

	struct BGItem{
		void(*function)(void*);
		void* arg;
	};

	class BGThread{
	public:
		BGThread();
		~BGThread();

		void Schedule(void (*function)(void*), void* arg);

		static void BGThreadWrapper(void* arg);

		void DoThreadWork();

	private:

		bool busy;
		bool bg_thread_start;

		std::queue<BGItem> thread_queue;

		std::mutex mu;
		std::condition_variable cv;
		//std::atomic_flag flag;

		std::thread* bg_thread;

	};
}//namespace sshkv
#endif //SSHKV_BGTHREAD_H
