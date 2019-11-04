
#ifndef RECOVERY_RECOVERY_ENGINE_H
#define RECOVERY_RECOVERY_ENGINE_H

#include "recovery_defs.h"



class RecoveryEngine {
public:
    RecoveryEngine() {}
    RecoveryEngine(RecoveryEngine &&) = default;
    RecoveryEngine& operator=(RecoveryEngine &&) = default;
    virtual ~RecoveryEngine() {}
    virtual RecoveryStatus PersistUpdate(uint64_t key,
                                 const std::vector<KVPair>& values) = 0;
    virtual RecoveryStatus PersistDelete(uint64_t key) = 0;
    virtual RecoveryStatus Recover(std::map<std::string, Table> &tables) = 0;
};


#endif //RECOVERY_RECOVERY_ENGINE_H
