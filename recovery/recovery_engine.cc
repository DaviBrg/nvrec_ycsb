#include "recovery/recovery_engine.h"

std::vector<KVPair> RawToDB(Tuple &tuple) {
    std::string field = "field";
    std::vector<KVPair> result;
    for (size_t i = 0; i < kNumOfFields; i++) {
        tuple.fields[i].str[kFieldSize - 1] = '\0';
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

Tuple DBToRaw(const std::vector<KVPair> &values) {
    Tuple tuple;
    for (const auto &value : values) {
        size_t i = std::stoi(value.first.substr(value.first.size() - 1, 1));
        assert((i < kNumOfFields) && (value.second.size() < kFieldSize));
        std::strncpy(tuple.fields[i].str,
                     value.second.c_str(), value.second.size());
    }
    return tuple;
}
