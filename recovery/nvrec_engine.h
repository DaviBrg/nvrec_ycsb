#ifndef RECOVERY_NVREC_ENGINE_H
#define RECOVERY_NVREC_ENGINE_H

#include "recovery/recovery_engine.h"

class NVRecEngine : public RecoveryEngine {
public:
    virtual RecoveryStatus PersistUpdate(uint64_t key,
                                 const std::vector<KVPair>& values);
    virtual RecoveryStatus PersistDelete(uint64_t key);
    RecoveryStatus Recover(std::map<std::string, Table> &tables);
private:
    NVRecEngine();
};


#endif //RECOVERY_NVREC_ENGINE_H
