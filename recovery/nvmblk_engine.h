#ifndef RECOVERY_NVMBLK_ENGINE_H
#define RECOVERY_NVMBLK_ENGINE_H

#include <memory>
#include <unordered_map>
#include <libpmemblk.h>

#include "recovery/recovery_engine.h"


class NVMBlkEngine : RecoveryEngine{
public:
    ~NVMBlkEngine();
    NVMBlkEngine(const NVMBlkEngine&) = delete;
    NVMBlkEngine(NVMBlkEngine&&) = delete;
    NVMBlkEngine operator=(const NVMBlkEngine&) = delete;
    NVMBlkEngine operator=(NVMBlkEngine&&) = delete;
    static std::unique_ptr<RecoveryEngine> BuildEngine();
    RecoveryStatus PersistUpdate(uint64_t key,
                                 const std::vector<KVPair>& values);
    RecoveryStatus PersistDelete(uint64_t key);
    RecoveryStatus Recover(std::map<std::string, Table> &tables);
private:
    NVMBlkEngine(PMEMblkpool *blk_pool, size_t max_num_blocks);
    RecoveryStatus UpdateOnNewBlk(const Tuple &tuple);
    RecoveryStatus UpdateOnBlkPosition(const Tuple &tuple, size_t position);
    RecoveryStatus UpdateTuple(const Tuple &tuple);
    void FlushToDisk();
    std::unordered_map<uint64_t, size_t> lookup_table_;
    PMEMblkpool *blk_pool_ = nullptr;
    size_t max_num_blocks_ = 0, next_block_ = 0;
};


#endif //RECOVERY_NVMBLK_ENGINE_H
