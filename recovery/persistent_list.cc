#include "recovery/persistent_list.h"

#include <libpmemobj++/make_persistent.hpp>
#include <libpmemobj++/transaction.hpp>
#include <experimental/filesystem>

const std::string kFileName = "log_file.txt";


namespace fs = std::experimental::filesystem;


PersistentList::PersistentList() {}

pmem::obj::pool<PersistentList> PersistentList::MakePersistentListPool(
        const std::string &pool_path,
        size_t pool_size, const std::string &layout) {
        pmem::obj::pool<PersistentList> pool;
    if (fs::exists(pool_path)) {
        pool =
                pmem::obj::pool<PersistentList>::open(pool_path.c_str(),
                                                      layout.c_str());

    } else {
        pool =
                pmem::obj::pool<PersistentList>::create(
                    pool_path.c_str(), layout.c_str(), pool_size,
                    S_IWUSR | S_IRUSR);
    }
    return pool;
}

void PersistentList::Persist(pmem::obj::pool<PersistentList> &pool,
                             const Tuple &value, std::unordered_map<uint64_t,
                             pmem::obj::persistent_ptr<ListNode> > &lookup_table) {
    try {
        pmem::obj::transaction::exec_tx(pool, [&](){
            auto it = lookup_table.find(value.key);
            if (std::end(lookup_table) == it) {
                auto ptr = AddNewEntry(value);
                lookup_table[value.key] = ptr;
            } else {
                it->second->obj = value;
            }
        });
    } catch (const std::exception &e) {
        Dump();
        Persist(pool, value, lookup_table);
    }
}

void PersistentList::Recover(
        std::map<std::string, Table> &tables, std::unordered_map<uint64_t,
        pmem::obj::persistent_ptr<ListNode> > &lookup_table) {
    auto current = head_;
    while (current != nullptr) {
        tables[kDefautTableName][std::to_string(current->obj.get_ro().key)] =
                RawToDB(current->obj.get_ro());
        lookup_table.insert({current->key.get_ro(), current});
        current = current->next;
    }
}

void PersistentList::Dump() {
    auto current = head_;
    decltype(current) prev = nullptr;
    out_dump_file_.open(kFileName,std::ios::app | std::ios::binary);
    assert(out_dump_file_.is_open());
    while (current != tail_) {
        Tuple current_tuple = current->obj.get_ro();
        out_dump_file_.write(reinterpret_cast<char*>(&current_tuple),
                             sizeof(current_tuple));
        prev = current;
        current = current->next;
    }
    out_dump_file_.flush();
}

pmem::obj::persistent_ptr<ListNode> PersistentList::AddNewEntry(
        const Tuple &entry) {
    auto entry_node = pmem::obj::make_persistent<ListNode>();
    entry_node->key = entry.key;
    entry_node->obj = entry;
    if (head_ == nullptr) {
        head_ = tail_ = entry_node;
    }
    else {
        entry_node->next = head_;
        head_->prev = entry_node;
        head_ = entry_node;
    }
    return entry_node;
}
