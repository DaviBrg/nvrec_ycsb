#ifndef NVMDB_H
#define NVMDB_H

#include <map>
#include <memory>
#include <mutex>

#include "core/db.h"
#include "recovery/nvrec_engine.h"

namespace ycsbc {

enum kNVMDBType {
    kNVMBlk,
    kNVMLog,
    kNVMRec
};

class NVMDB : public DB {
public:
    NVMDB(kNVMDBType type);
    using Table = std::map<std::string, std::vector<KVPair>>;
    ///
    /// Reads a record from the database.
    /// Field/value pairs from the result are stored in a vector.
    ///
    /// @param table The name of the table.
    /// @param key The key of the record to read.
    /// @param fields The list of fields to read, or NULL for all of them.
    /// @param result A vector of field/value pairs for the result.
    /// @return Zero on success, or a non-zero error code on error/record-miss.

    int Read(const std::string &table, const std::string &key,
                     const std::vector<std::string> *fields,
                     std::vector<KVPair> &result);
    ///
    /// Performs a range scan for a set of records in the database.
    /// Field/value pairs from the result are stored in a vector.
    ///
    /// @param table The name of the table.
    /// @param key The key of the first record to read.
    /// @param record_count The number of records to read.
    /// @param fields The list of fields to read, or NULL for all of them.
    /// @param result A vector of vector, where each vector contains field/value
    ///        pairs for one record
    /// @return Zero on success, or a non-zero error code on error.
    ///
    int Scan(const std::string &table, const std::string &key,
                     int record_count, const std::vector<std::string> *fields,
                     std::vector<std::vector<KVPair>> &result);
    ///
    /// Updates a record in the database.
    /// Field/value pairs in the specified vector are written to the record,
    /// overwriting any existing values with the same field names.
    ///
    /// @param table The name of the table.
    /// @param key The key of the record to write.
    /// @param values A vector of field/value pairs to update in the record.
    /// @return Zero on success, a non-zero error code on error.
    ///
    int Update(const std::string &table, const std::string &key,
                       std::vector<KVPair> &values);
    ///
    /// Inserts a record into the database.
    /// Field/value pairs in the specified vector are written into the record.
    ///
    /// @param table The name of the table.
    /// @param key The key of the record to insert.
    /// @param values A vector of field/value pairs to insert in the record.
    /// @return Zero on success, a non-zero error code on error.
    ///
    int Insert(const std::string &table, const std::string &key,
                       std::vector<KVPair> &values);
    ///
    /// Deletes a record from the database.
    ///
    /// @param table The name of the table.
    /// @param key The key of the record to delete.
    /// @return Zero on success, a non-zero error code on error.
    ///
    int Delete(const std::string &table, const std::string &key);

private:
    auto FindByTableKey(const std::string &table, const std::string &key);
    std::vector<KVPair> FilterByFields(const std::vector<KVPair> &values,
                                       const std::vector<std::string> &fields);
    std::map<std::string, Table> tables_;
    std::mutex mutex_;
    std::unique_ptr<RecoveryEngine> rec_engine_ = nullptr;

};

}


#endif //NVMDB_H
