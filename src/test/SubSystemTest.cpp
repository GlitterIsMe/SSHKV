//
// Created by lenovo on 2017/4/30.
//

#include <cstdio>
#include <string>
#include <iostream>
#include "../include/SubSystem.h"
#include "../include/BGThread.h"
#include "../include/SCMKeyTable.h"
#include "../include/Hash.h"
#include "../include/slice.h"

using namespace sshkv;

BGThread* g_thread = nullptr;
SCMKey* g_table = nullptr;

void write_cb(void* arg){
    io_request* req = reinterpret_cast<io_request*>(arg);
    std::string* msg = reinterpret_cast<std::string*>(req->cb_arg);
    if(req->completion){
        //printf("%s\n", msg->c_str());
        std::cout<<(*msg)<<std::endl;
    }
}

void read_cb(void* arg){
    io_request* req = reinterpret_cast<io_request*>(arg);
    if(req->completion){
        std::cout<<*(req->read_value)<<std::endl;
    }
}


int main(){
    //g_thread = new BGThread;
    //BGThread g_thread;
    g_table = new SCMKey[1024];
    std::string msg("success");
    std::string read_value;
    Options opt;
    opt.max_LBA = (uint64_t)2 * 1024 * 1024 * 1024;
    opt.max_zone_size = 2 * 1024 * 1024;
    opt.max_block_size = 64 * 1024;
    SubSystem* system = new SubSystem(0, (uint64_t)2 * 1024 * 1024 * 1024, g_table, 0, 1024, -1, opt);

    printf("init finish\n");
    io_request* request = io_request::NewWriteIORequest("hello", "world", &write_cb, &msg);
    io_request* read_req = io_request::NewReadIORequest("hello", &read_value, &read_cb, nullptr);

    system->Submit(request);
    system->Submit(read_req);
    printf("finish submit\n");

    while(!request->completion){}
    while(!read_req->completion){}

    system->ShutDown();

    delete system;

    delete request;
    //delete g_thread;
    delete[] g_table;

    return 0;
}
