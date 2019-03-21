#include "recovery/persistent_list.h"

#include <libpmemobj++/make_persistent.hpp>
#include <libpmemobj++/transaction.hpp>
#include <experimental/filesystem>
#include <chrono>

#include <iostream>

#include "recovery/os_file.h"

int my_counter = 0;

const std::string kFileName = "disk_log.txt";


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
        std::cerr << "Flush triggered" << std::endl;
        Dump(pool, lookup_table);
        std::cerr << "Flush finished" << std::endl;
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

void PersistentList::Dump(pmem::obj::pool<PersistentList> &pool,
                          std::unordered_map<uint64_t,
                          pmem::obj::persistent_ptr<ListNode> > &lookup_table) {
    auto before = std::chrono::high_resolution_clock::now();
    auto current = head_;
//    std::ofstream out_dump_file_(kFileName.c_str(),
//                                std::ios::app | std::ios::binary);
//    assert(out_dump_file_.is_open());
    OSFile osf{kFileName};
    while (current != nullptr) {
        Tuple current_tuple = current->obj.get_ro();
        osf.Write(reinterpret_cast<char*>(&current_tuple), sizeof(current_tuple));
//        out_dump_file_.write(reinterpret_cast<char*>(&current_tuple),
//                             sizeof(current_tuple));
        current = current->next;
    }
    osf.Sync();
//    out_dump_file_.flush();
    auto prev = current = head_;

    while (current != nullptr) {
        prev = current;
        current = current->next;
        pmem::obj::transaction::exec_tx(pool, [&](){
            pmem::obj::delete_persistent<ListNode>(prev);
        });
    }
    pmem::obj::transaction::exec_tx(pool, [&](){
        head_ = tail_ = nullptr;
    });
    lookup_table.clear();
    auto after = std::chrono::high_resolution_clock::now();
    std::cerr << "Flush duration: " <<
            std::chrono::duration_cast<std::chrono::milliseconds>(
                     after - before).count() << std::endl;
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
