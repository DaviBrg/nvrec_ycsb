#include "db/nvmdb.h"

#include <algorithm>

using namespace ycsbc;

using Tables = std::map<std::string, NVMDB::Table>;

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
}

int NVMDB::Insert(const std::string &table, const std::string &key,
                  std::vector<DB::KVPair> &values) {
    std::lock_guard lk{mutex_};
    tables_[table][key] = values;
}

int NVMDB::Delete(const std::string &table, const std::string &key) {
    std::lock_guard lk{mutex_};
    auto [found, table_it, key_it] = FindByTableKey(table, key);
    if (!found) {
        return kErrorNoData;
    }
    table_it->second.erase(key_it);
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



