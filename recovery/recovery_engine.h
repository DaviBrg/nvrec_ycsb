#ifndef RECOVERY_RECOVERY_ENGINE_H
#define RECOVERY_RECOVERY_ENGINE_H

#include <cassert>
#include <cstddef>
#include <cstring>
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

struct Field {
    char str[kFieldSize] = {'\0'};
} __attribute__((packed));

struct Tuple {
    uint64_t key = 0;
    Field fields[kNumOfFields] = {'\0'};
} __attribute__((packed));

std::vector<KVPair> RawToDB(Tuple &tuple);

Tuple DBToRaw(const std::vector<KVPair> &values);


class RecoveryEngine {
public:
    RecoveryEngine() {}
    virtual RecoveryStatus Persist(const std::vector<KVPair> &values) = 0;
    virtual RecoveryStatus Recover(std::map<std::string, Table> &tables) = 0;
    virtual ~RecoveryEngine() {}
};

#endif //RECOVERY_RECOVERY_ENGINE_H
