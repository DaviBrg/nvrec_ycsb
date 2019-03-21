#ifndef RECOVERY_OS_FILE_H
#define RECOVERY_OS_FILE_H

#include <string_view>

class OSFile {
public:
    explicit OSFile(std::string_view path);
    ~OSFile();
    OSFile(const OSFile&) = delete;
    OSFile& operator=(const OSFile&) = delete;

    OSFile(OSFile &&other);
    OSFile& operator=(OSFile &&other);

    int Write(const char *data, size_t size);

    int Sync();


private:
    void Move(OSFile &other);
    int fd_ = -1;

};

#endif //RECOVERY_OS_FILE_H
