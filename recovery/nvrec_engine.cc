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

//constexpr const size_t kPoolSize = 250000000;
constexpr const size_t kPoolSize = 62500000;

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
        OSFile osf{kFileName};
        auto current = pool_.get_root()->head();
        Tuple t = current->obj.get_ro();
        std::ostringstream o;
        while(osf.Read(reinterpret_cast<char*>(&t),
                       sizeof(t)) == sizeof(t)){
            o << t.key;
            const std::string str = o.str();
            std::vector<KVPair> fields = RawToDB(t);
            tables["usertable"][str] = fields;

        }

        current = pool_.get_root()->head();
        while (current != nullptr) {
            Tuple current_tuple = current->obj.get_ro();
            o << current_tuple.key;
            const std::string str = o.str();
            std::vector<KVPair> fields = RawToDB(current_tuple);
            //if(conjunto.find(str) == conjunto.end()){
            tables["usertable"][str] = fields;



            current = current->next;
        }
        //pool_.close();
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
        if (flush_result_.valid()) flush_result_.get();
        std::cerr << "<FLUSH STARTED>" << std::endl;
        auto current_pool_path = [this](){
            if (pool_counter_ == 0) return pool_path_;
            else return pool_path_ + std::to_string(pool_counter_);
        }();

        flush_result_ = std::async(std::launch::async,
                                   &NVRecEngine::FlushToDisk,
                                   this,
                                   current_pool_path,
                                   std::move(pool_),
                                   std::move(lookup_table_));
        pool_ = PersistentList::MakePersistentListPool(NextPoolPath(),
                                                       kPoolSize, kLayout);
        lookup_table_.clear();
        return PersistRaw(value);
    }
    return kSuccess;
}

RecoveryStatus NVRecEngine::FlushToDisk(std::string pool_path,
                                        pmem::obj::pool<PersistentList> pool,
                                        std::unordered_map<uint64_t,
                                        pmem::obj::persistent_ptr<ListNode>>
                                        lookup_table) {
    auto before = std::chrono::high_resolution_clock::now();
    lookup_table.clear();
    auto current = pool.get_root()->head();
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
    pool.close();
    if (unlink(pool_path.c_str()) == -1) throw std::runtime_error("Cannot delete pool file: " + std::string{strerror(errno)} + "\n File name: " + pool_path );
    auto after = std::chrono::high_resolution_clock::now();
    std::cerr << "<FLUSH FINISHED> DURARION: " <<
                 std::chrono::duration_cast<std::chrono::milliseconds>( after - before).count() << "ms" << std::endl;
    return kSuccess;
}

std::string NVRecEngine::NextPoolPath() {
    return pool_path_ + std::to_string(++pool_counter_);
}
