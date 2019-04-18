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

enum PListStatus {
    kPersisted,
    kAllocError
};

class PersistentList {
public:
    static pmem::obj::pool<PersistentList> MakePersistentListPool(
            const std::string &pool_path, size_t pool_size,
            const std::string &layout);
//    PListStatus Persist(pmem::obj::pool<PersistentList> &pool,
//            const Tuple &value, std::unordered_map<uint64_t,
//            pmem::obj::persistent_ptr<ListNode>> &lookup_table);
//    void Recover(std::map<std::string, Table> &tables,
//                 std::unordered_map<uint64_t,
//                 pmem::obj::persistent_ptr<ListNode>> &lookup_table);
    pmem::obj::persistent_ptr<ListNode> head() {return head_;}
    pmem::obj::persistent_ptr<ListNode> tail() {return tail_;}
    pmem::obj::persistent_ptr<ListNode> Insert(const Tuple &entry);
private:
    PersistentList();

    pmem::obj::persistent_ptr<ListNode> head_;
    pmem::obj::persistent_ptr<ListNode> tail_;
    std::ifstream in_dump_file_;
    std::string pool_path_;
};


#endif //RECOVERY_PERSISTENT_LIST_H
