#include "recovery/nvmlog_engine.h"

#include <fstream>

constexpr const char * const kLogPath = "/mnt/mem/pmem_log";
constexpr size_t kLogSize = 500000000;

NVMLogEngine::NVMLogEngine(PMEMlogpool *log_pool, size_t available_bytes) :
    log_pool_(log_pool),
    available_bytes_(available_bytes) {}

NVMLogEngine::~NVMLogEngine() {
    if (nullptr != log_pool_) {
        pmemlog_close(log_pool_);
    }
}

NVMLogEngine::NVMLogEngine(NVMLogEngine &&other) noexcept :
    log_pool_(other.log_pool_),
    current_length_(other.current_length_),
    available_bytes_(other.available_bytes_) {
    other.log_pool_ = nullptr;
}


NVMLogEngine &NVMLogEngine::operator=(NVMLogEngine &&other) noexcept {
    if (this != &other) {
        log_pool_ = other.log_pool_;
        current_length_ = other.current_length_;
        available_bytes_ = other.available_bytes_;
        other.log_pool_ = nullptr;
    }
    return *this;
}

std::unique_ptr<NVMLogEngine> NVMLogEngine::BuildEngine() {
    auto log_pool = pmemlog_create(kLogPath, kLogSize, 0666);

    if (nullptr == log_pool) {
        log_pool = pmemlog_open(kLogPath);
    }

    if (nullptr == log_pool) {
        return nullptr;
    }

    size_t available_bytes = pmemlog_nbyte(log_pool);

    return std::unique_ptr<NVMLogEngine>(
                new NVMLogEngine(log_pool, available_bytes));
}

RecoveryStatus NVMLogEngine::UpdateOnLog(const Tuple &tuple) {
    if ((current_length_ + sizeof(Tuple)) > available_bytes_) {
        FlushToDisk();
    }
    if (pmemlog_append(log_pool_,
                       reinterpret_cast<const void*>(&tuple),
                       sizeof(Tuple)) < 0) {
        return kFail;
    } else {
        current_length_ += sizeof(Tuple);
        return kSuccess;
    }
}

void NVMLogEngine::FlushToDisk() {
    std::ofstream disk_flush_file(kLogPath, std::ios::app | std::ios::binary);
    if (!disk_flush_file.is_open()) throw std::runtime_error("Coud not open file");
    pmemlog_walk(log_pool_, sizeof(Tuple),
                 [](const void *buf, size_t len, void *arg)->int{
        auto& disk_flush_file = reinterpret_cast<std::ofstream&>(arg);
        disk_flush_file.write(reinterpret_cast<const char*>(buf), len);
    },
    reinterpret_cast<void*>(&disk_flush_file));
    disk_flush_file.flush();
    pmemlog_rewind(log_pool_);
    current_length_ = 0;
}


RecoveryStatus NVMLogEngine::PersistUpdate(uint64_t key,
                                           const std::vector<KVPair> &values) {
    Tuple tuple = DBToRaw(values);
    tuple.key = key;
    return UpdateOnLog(tuple);
}


RecoveryStatus NVMLogEngine::PersistDelete(uint64_t key) {
    Tuple tuple;
    tuple.key = key;
    tuple.deleted = true;
    return UpdateOnLog(tuple);
}

RecoveryStatus NVMLogEngine::Recover(std::map<std::string, Table> &tables) {
    return kSuccess;
}
