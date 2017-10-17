#include <iostream>
#include "../include/DBImpl.h"
#include "../include/DB.h"


struct arg_s{
	bool completion;
	std::string msg;
};
void cb_func(void* arg){
    arg_s* s = reinterpret_cast<arg_s*>(arg);
    std::cout<<s->msg<<std::endl;
	std::cout<<"end cb func"<<std::endl;
	s->completion = true;
}


int main() {
    std::string name("sshkv");
    sshkv::DB* db = nullptr;
    std::cout<<"before open"<<std::endl;
    sshkv::DBImpl::Open(name, &db);
	std::cout<<"end open"<<std::endl;
    //std::string arg("complete");
	arg_s arg;
	arg.msg = "completoin";
	arg.completion = false;

    db->Put("hello", "world", &cb_func, &arg);
    std::cout << "end put" << std::endl;

	std::string get_value;
	arg_s arg1;
	arg1.completion = false;
	arg1.msg = "read complete";

	db->Get("hello", &get_value, &cb_func, &arg1);



	while(!arg1.completion){

	}
	std::cout<<"end get"<<std::endl;
	std::cout<<get_value<<std::endl;
	return 0;
}