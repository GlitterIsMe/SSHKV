//
// Created by zzyyyww on 17-5-4.
//
#include <iostream>
#include "../include/Hash.h"

int main(){
    sshkv::PrepareCryptTable();
    std::string buf("stringtest");
    for(int i = 0; i < 10; i++){
        size_t hash = sshkv::hash(buf, 0);
        std::cout<<"hash = "<<hash<<std::endl;
    }
    return 0;
}

