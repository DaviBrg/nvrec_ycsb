#ifndef RECOVERY_NVREC_ENGINE_H
#define RECOVERY_NVREC_ENGINE_H

#include <future>

#include "recovery/persistent_list.h"
#include "recovery/recovery_engine.h"

class NVRecEngine : public RecoveryEngine {
public:
    NVRecEngine();
    ~NVRecEngine();
    NVRecEngine(NVRecEngine &&) = default;
    NVRecEngine& operator=(NVRecEngine &&) = default;
    RecoveryStatus PersistUpdate(uint64_t key,
                                 const std::vector<KVPair>& values);
    RecoveryStatus PersistDelete(uint64_t key);
    RecoveryStatus Recover(std::map<std::string, Table> &tables);
private:
    RecoveryStatus PersistRaw(const Tuple &value);
    RecoveryStatus FlushToDisk(std::string pool_path,
                               pmem::obj::pool<PersistentList> pool,
                               std::unordered_map<uint64_t,
                               pmem::obj::persistent_ptr<ListNode> > lookup_table);
    std::string NextPoolPath();
    pmem::obj::pool<PersistentList> pool_;
    std::unordered_map<uint64_t,
    pmem::obj::persistent_ptr<ListNode>> lookup_table_;
    size_t pool_counter_ = 0;
    std::string pool_path_ = "/mnt/mem/pmem";
    std::future<RecoveryStatus> flush_result_;
};


#endif //RECOVERY_NVREC_ENGINE_H
