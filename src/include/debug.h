//
// Created by 張藝文 on 2019/9/24.
//
//#define  SSHKV_DEBUG_OUTPUT

#ifndef SSHKV_DEBUG_H
#define SSHKV_DEBUG_H

#ifdef SSHKV_DEBUG_OUTPUT
#define DBGprint(...) printf(__VA_ARGS__);
#else
#define DBGprint(...)
#endif

#endif //SSHKV_DEBUG_H
