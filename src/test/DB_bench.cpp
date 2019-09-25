//
// Created by zzyyy on 2017/4/24.
//


#include <thread>
#include <string>
#include <mutex>
#include <cstdio>
#include <vector>
#include <sys/time.h>
#include <cstdlib>
#include <fcntl.h>
#include <zconf.h>
#include <atomic>
#include "DB.h"
#include "../include/DBImpl.h"
#include "../include/slice.h"
#include "../include/Options.h"
#include "../include/Random.h"
#include "../include/TestUtil.h"

static const char *FLAGS_benchmarks =
                "fillseq,"
                "fillrandom,"
                "fillrandom,"
                //"deleterandom,"
                //"readseq,"
                "fillrandom,"
                "fillrandom,"
                "readrandom,";
                //"readseq,";

static int FLAGS_num = 2500000;

static int FLAGS_reads = 1000000;

static int FLAGS_threads = 1;

static int FLAGS_value_size = 4096;

static int FLAGS_write_buffer_size = 0;

static int FLAGS_block_size = 0;

static bool FLAGS_use_existing_db = false;

static int FLAGS_subsystem_num = 4;

//static const char* FLAGS_db;

static const char *db_NAME = "SSHKV";

int kMajorVersion = 1;
int kMinorVersion = 0;


namespace sshkv {

    uint64_t NowMicros() {
        struct timeval tv;
        gettimeofday(&tv, nullptr);
        return static_cast<uint64_t>(tv.tv_sec) * 1000000 + tv.tv_usec;
    }

    struct StartThreadState {
        void (*user_function)(void *);

        void *arg;
    };

    class StartThread {
    public:
        StartThread() {/*printf("StartThread init");*/}

        ~StartThread() {
            if (start_thread->joinable()) {
                start_thread->join();
            }
            delete start_thread;
        }

        void Start(void(*function)(void *arg), void *arg) {
            StartThreadState *state = new StartThreadState;
            state->user_function = function;
            state->arg = arg;
            //printf("new thread\n");
            start_thread = new std::thread(&StartThread::StartThreadWrapper, state);
            //printf("after new thread\n");
        }

        static void *StartThreadWrapper(void *arg) {
            StartThreadState *state = reinterpret_cast<StartThreadState *>(arg);
            //printf("do method\n");
            state->user_function(state->arg);
            delete state;
            //if (start_thread->joinable())start_thread->join();
            //delete start_thread;
            return nullptr;
        }

    private:
        std::thread *start_thread;
    };

    StartThread *g_thread = nullptr;


    class RandomGenerator {
    private:
        std::string data_;
        int pos_;

    public:
        RandomGenerator() {
            // We use a limited amount of data over and over again and ensure
            // that it is larger than the compression window (32KB), and also
            // large enough to serve all typical value sizes we want to write.
            Random rnd(301);
            std::string piece;
            while (data_.size() < 1048576) {
                // Add a short fragment
                CompressibleString(&rnd, 100, &piece);
                data_.append(piece);
            }
            pos_ = 0;
        }

        Slice Generate(size_t len) {
            if (pos_ + len > data_.size()) {
                pos_ = 0;
                assert(len < data_.size());
            }
            pos_ += len;
            char *value_data = new char[len];
            memcpy(value_data, data_.data() + pos_ - len, len);
            return Slice(value_data, len);
        }
    };

    static void AppendWithSpace(std::string *str, Slice msg) {
        if (msg.empty()) return;
        if (!str->empty()) {
            str->push_back(' ');
        }
        str->append(msg.data(), msg.size());
    }

    class Stats {
    private:
        double start_;//start time
        double finish_;//end time
        double seconds_;//total run time
        int done_;
        int next_report_;
        int64_t bytes_;
        double last_op_finish_;
        //Histogram hist_;
        std::string message_;

    public:
        Stats() { Start(); }

        void Start() {
            //printf("start\n");
            next_report_ = 100;
            //printf("end next_report\n");
            last_op_finish_ = start_;
            //hist_.Clear();
            done_ = 0;
            bytes_ = 0;
            seconds_ = 0;
            start_ = NowMicros();
            finish_ = start_;
            message_.clear();
            //printf("end start\n");
        }

        /*void Merge(const Stats& other) {
            //hist_.Merge(other.hist_);
            done_ += other.done_;
            bytes_ += other.bytes_;
            seconds_ += other.seconds_;
            if (other.start_ < start_) start_ = other.start_;
            if (other.finish_ > finish_) finish_ = other.finish_;

            // Just keep the messages from one thread
            if (message_.empty()) message_ = other.message_;
        }*/

        void Stop() {
            finish_ = NowMicros();
            seconds_ = (finish_ - start_) * 1e-6;
        }

        void AddMessage(Slice msg) {
            AppendWithSpace(&message_, msg);
        }

        void FinishedSingleOp() {
            done_++;
            if (done_ >= next_report_) {
                if (next_report_ < 1000) next_report_ += 100;
                else if (next_report_ < 5000) next_report_ += 500;
                else if (next_report_ < 10000) next_report_ += 1000;
                else if (next_report_ < 50000) next_report_ += 5000;
                else if (next_report_ < 100000) next_report_ += 10000;
                else if (next_report_ < 500000) next_report_ += 50000;
                else next_report_ += 100000;
                fprintf(stderr, "... finished %d ops%30s\r", done_, "");
                fflush(stderr);
            }
        }

        void AddBytes(int64_t n) {
            bytes_ += n;
        }

        void Report(const Slice &name) {
            if (done_ < 1) done_ = 1;

            std::string extra;
            if (bytes_ > 0) {
                // Rate is computed on actual elapsed time, not the sum of per-thread
                // elapsed times.
                double elapsed = (finish_ - start_) * 1e-6;
                char rate[100];
                snprintf(rate, sizeof(rate), "%6.1f MB/s",
                         (bytes_ / 1048576.0) / elapsed);
                extra = rate;
            }
            AppendWithSpace(&extra, message_);

            fprintf(stdout, "%-12s : %11.3f micros/op;%s%s\n",
                    name.ToString().c_str(),
                    seconds_ * 1e6 / done_,
                    (extra.empty() ? "" : " "),
                    extra.c_str());
            fflush(stdout);
        }
    };

// Per-thread state for concurrent executions of the same benchmark.
    struct ThreadState {
        int tid;             // 0..n-1 when running in n threads
        Random rand;         // Has different seeds for different threads
        Stats stats;
        bool done;
        //SharedState* shared;

        ThreadState(int index)
                : tid(index),
                  rand(1000 + index),
                  done(false) {
        }
    };

    class Benchmark {
    private:
        DB *db_;
        int num_;
        int value_size_;
        //int entries_per_batch_;
        //WriteOptions write_options_;
        int reads_;
	bool seconds_write;
        int heap_counter_;

        void PrintHeader() {
            const int kKeySize = 16;
            PrintEnvironment();
            fprintf(stdout, "Keys:       %d bytes each\n", kKeySize);
            fprintf(stdout, "Values:     %d bytes each (%d bytes after compression)\n",
                    FLAGS_value_size,
                    static_cast<int>(FLAGS_value_size));
            fprintf(stdout, "Entries:    %d\n", num_);
            fprintf(stdout, "RawSize:    %.1f MB (estimated)\n",
                    ((static_cast<int64_t>(kKeySize + FLAGS_value_size) * num_)
                     / 1048576.0));
            fprintf(stdout, "FileSize:   %.1f MB (estimated)\n",
                    (((kKeySize + FLAGS_value_size) * num_)
                     / 1048576.0));
            //PrintWarnings();
            fprintf(stdout, "------------------------------------------------\n");
        }


        void PrintEnvironment() {
            fprintf(stdout, "SSHKV:    version %d.%d\n",
                    kMajorVersion, kMinorVersion);
        }

    public:
        Benchmark()
                : db_(NULL),
                  num_(FLAGS_num),
                  value_size_(FLAGS_value_size),
                //entries_per_batch_(1),
                  reads_(FLAGS_reads < 0 ? FLAGS_num : FLAGS_reads),
                  heap_counter_(0) {
			seconds_write = false;
        }

        ~Benchmark() {
            delete db_;
        }

        void Run() {
            PrintHeader();
            Open();

            const char *benchmarks = FLAGS_benchmarks;
            while (benchmarks != NULL) {
                const char *sep = strchr(benchmarks, ',');
                Slice name;
                if (sep == NULL) {
                    name = benchmarks;
                    benchmarks = NULL;
                } else {
                    name = Slice(benchmarks, sep - benchmarks);
                    benchmarks = sep + 1;
                }

                // Reset parameters that may be overridden below
                num_ = FLAGS_num;
                reads_ = (FLAGS_reads < 0 ? FLAGS_num : FLAGS_reads);
                value_size_ = FLAGS_value_size;
                //entries_per_batch_ = 1;
                //write_options_ = WriteOptions();

                void (Benchmark::*method)(ThreadState *) = NULL;
                bool fresh_db = false;
                int num_threads = FLAGS_threads;

                if (name == Slice("open")) {
                    method = &Benchmark::OpenBench;
                    num_ /= 10000;
                    if (num_ < 1) num_ = 1;
                } else if (name == Slice("fillseq")) {
                    //fresh_db = true;
                    method = &Benchmark::WriteSeq;
                } else if (name == Slice("fillrandom")) {
                    //fresh_db = true;
                    method = &Benchmark::WriteRandom;
		    //num_ = 1250000;
                } else if (name == Slice("overwrite")) {
                    //fresh_db = false;
                    method = &Benchmark::WriteRandom;
                } else if (name == Slice("fillsync")) {
                    //fresh_db = true;
                    num_ /= 1000;
                    //write_options_.sync = true;
                    method = &Benchmark::WriteRandom;
                } else if (name == Slice("fill100K")) {
                    //fresh_db = true;
                    num_ /= 1000;
                    value_size_ = 100 * 1000;
                    method = &Benchmark::WriteRandom;
                } else if (name == Slice("readseq")) {
                    method = &Benchmark::ReadSequential;
                } else if (name == Slice("readrandom")) {
                    method = &Benchmark::ReadRandom;
                } else if (name == Slice("seekrandom")) {
                    method = &Benchmark::SeekRandom;
                } else if (name == Slice("readrandomsmall")) {
                    reads_ /= 1000;
                    method = &Benchmark::ReadRandom;
                } else if (name == Slice("deleteseq")) {
                    method = &Benchmark::DeleteSeq;
                } else if (name == Slice("deleterandom")) {
                    method = &Benchmark::DeleteRandom;
			num_ = 1250000;
                } else {
                    if (name != Slice()) {  // No error message for empty name
                        fprintf(stderr, "unknown benchmark '%s'\n", name.ToString().c_str());
                    }
                }

                if (method != NULL) {
                    printf("method name : %s size : %ld\n", name.data(), name.size());
                    RunBenchmark(name, method);
                }
            }
        }

    private:
        struct ThreadArg {
            Benchmark *bm;
            ThreadState *thread;

            void (Benchmark::*method)(ThreadState *);
        };

        static void ThreadBody(void *v) {
            printf("thread body\n");
            ThreadArg *arg = reinterpret_cast<ThreadArg *>(v);

            ThreadState *thread = arg->thread;

            thread->stats.Start();
            printf("after start\n");
            (arg->bm->*(arg->method))(thread);
            thread->stats.Stop();
            thread->done = true;

        }

        static void write_cb(void *arg) {
            io_request *req = reinterpret_cast<io_request *>(arg);
            write_cb_arg *write_arg = reinterpret_cast<write_cb_arg *>(req->cb_arg);
            //printf("cb func\n");
            delete[] req->key.data();
            //write_arg->mu.lock();
            write_arg->pending--;
            //write_arg->mu.unlock();
            //printf("cb func end\n");
        }

        struct write_cb_arg {
            //int pending;
            //std::mutex mu;
            std::atomic_long pending;
            write_cb_arg():pending(0){}
        };

        void RunBenchmark(Slice name, void (Benchmark::*method)(ThreadState *)) {

            ThreadArg *arg = new ThreadArg;
            arg->bm = this;
            arg->method = method;
            //arg[i].shared = &shared;
            arg->thread = new ThreadState(1);
            //arg[i].thread->shared = &shared;
            g_thread->Start(ThreadBody, arg);
            while (!arg->thread->done) {/*wait*/}

            arg->thread->stats.Report(name);

            delete arg->thread;
            delete arg;
        }

        void Open() {
            assert(db_ == NULL);
            STATUS s = DBImpl::Open(db_NAME, &db_);
            if (s != SUCCESS) {
                fprintf(stderr, "open error\n");
                exit(1);
            }
        }

        void OpenBench(ThreadState *thread) {
            for (int i = 0; i < num_; i++) {
                delete db_;
                Open();
                thread->stats.FinishedSingleOp();
            }
        }

        void WriteSeq(ThreadState *thread) {
            DoWrite(thread, true);
        }

        void WriteRandom(ThreadState *thread) {
            DoWrite(thread, false);
        }

        void DoWrite(ThreadState *thread, bool seq) {
            //printf("Do write\n");
            if (num_ != FLAGS_num) {
                char msg[100];
                snprintf(msg, sizeof(msg), "(%d ops)", num_);
                thread->stats.AddMessage(msg);
            }

            int key_size = 100;
            write_cb_arg arg;
            //arg.pending = 0;

            RandomGenerator gen;

            //FILE* fp = fopen("/home/zzyyyww/GraduationProject/writelog.txt", "w");

            STATUS s;
            int64_t bytes = 0;
            for (int i = 0; i < num_; i++) {
                const int k = seq ? i : (thread->rand.Next() % FLAGS_num);
                //char key[100];
                char *key = new char[100];
                //printf("sizeof key %ld\n", sizeof(key));
                snprintf(key, key_size, "%016d", k);
                //printf("put %s\n", key);
                //fprintf(fp, "%s\n", key);
                //batch.Put(key, gen.Generate(value_size_));
                bytes += value_size_ + strlen(key);
                thread->stats.FinishedSingleOp();
                //arg.mu.lock();
                arg.pending++;
                //printf("pending request : %d\n", arg.pending);
                //arg.mu.unlock();
                s = db_->Put(key, gen.Generate(value_size_), &Benchmark::write_cb, &arg);
                if (s != SUCCESS) {
                    fprintf(stderr, "put error %d\n", s);
                    exit(1);
                }
            }
            thread->stats.AddBytes(bytes);
            while (arg.pending != 0) {/*printf("pending : %d\n", arg.pending); */}
            //fclose(fp);
        }

        struct shared_state {

            //std::mutex mu;
            //int found;
            //int pending;
            std::atomic_long found;
            std::atomic_long pending;

            shared_state(): found(0), pending(0){}
        };


        struct read_cb_arg {
            shared_state *share;
            std::string *read_value;
            //FILE* fp;
        };

        static void read_cb(void *arg) {
            io_request *req = reinterpret_cast<io_request *>(arg);
            read_cb_arg *read_arg = reinterpret_cast<read_cb_arg *>(req->cb_arg);

            //read_arg->share->mu.lock();

            read_arg->share->pending--;

            if (req->completion) {
                read_arg->share->found++;
            } else {
                //DEBUG
                //fprintf(read_arg->fp, "%s\n", req->key.data());
            }
            delete req->read_value;
            delete[] req->key.data();
            //read_arg->share->mu.unlock();
        }

        void ReadSequential(ThreadState *thread) {

            ReadOptions options;
            //std::string value;
            //int found = 0;
            //int pending = 0;

            shared_state shared;
            //shared.found = 0;
            //shared.pending = 0;

            int key_size = 100;

            for (int i = 0; i < reads_; i++) {
                //char key[100];
                char *key = new char[key_size];
                std::string *value = new std::string;
                const int k = i;
                snprintf(key, key_size, "%016d", k);

                //shared.mu.lock();
                shared.pending++;
                //shared.mu.unlock();

                read_cb_arg arg;
                arg.share = &shared;
                arg.read_value = value;
                //arg.fp = fp;

                if(db_->Get(key, value, &Benchmark::read_cb, &arg) == SUCCESS){
			thread->stats.FinishedSingleOp();
		}
            }
            while (shared.pending > 0) {}
            char msg[100];

            long result = 0;
            result = shared.found.load(std::memory_order_relaxed);
            snprintf(msg, sizeof(msg), "(%ld of %d found)", result, num_);
            
	    thread->stats.AddMessage(msg);
        }

        void ReadRandom(ThreadState *thread) {
            ReadOptions options;
            //std::string value;
            //int found = 0;
            //int pending = 0;

            shared_state shared;
            //shared.found = 0;
            //shared.pending = 0;

            int key_size = 100;
            //FILE* fp = fopen("/home/zzyyyww/GraduationProject/readlog.txt", "w");

            for (int i = 0; i < reads_; i++) {
                //char key[100];
                char *key = new char[key_size];
                std::string *value = new std::string;
                const int k = thread->rand.Next() % FLAGS_num;
                snprintf(key, key_size, "%016d", k);

                //shared.mu.lock();
                shared.pending++;
                //shared.mu.unlock();

                read_cb_arg arg;
                arg.share = &shared;
                arg.read_value = value;
                //arg.fp = fp;

                if(db_->Get(key, value, &Benchmark::read_cb, &arg) == SUCCESS){
			thread->stats.FinishedSingleOp();
		}
            }
            while (shared.pending > 0) {}
            char msg[100];

            long result = 0;
            result = shared.found.load(std::memory_order_relaxed);
            snprintf(msg, sizeof(msg), "(%ld of %d found)", result, num_);

            thread->stats.AddMessage(msg);
            //fclose(fp);

        }

        void SeekRandom(ThreadState *thread) {
        }

        void DoDelete(ThreadState *thread, bool seq) {
            RandomGenerator gen;
            //WriteBatch batch;
            STATUS s;
            //int pending = 0;
            int key_size = 100;
            write_cb_arg arg;
            //arg.pending = 0;
            for (int i = 0; i < num_; i++) {
                //batch.Clear();

                const int k = seq ? i : (thread->rand.Next() % FLAGS_num);
                char* key = new char[100];
                snprintf(key, key_size, "%016d", k);
                //batch.Delete(key);
                thread->stats.FinishedSingleOp();

                //arg.mu.lock();
                arg.pending++;
                //arg.mu.unlock();

                s = db_->Delete(key, &Benchmark::write_cb, &arg);
                if (s != SUCCESS) {
                    fprintf(stderr, "del error\n");
                    exit(1);
                }
            }
            while (arg.pending != 0) {/*wait*/ }
        }

        void DeleteSeq(ThreadState *thread) {
            DoDelete(thread, true);
        }

        void DeleteRandom(ThreadState *thread) {
            DoDelete(thread, false);
        }

    };

}//namespace sshkv


int main(int argc, char **argv) {

    for (int i = 1; i < argc; i++) {
        //double d;
        int n;
        char junk;
        if (sshkv::Slice(argv[i]).starts_with("--benchmarks=")) {
            FLAGS_benchmarks = argv[i] + strlen("--benchmarks=");
        } else if (sscanf(argv[i], "--num=%d%c", &n, &junk) == 1) {
            FLAGS_num = n;
        } else if (sscanf(argv[i], "--reads=%d%c", &n, &junk) == 1) {
            FLAGS_reads = n;
        } else if (sscanf(argv[i], "--value_size=%d%c", &n, &junk) == 1) {
            FLAGS_value_size = n;
        } else if (sscanf(argv[i], "--block_size=%d%c", &n, &junk) == 1) {
            FLAGS_block_size = n;
        } else if (sscanf(argv[i], "--subsystem_num=%d%c", &n, &junk) == 1) {
            FLAGS_subsystem_num = n;
        } else if (strncmp(argv[i], "--db=", 5) == 0) {
            db_NAME = argv[i] + 5;
        } else {
            fprintf(stderr, "Invalid flag '%s'\n", argv[i]);
            exit(1);
        }
    }
    printf("before init thread\n");
    sshkv::g_thread = new sshkv::StartThread;
    sshkv::Benchmark benchmark;
    printf("after init\n");
    benchmark.Run();
    return 0;
}

