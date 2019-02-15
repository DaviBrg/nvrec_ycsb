#ifndef RECOVERY_PERSISTENT_LIST_H
#define RECOVERY_PERSISTENT_LIST_H


#include <fstream>
#include <libpmemobj++/p.hpp>
#include <libpmemobj++/persistent_ptr.hpp>
#include <libpmemobj++/pool.hpp>
#include <memory>
#include <unordered_map>

#include "recovery/recovery_defs.h"


struct ListNode {
        pmem::obj::persistent_ptr<ListNode> next;
        pmem::obj::persistent_ptr<ListNode> prev;
        pmem::obj::p<uint64_t> key;
        pmem::obj::p<Tuple> obj;
};

class PersistentList {
public:
    static pmem::obj::pool<PersistentList> MakePersistentListPool(
            const std::string &pool_path, size_t pool_size,
            const std::string &layout);
    void Persist(pmem::obj::pool<PersistentList> &pool,
            const Tuple &value, std::unordered_map<uint64_t,
            pmem::obj::persistent_ptr<ListNode>> &lookup_table);
    void Recover(std::map<std::string, Table> &tables,
                 std::unordered_map<uint64_t,
                 pmem::obj::persistent_ptr<ListNode>> &lookup_table);
    void Dump();
private:
    PersistentList();

    pmem::obj::persistent_ptr<ListNode> AddNewEntry(const Tuple &entry);
    pmem::obj::persistent_ptr<ListNode> head_;
    pmem::obj::persistent_ptr<ListNode> tail_;
    std::ofstream out_dump_file_;
    std::ifstream in_dump_file_;
    std::string pool_path_;
};


#endif //RECOVERY_PERSISTENT_LIST_H
