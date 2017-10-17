//
// Created by zzyyy on 2017/4/18.
//

#ifndef SSHKV_SUBSYSTEM_H
#define SSHKV_SUBSYSTEM_H

#include <queue>
#include <thread>
#include <mutex>
#include <atomic>
#include <condition_variable>
#include "status.h"
#include "IORequest.h"
#include "Options.h"

namespace sshkv {

    class SCMKeyTable;

    class SSDAllocator;

    class WriteBuffer;

    class DataReader;

    class DataWriter;

    class Keymap;

    struct GCUnit;

    class BGThread;

    class SCMKey;

    class SubSystem {
    public:
        SubSystem(uint64_t LBA_start_, uint64_t LBA_size_, SCMKey *table, size_t SCM_start_, size_t SCM_size_, int fd_,
                  const Options &options_);

        ~SubSystem();

        //submit a request to the io queue
        void Submit(io_request *request);

        //system on
        static void RunWrapper(void *arg);

        //stop system
        //void Stop();

        //shut down system
        void ShutDown();

        //check whether the subsystem is busy or not
        bool Busy() {
            return system_busy;
        }

        uint64_t real_data_size();


    private:

        STATUS DoRead(io_request *request);

        STATUS DoWrite(io_request *request);

        STATUS DoDelete(io_request *request);

        STATUS Update(size_t hash_addr, const Slice &value);

        STATUS Write(size_t hash_addr, const Slice &value);

        void DoPersist();

        WriteBuffer* NewWriteBuffer(uint64_t buffer_size);

        WriteBuffer* NewWriteBuffer(GCUnit* unit, uint64_t buffer_size);

        DataWriter *NewDataWriter(int fd_, uint64_t LBA_, const Slice &data_);

        static void BGWork_W(void *arg);

        //take out a request from the queue and execute it
        STATUS ExecuteRequest(io_request *request);

        uint64_t LBAtoBlockno(uint64_t LBA_);

        void Run();

        bool Scheduled(){
            return !request_queue.empty();
        }
        //STATUS MakeRoomforWrite();

        //GC
        void MaybeScheduleGC();

        static void BGWork_GC(void *subsystem_);

        void PrepareforGC();

        void DoGarbageCollection(GCUnit *unit);
        void DoCleanUp(GCUnit* unit);

        const Options options;
        uint64_t LBA_start;
        size_t SCM_start;

        uint64_t LBA_size;
        size_t SCM_size;


        SCMKeyTable *key_table;
        SSDAllocator *allocator;
        Keymap *keymap;

        BGThread *bg_thread;

        WriteBuffer *active_buffer;
        WriteBuffer *mutable_buffer;

        std::thread *main_thread;

        int DISK;

        bool system_busy;
        bool run;

        bool GCworking;
        bool bgWriteWorking;

        std::mutex mu;
        std::condition_variable cv;
        //std::atomic_flag flag;

        std::queue<io_request *> request_queue;

        static int number;
        int no;

    };
}//namespace sshkv
#endif //SSHKV_SUBSYSTEM_H
