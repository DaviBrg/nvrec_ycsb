#ifndef RECOVERY_RECOVERY_DEFS_H
#define RECOVERY_RECOVERY_DEFS_H


#include <map>
#include <string>
#include <vector>

using KVPair = std::pair<std::string,std::string>;
using Table = std::map<std::string, std::vector<KVPair>>;

enum RecoveryStatus {
    kFail = -1,
    kSuccess = 0
};

constexpr size_t kFieldSize = 100;
constexpr size_t kNumOfFields = 10;
const std::string kDefautTableName = "useratble";

struct Field {
    char str[kFieldSize] = {'\0'};
} __attribute__((packed));

struct Tuple {
    bool deleted = false;
    uint64_t key = 0;
    Field fields[kNumOfFields] = {'\0'};
} __attribute__((packed));

std::vector<KVPair> RawToDB(const Tuple &tuple);

Tuple DBToRaw(const std::vector<KVPair> &values);

#endif //RECOVERY_RECOVERY_DEFS_H
