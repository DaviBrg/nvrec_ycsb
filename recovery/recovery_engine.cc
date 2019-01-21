#include "recovery/recovery_engine.h"

RecoveryStatus RecoveryEngine::Persist(uint64_t key,
        const std::vector<KVPair> *values) {
    if (nullptr != values) {
        return PersistUpdate(key, *values);
    } else {
        return PersistDelete(key);
    }

}
