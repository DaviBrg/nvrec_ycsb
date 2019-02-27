#include "recovery/nvrec_engine.h"

#include <experimental/filesystem>


std::string kPoolPath = "/mnt/mem/pmem";
std::string kLayout = "linked_list";
size_t kPoolSize = 200000000;


NVRecEngine::NVRecEngine() :
    pool_(PersistentList::MakePersistentListPool(kPoolPath,
                                                 kPoolSize,
                                                 kLayout)),
    list_(pool_.get_root()) {}

NVRecEngine::~NVRecEngine() {
    pool_.close();
    std::experimental::filesystem::remove(kPoolPath);
}

RecoveryStatus NVRecEngine::PersistUpdate(uint64_t key,
                                          const std::vector<KVPair> &values) {
    auto tuple = DBToRaw(values);
    tuple.key = key;
    list_->Persist(pool_, tuple, lookup_table_);
    return kSuccess;
}

RecoveryStatus NVRecEngine::PersistDelete(uint64_t key) {
    Tuple tuple;
    tuple.deleted = true;
    tuple.key = key;
    list_->Persist(pool_, tuple, lookup_table_);
    return kSuccess;
}

RecoveryStatus NVRecEngine::Recover(std::map<std::string, Table> &tables) {
    list_->Recover(tables, lookup_table_);
    return kSuccess;
}
