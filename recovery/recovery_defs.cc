#include "recovery/recovery_defs.h"

#include <cassert>
#include <cstring>


std::vector<KVPair> RawToDB(const Tuple &tuple) {
    std::string field = "field";
    std::vector<KVPair> result;
    for (size_t i = 0; i < kNumOfFields; i++) {
//        tuple.fields[i].str[kFieldSize - 1] = '\0';
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
        auto sz1 = kDefautFieldName.size();
        auto sz2 = value.first.size() - kDefautFieldName.size();
        auto substr = value.first.substr(kDefautFieldName.size(),
                                         value.first.size() -
                                         kDefautFieldName.size());
        size_t i = std::stoi(value.first.substr(kDefautFieldName.size(),
                                                value.first.size() -
                                                kDefautFieldName.size()));
        assert((i < kNumOfFields) && (value.second.size() <= kFieldSize));
        std::strncpy(tuple.fields[i].str,
                     value.second.c_str(), value.second.size());
    }
    return tuple;
}
