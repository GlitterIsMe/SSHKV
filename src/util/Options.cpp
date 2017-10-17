//
// Created by zzyyy on 2017/4/16.
//
#include "../include/Options.h"

namespace sshkv {
    Options::Options()
            : max_LBA((uint64_t) 60 * 1024 * 1024 * 1024),
              max_block_size(1 * 1024 * 1024),
              max_zone_size((uint64_t)512 * 1024 * 1024) {
    }

    Options::~Options() {
    }

    ReadOptions::ReadOptions() {
        LBA = 0;
        size = 0;
    }

    StartupOptions::StartupOptions()
            : max_LBA_size((uint64_t) 10 * 1024 * 1024 * 1024),
              Physical_size((uint64_t) 5 * 1024 * 1024 * 1024),
              max_SCM_size(1500 * 100 * 100),
              DISK("/dev/sde"),
              max_subsystem_num(1) {

    }
    /*StartupOptions::StartupOptions()
            : max_LBA_size((uint64_t) 80 * 1024 * 1024),
              Physical_size((uint64_t) 40 * 1024 * 1024),
              max_SCM_size(10 * 100 * 100),
              DISK("/tfcard/SSDFile"),
              max_subsystem_num(1) {
    }*/
}

