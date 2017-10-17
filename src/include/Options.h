//
// Created by zzyyy on 2017/4/16.
//

#ifndef SSHKV_OPTIONS_H
#define SSHKV_OPTIONS_H

#include <stdint.h>
#include <string>

namespace sshkv {
    class Options {
    public:

        Options();

        ~Options();

        uint64_t max_LBA;

        uint64_t max_block_size;

        uint64_t max_zone_size;
    };

    struct ReadOptions {
        //enum READ_TYPE{BLOCK, VALUE};
        //READ_TYPE type;
        uint64_t LBA;
        uint64_t size;

        ReadOptions();
    };

    struct StartupOptions {
        uint64_t max_LBA_size;
        uint64_t Physical_size;
        size_t max_SCM_size;
        std::string DISK;
        size_t max_subsystem_num;

        StartupOptions();
    };
}//namespace sshkv
#endif //SSHKV_OPTIONS_H
