//
// Created by zzyyy on 2017/4/23.
//
#include <../include/BGThread.h>

namespace sshkv{
	BGThread::BGThread()
			: bg_thread_start(false),
			  busy(false)
			  /*flag(ATOMIC_FLAG_INIT)*/{
		while(!thread_queue.empty()){thread_queue.pop();}
	}

	BGThread::~BGThread() {
		cv.notify_one();
		while(busy){}
		bg_thread_start = false;
		if (bg_thread->joinable()){
			bg_thread->join();
		}
		delete bg_thread;
	}

	void BGThread::BGThreadWrapper(void *arg) {
		reinterpret_cast<BGThread*>(arg)->DoThreadWork();
	}

	void BGThread::Schedule(void (*function)(void *), void *arg) {
        //printf("start schedule\n");
		//mu.lock();
		std::lock_guard<std::mutex> lk(mu);
		//while(flag.test_and_set()){}
		if (!bg_thread_start){
			bg_thread_start = true;
			bg_thread = new std::thread(std::bind(&BGThread::BGThreadWrapper, this));
		}

		BGItem item;
		item.function = function;
		item.arg = arg;
		//printf("bgthread push\n");
		thread_queue.push(item);

		//flag.clear();
		cv.notify_one();
        //printf("end schedule\n");
	}

	void BGThread::DoThreadWork() {
		while(bg_thread_start){

			std::unique_lock<std::mutex> lk(mu);
			while(thread_queue.empty()){
				busy = false;
				if (!bg_thread_start) return ;
				cv.wait(lk);
			}

            //mu.lock();
			//while(flag.test_and_set()){}
			busy = true;
			BGItem item = thread_queue.front();
			thread_queue.pop();
			lk.unlock();
			//printf("thread_queue num = %d\n", thread_queue.size());
			void(*function)(void*) = item.function;
			void* arg = item.arg;
			//flag.clear();
			//mu.unlock();
			(*function)(arg);
		}
	}
}//namespace sshkv

