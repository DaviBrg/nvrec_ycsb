cmake_minimum_required(VERSION 3.5)

set (CMAKE_CXX_STANDARD 17)

project(ycsb_nvrec)

set(HEADERS
  core/properties.h
  core/skewed_latest_generator.h
  core/timer.h
  core/uniform_generator.h
  core/counter_generator.h
  core/utils.h
  core/generator.h
  core/zipfian_generator.h
  core/core_workload.h
  core/client.h
  core/scrambled_zipfian_generator.h
  core/const_generator.h
  core/db.h
  core/discrete_generator.h
#  redis/redis_client.h
#  redis/hiredis/read.h
#  redis/hiredis/sds.h
#  redis/hiredis/adapters/glib.h
#  redis/hiredis/adapters/libuv.h
#  redis/hiredis/adapters/qt.h
#  redis/hiredis/adapters/libev.h
#  redis/hiredis/adapters/ae.h
#  redis/hiredis/adapters/libevent.h
#  redis/hiredis/adapters/ivykis.h
#  redis/hiredis/adapters/macosx.h
#  redis/hiredis/dict.h
#  redis/hiredis/net.h
#  redis/hiredis/fmacros.h
#  redis/hiredis/examples/example-qt.h
#  redis/hiredis/hiredis.h
#  redis/hiredis/async.h
#  redis/hiredis/win32.h
  lib/stl_hashtable.h
  lib/mem_alloc.h
  lib/tbb_rand_hashtable.h
  lib/string.h
  lib/lock_stl_hashtable.h
  lib/string_hashtable.h
  lib/tbb_scan_hashtable.h
  db/tbb_scan_db.h
  db/db_factory.h
  db/hashtable_db.h
  db/basic_db.h
  db/lock_stl_db.h
  db/tbb_rand_db.h
  db/nvmdb.h
#  db/redis_db.h
  recovery/recovery_engine.h
)

set(SOURCES
  core/core_workload.cc
#  redis/hiredis/dict.c
#  redis/hiredis/net.c
#  redis/hiredis/test.c
#  redis/hiredis/examples/example-libuv.c
#  redis/hiredis/examples/example-macosx.c
#  redis/hiredis/examples/example-libev.c
#  redis/hiredis/examples/example-glib.c
#  redis/hiredis/examples/example-libevent.c
#  redis/hiredis/examples/example-qt.cpp
#  redis/hiredis/examples/example-ivykis.c
#  redis/hiredis/examples/example-ae.c
#  redis/hiredis/examples/example.c
#  redis/hiredis/read.c
#  redis/hiredis/hiredis.c
#  redis/hiredis/async.c
#  redis/hiredis/sds.c
#  redis/sample.cc
  db/hashtable_db.cc
#  db/redis_db.cc
  db/db_factory.cc
  db/nvmdb.cc
  recovery/recovery_engine.cc
  ycsbc.cc
)

add_executable(${PROJECT_NAME} ${SOURCES} ${HEADERS})

target_link_libraries(${PROJECT_NAME} -lpthread -ltbb)

target_include_directories(${PROJECT_NAME}
    PRIVATE
        ${PROJECT_SOURCE_DIR}
)
