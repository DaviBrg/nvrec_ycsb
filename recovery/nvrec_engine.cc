#include "recovery/nvrec_engine.h"

#include <chrono>
#include <experimental/filesystem>
#include <iostream>
#include <libpmemobj++/make_persistent.hpp>
#include <libpmemobj++/transaction.hpp>
#include <unistd.h>

#include "recovery/os_file.h"

constexpr const char * const kPoolPath = "/mnt/mem/pmem";
constexpr const char * const kLayout = "linked_list";
constexpr const char * const kFileName = "disk_log.txt";

constexpr const size_t kPoolSize = 200000000;

NVRecEngine::NVRecEngine() :
    pool_(PersistentList::MakePersistentListPool(kPoolPath,
                                                 kPoolSize,
                                                 kLayout)) {}

NVRecEngine::~NVRecEngine() {
    pool_.close();
}

RecoveryStatus NVRecEngine::PersistUpdate(uint64_t key,
                                          const std::vector<KVPair> &values) {
    Tuple tuple = DBToRaw(values);
    tuple.key = key;
    return PersistRaw(tuple);
}

RecoveryStatus NVRecEngine::PersistDelete(uint64_t key) {
    Tuple tuple;
    tuple.deleted = true;
    tuple.key = key;
    return PersistRaw(tuple);
}

RecoveryStatus NVRecEngine::Recover(std::map<std::string, Table> &tables) {
//    list_->Recover(tables, lookup_table_);
    return kSuccess;
}

RecoveryStatus NVRecEngine::PersistRaw(const Tuple &value) {
    try {
        pmem::obj::transaction::exec_tx(pool_, [&](){
            auto it = lookup_table_.find(value.key);
            if (std::end(lookup_table_) == it) {
                auto ptr = pool_.get_root()->Insert(value);
                lookup_table_[value.key] = ptr;
            } else {
                it->second->obj = value;
            }
        });
    } catch (const std::exception &e) {
        std::cerr << "<FLUSH STARTED>" << std::endl;
        return FlushToDisk();
    }
    return kSuccess;
}

RecoveryStatus NVRecEngine::FlushToDisk() {
    auto before = std::chrono::high_resolution_clock::now();
    auto current = pool_.get_root()->head();
    OSFile osf{kFileName};
    while (current != nullptr) {
        Tuple current_tuple = current->obj.get_ro();
        if (osf.Write(reinterpret_cast<char*>(&current_tuple),
                      sizeof(current_tuple)) != sizeof(current_tuple)) {
            throw std::runtime_error("Write operation failed");
        }
        current = current->next;
    }
    osf.Sync();
    lookup_table_.clear();
    pool_.close();
    if (unlink(kPoolPath) == -1) throw std::runtime_error("Cannot delete pool file");
    pool_ = PersistentList::MakePersistentListPool(kPoolPath, kPoolSize, kLayout);
    auto after = std::chrono::high_resolution_clock::now();
    std::cerr << "<FLUSH FINISHED> DURARION: " <<
            std::chrono::duration_cast<std::chrono::milliseconds>(
                     after - before).count() << "ms" << std::endl;
    return kSuccess;
}
