//
// Created by zzyyy on 2017/4/24.
//
#include "../include/BGThread.h"
#include <string>
#include <iostream>

using namespace sshkv;

void func(void* arg){
	std::cout<<*(reinterpret_cast<std::string*>(arg))<<std::endl;
}

class thread_test{
public:
	thread_test():info("test info"){

	}

	~thread_test(){

	}

	void startBG(){
		bgThread.Schedule(&thread_test::BGWork, this);
	}

private:
	static void BGWork(void* arg){
		reinterpret_cast<thread_test*>(arg)->print();
	}

	void print(){
		std::cout<<info<<std::endl;
	}

	std::string info;
	BGThread bgThread;
};

int main(){
	BGThread bg_thread;
	thread_test t1;
	std::string s("hello world");
	std::cout<<"before start work"<<std::endl;
	bg_thread.DoThreadWork();

	std::cout<<"schedule"<<std::endl;

	bg_thread.Schedule(&func, &s);

	//bg_thread.Schedule(&test::BGWork, &t1);
	t1.startBG();
	std::cout<<"end work"<<std::endl;
	return 0;

}