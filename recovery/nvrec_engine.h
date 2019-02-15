#ifndef RECOVERY_NVREC_ENGINE_H
#define RECOVERY_NVREC_ENGINE_H

#include "recovery/persistent_list.h"

class NVRecEngine {
public:
    NVRecEngine();
    ~NVRecEngine();
    RecoveryStatus PersistUpdate(uint64_t key,
                                 const std::vector<KVPair>& values);
    RecoveryStatus PersistDelete(uint64_t key);
    RecoveryStatus Recover(std::map<std::string, Table> &tables);
private:
    pmem::obj::pool<PersistentList> pool_;
    pmem::obj::persistent_ptr<PersistentList> list_;
    std::unordered_map<uint64_t,
    pmem::obj::persistent_ptr<ListNode>> lookup_table_;
};


#endif //RECOVERY_NVREC_ENGINE_H
