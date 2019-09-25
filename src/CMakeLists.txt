
INCLUDE_DIRECTORIES(${PROJECT_SOURCE_DIR}/include)

AUX_SOURCE_DIRECTORY(util DIR_UTILS)

#AUX_SOURCE_DIRECTORY(test/BlockTest DIR_TEST)

#SET(TEST_SSHKV ${DIR_UTILS})

#set(SOURCE_FILES main.cpp util/Block.cpp include/Block.h include/BlockZone.h util/BlockZone.cpp include/status.h include/Keymap.h util/Keymap.cpp include/SSDAllocator.h util/SSDAllocator.cpp include/Options.h util/Options.cpp include/DB.h util/DBImpl.cpp include/DBImpl.h)

#add_executable(SSHKV ${SOURCE_FILES})

SET(BLOCK_TEST_DIR test/BlockTest.cpp include/Block.h util/Block.cpp)

SET(BLOCKZONE_TEST_DIR test/BlockZoneTest.cpp include/BlockZone.h util/BlockZone.cpp include/Block.h util/Block.cpp)

SET(BGTHREAD_TEST_DIR test/BGThreadTest.cpp include/BGThread.h util/BGThread.cpp)

SET(SUBSYSTEM_TEST_DIR
        test/SubSystemTest.cpp
        util/SubSystem.cpp
        util/BGThread.cpp
        util/SCMKeyTable.cpp
        util/IORequest.cpp
        util/Options.cpp
        util/WriteBuffer.cpp
        util/SSDAllocator.cpp
        util/Block.cpp
        util/BlockZone.cpp
        util/DataReader.cpp
        util/DataWriter.cpp
        util/Keymap.cpp
        util/Hash.cpp)

SET(DB_BENCH_DIR test/DB_bench.cpp
        util/BGThread.cpp
        util/Block.cpp
        util/BlockZone.cpp
        util/DataReader.cpp
        util/DataWriter.cpp
        util/DBImpl.cpp
        util/Hash.cpp
        util/IORequest.cpp
        util/Keymap.cpp
        util/Options.cpp
        util/SCMKeyTable.cpp
        util/SSDAllocator.cpp
        util/SubSystem.cpp
        util/WriteBuffer.cpp)

SET(DISK_TEST_DIR test/DISKTest.cpp)

SET(HASH_TEST_DIR
        test/HashTest.cpp
        util/Hash.cpp)

SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAG} -std=c++11 -pthread")

ADD_EXECUTABLE(sshkv ${DIR_UTILS} util/IORequest.cpp)

ADD_EXECUTABLE(block_test ${BLOCK_TEST_DIR})

ADD_EXECUTABLE(block_zone_test ${BLOCKZONE_TEST_DIR})

ADD_EXECUTABLE(bgthread_test ${BGTHREAD_TEST_DIR})

ADD_EXECUTABLE(subsystem_test ${SUBSYSTEM_TEST_DIR})

ADD_EXECUTABLE(db_bench ${DB_BENCH_DIR})

ADD_EXECUTABLE(disk_test ${DISK_TEST_DIR})

ADD_EXECUTABLE(hash_test ${HASH_TEST_DIR})