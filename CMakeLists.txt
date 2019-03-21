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
  recovery/recovery_engine.h
  recovery/nvrec_engine.h
  recovery/recovery_defs.h
  recovery/persistent_list.h
  recovery/nvmlog_engine.h
  recovery/nvmblk_engine.h
  recovery/os_file.h
)

set(SOURCES
  core/core_workload.cc
  db/hashtable_db.cc
  db/db_factory.cc
  db/nvmdb.cc
  recovery/nvrec_engine.cc
  recovery/recovery_defs.cc
  recovery/persistent_list.cc
  recovery/nvmlog_engine.cc
  recovery/nvmblk_engine.cc
  recovery/os_file.cc
  ycsbc.cc
)

add_executable(${PROJECT_NAME} ${SOURCES} ${HEADERS})

target_link_libraries(${PROJECT_NAME} -lpthread -ltbb -lpmemobj -lpmemlog -lpmemblk -lstdc++fs)

add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
  COMMAND "${CMAKE_COMMAND}" -E copy
     ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}
     ${PROJECT_SOURCE_DIR}
  COMMENT "Copying to output directory")

target_include_directories(${PROJECT_NAME}
    PRIVATE
        ${PROJECT_SOURCE_DIR}
)
