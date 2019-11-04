#include "recovery/os_file.h"

#include <fcntl.h>
#include <stdexcept>
#include <unistd.h>

#include <iostream>

OSFile::OSFile(std::string_view path) {
    fd_ = open(path.data(), O_WRONLY | O_CREAT | O_APPEND, S_IRWXG);
    if (-1 == fd_) {
        throw std::runtime_error("File Open Failed");
    }
}

OSFile::~OSFile() {
    if (-1 != fd_) {
        close(fd_);
    }
}

inline void OSFile::Move(OSFile &other) {
    fd_ = other.fd_;
    other.fd_ = -1;
}

OSFile::OSFile(OSFile &&other) {
    Move(other);
}

OSFile &OSFile::operator=(OSFile &&other) {
    if (this != &other) {
        Move(other);
    }
    return *this;
}

int OSFile::Write(const char *data, size_t size) {
    return write(fd_, data, size);
}

int OSFile::Read(char *data, size_t size) {
    return read(fd_, data, size);
}

int OSFile::Sync() {
    fsync(fd_);
}
