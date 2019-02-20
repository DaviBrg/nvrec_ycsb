#ifndef RECOVERY_NVMLOG_ENGINE_H
#define RECOVERY_NVMLOG_ENGINE_H

#include "recovery/recovery_engine.h"

#include <libpmemlog.h>
#include <memory>

class NVMLogEngine : public RecoveryEngine {
public:
    ~NVMLogEngine();
    NVMLogEngine(const NVMLogEngine&) = delete;
    NVMLogEngine& operator=(const NVMLogEngine&) = delete;
    NVMLogEngine(NVMLogEngine &&other) noexcept;
    NVMLogEngine& operator=(NVMLogEngine &&other) noexcept;
    static std::unique_ptr<NVMLogEngine> BuildEngine();
    RecoveryStatus PersistUpdate(uint64_t key,
                                 const std::vector<KVPair>& values);
    RecoveryStatus PersistDelete(uint64_t key);
    RecoveryStatus Recover(std::map<std::string, Table> &tables);
private:
    RecoveryStatus UpdateOnLog(const Tuple &tuple);
    void FlushToDisk();
    NVMLogEngine(PMEMlogpool *log_pool, size_t available_bytes);
    PMEMlogpool *log_pool_ = nullptr;
    size_t current_length_ = 0, available_bytes_ = 0;
};


#endif //RECOVERY_NVMLOG_ENGINE_H
