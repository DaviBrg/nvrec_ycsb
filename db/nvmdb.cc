#include "db/nvmdb.h"

#include <algorithm>
#include <iostream>
#include "recovery/nvmblk_engine.h"
#include "recovery/nvmlog_engine.h"
#include "recovery/nvrec_engine.h"

using namespace ycsbc;

using Tables = std::map<std::string, NVMDB::Table>;

using namespace std;

NVMDB::NVMDB(kNVMDBType type) {
    switch (type) {
    case kNVMBlk:
        rec_engine_= NVMBlkEngine::BuildEngine();
        break;
    case kNVMLog:
        rec_engine_= NVMLogEngine::BuildEngine();
        break;
    case kNVMRec:
        rec_engine_= std::make_unique<NVRecEngine>();
        break;
    default:
        throw std::runtime_error("Unknown NVM database");
        break;
    }
    if (nullptr == rec_engine_) throw std::runtime_error("NVMDB instance error");
    /*rec_engine_->Recover(tables_);
    std::vector<DB::KVPair> result;
    Read("usertable", "8458488093983038188",NULL, result);
    cout << "Tuple check: First: " << result.at(0).first << " Second: " << result.at(0).second << endl;*/
}

auto NVMDB::FindByTableKey(const std::string &table, const std::string &key) {
    auto table_it = tables_.find(table);
    if (std::end(tables_) == table_it) {
        return std::make_tuple(false, Tables::iterator{}, Table::iterator{});
    }
    Table& my_table = table_it->second;
    auto key_it = my_table.find(key);
    if (std::end(my_table) == key_it) {
        return std::make_tuple(false, Tables::iterator{}, Table::iterator{});
    }
    return std::make_tuple(true, table_it, key_it);
}


int NVMDB::Read(const std::string &table, const std::string &key,
                const std::vector<std::string> *fields,
                std::vector<DB::KVPair> &result) {
    std::lock_guard lk{mutex_};
    auto [found, table_it, key_it] = FindByTableKey(table, key);
    if (!found) {
        return kErrorNoData;
    }
    if (nullptr == fields) {
        result = key_it->second;
    }
    else {
        result = FilterByFields(key_it->second, *fields);
    }
    return kOK;
}

int NVMDB::Scan(const std::string &table, const std::string &key,
                int record_count, const std::vector<std::string> *fields,
                std::vector<std::vector<DB::KVPair> > &result) {
    std::lock_guard lk{mutex_};
    auto [found, table_it, key_it] = FindByTableKey(table, key);
    if (!found) {
        return kErrorNoData;
    }
    auto end_it = std::end(table_it->second);
    for (int i = 0; i < record_count; i++) {
        if (key_it == end_it) {
            break;
        }
        result.emplace_back(((key_it++)->second));
    }
}


int NVMDB::Update(const std::string &table, const std::string &key,
                  std::vector<DB::KVPair> &values) {
    std::lock_guard lk{mutex_};
    auto [found, table_it, key_it] = FindByTableKey(table, key);
    if (!found) {
        return kErrorNoData;
    }
    auto &stored_values = (*key_it).second;
    for (const auto& value : values) {
        auto find_it = std::find_if(std::begin(stored_values),
                                    std::end(stored_values),
                                    [&value](const auto& kvpair){
            return kvpair.first == value.first;
        });
        if (std::end(stored_values) != find_it) {
            find_it->second = value.second;
        } else {
            stored_values.emplace_back(value);
        }
    }
    if (rec_engine_->PersistUpdate(std::stoul(key), values) == kSuccess) {
        return kOK;
    } else {
        return kErrorConflict;
    }
}

int NVMDB::Insert(const std::string &table, const std::string &key,
                  std::vector<DB::KVPair> &values) {
    std::lock_guard lk{mutex_};
    tables_[table][key] = values;
    if (rec_engine_->PersistUpdate(std::stoul(key), values) == kSuccess) {
        return kOK;
    } else {
        return kErrorConflict;
    }
}

int NVMDB::Delete(const std::string &table, const std::string &key) {
    std::lock_guard lk{mutex_};
    auto [found, table_it, key_it] = FindByTableKey(table, key);
    if (!found) {
        return kErrorNoData;
    }
    table_it->second.erase(key_it);
    if (rec_engine_->PersistDelete(std::stoul(key)) == kSuccess) {
        return kOK;
    } else {
        return kErrorConflict;
    }
}

std::vector<DB::KVPair> NVMDB::FilterByFields(
        const std::vector<DB::KVPair> &values,
        const std::vector<std::string> &fields) {
    std::vector<DB::KVPair> result;
    for (const auto& value : values) {
        auto it = std::find_if(std::begin(fields), std::end(fields),
                               [&value](const auto& field){
            return value.first == field;
        });
        if (std::end(fields) != it) {
            result.emplace_back(value);
        }
    }

    return result;
}
