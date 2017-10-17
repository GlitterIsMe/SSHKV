//
// Created by zzyyyww on 17-5-1.
//

#include <string>
#include <iostream>
#include <fcntl.h>
#include <zconf.h>
#include <stdio.h>
#include <unistd.h>

int main(){
    std::string words("helloworld");
    //const char* char_word = "helloworld";
    char value[1024] = {0};
    //int fd = open("/home/zzyyyww/GraduationProject/TESTFILE", O_RDWR);
    //int fd = open("/dev/sdb", O_RDWR|O_DIRECT);
    int fd = open("/tfcard/SSDFile", O_RDWR);
    if(fd == -1){
        std::cout<<"open failed"<<std::endl;
        return -1;
    }
    printf("open %d success\n", fd);
    ssize_t r;
    r = pwrite(fd, words.c_str(), words.size(), 0);
    //r = pwrite(fd, char_word, words.size(), 0);
    //r = write(fd, words.c_str(), words.size());
    printf("%ld\n", r);
    if(r == -1) {
        printf("write failed\n");
        return -1;
    }
    r = pread(fd, value, words.size(), 0);
    printf("%ld\n", r);
    if(r == -1){
        printf("read failed\n");
        return -1;
    }else{
        printf("%s\n", value);
    }
    close(fd);
    std::cout<<(*value)<<std::endl;
    return 0;
}