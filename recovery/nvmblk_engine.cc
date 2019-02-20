#include "recovery/nvmblk_engine.h"

#include <fstream>

constexpr const char * const kLogPath = "/mnt/mem/pmem_blk";
constexpr size_t kPoolSize = 500000000;

NVMBlkEngine::NVMBlkEngine(PMEMblkpool *blk_pool, size_t max_num_blocks) :
    blk_pool_(blk_pool),
    max_num_blocks_(max_num_blocks) {}

NVMBlkEngine::~NVMBlkEngine() {
    if (nullptr != blk_pool_) {
        pmemblk_close(blk_pool_);
    }
}

std::unique_ptr<RecoveryEngine> NVMBlkEngine::BuildEngine() {

    auto blk_pool = pmemblk_create(kLogPath, sizeof(Tuple), kPoolSize, 0666);

    if (nullptr == blk_pool) {
        blk_pool = pmemblk_open(kLogPath, sizeof(Tuple));
    }

    if (nullptr == blk_pool) {
        return nullptr;
    }

    size_t num_blocks = pmemblk_nblock(blk_pool);

    return std::unique_ptr<RecoveryEngine>{new NVMBlkEngine(blk_pool, num_blocks)};
}

RecoveryStatus NVMBlkEngine::PersistUpdate(uint64_t key,
                                           const std::vector<KVPair> &values) {
    Tuple data = DBToRaw(values);
    data.key = key;
    return UpdateTuple(data);
}

RecoveryStatus NVMBlkEngine::PersistDelete(uint64_t key) {
    Tuple data;
    data.deleted = true;
    data.key = key;
    return UpdateTuple(data);
}

RecoveryStatus NVMBlkEngine::UpdateOnNewBlk(const Tuple &tuple) {
    if (next_block_ == max_num_blocks_) {
        FlushToDisk();
    }
    if (pmemblk_write(blk_pool_, reinterpret_cast<const void*>(&tuple),
                      next_block_) < 0) {
        return kFail;
    } else {
        lookup_table_[tuple.key] = next_block_++;
        return kSuccess;
    }
}

RecoveryStatus NVMBlkEngine::UpdateOnBlkPosition(const Tuple &tuple,
                                                 size_t position) {
    if (pmemblk_write(blk_pool_, reinterpret_cast<const void*>(&tuple),
                      position) < 0) {
        return kFail;
    } else {
        return kSuccess;
    }

}

RecoveryStatus NVMBlkEngine::UpdateTuple(const Tuple &tuple) {
    auto it = lookup_table_.find(tuple.key);

    if (std::end(lookup_table_) != it) {
        return UpdateOnBlkPosition(tuple, it->second);
    } else {
        return UpdateOnNewBlk(tuple);
    }
}

void NVMBlkEngine::FlushToDisk(){
    std::ofstream disk_flush_file{kLogPath, std::ios::app | std::ios::binary};
    Tuple current;
    for (size_t block_idx = 0; block_idx < max_num_blocks_; ++block_idx) {
        if (pmemblk_read(blk_pool_,
                         reinterpret_cast<void*>(&current) , block_idx) < 0) {
            throw std::runtime_error("Disk flush failed");
        }
        disk_flush_file.write(reinterpret_cast<char*>(&current), sizeof(Tuple));
    }
    disk_flush_file.flush();
    lookup_table_.clear();
    next_block_ = 0;
}

RecoveryStatus NVMBlkEngine::Recover(std::map<std::__cxx11::string,
                                     Table> &tables) {
    return kFail;
}
