#ifndef RECOVERY_NVM_RECOVERY_H
#define RECOVERY_NVM_RECOVERY_H

#include <cassert>
#include <cstddef>
#include <cstring>
#include <string>
#include <vector>

using KVPair = std::pair<std::string,std::string>;

enum RecoveryStatus {
    kFail = -1,
    kSuccess = 0
};

constexpr size_t kFieldSize = 100;
constexpr size_t kNumOfFields = 10;

struct Field {
    char str[kFieldSize] = {'\0'};
} __attribute__((packed));

struct RawTuple {
    size_t key = 0;
    Field fields[kNumOfFields] = {'\0'};
} __attribute__((packed));

std::vector<KVPair> RawToDB(const RawTuple &tuple) {
    std::string field = "field";
    std::vector<KVPair> result;
    for (size_t i = 0; i < kNumOfFields; i++) {
        assert(tuple.fields[i].str[kFieldSize] == '\0');
        auto str_len = std::strlen(tuple.fields[i].str);
        if (str_len > 0 ) {
            result.emplace_back(std::make_pair(
                                    field + std::to_string(i),
                                    std::string(
                                        std::string(tuple.fields[i].str,
                                                    str_len)
                                        )
                                    )
                                );
        }
    }
    return result;
}

RawTuple DBToRaw(const std::vector<KVPair> &fields) {
    RawTuple tuple;
    for (const auto &field : fields) {
        size_t i = std::stoi(field.first.substr(field.first.size() - 1, 1));
        assert((i < kNumOfFields) && (field.second.size() < kFieldSize));
        std::strncpy(tuple.fields[i].str,
                     field.second.c_str(), field.second.size());
    }
    return tuple;
}



class NVMRecovery {
public:
    RecoveryStatus Persist(){}
    RecoveryStatus Recover(){}
};

#endif //RECOVERY_NVM_RECOVERY_H
