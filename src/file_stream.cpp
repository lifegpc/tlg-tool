#include "file_stream.h"
#include "fileop.h"
#include <stdexcept>
#include "err.h"

FileStream::FileStream(std::string fileName, std::string mode) {
    file = fileop::fopen(fileName, mode);
    if (!file) {
        throw std::runtime_error("Failed to open file: " + fileName);
    }
}

FileStream::~FileStream() {
    if (file) {
        fclose(file);
        file = nullptr;
    }
}

tjs_uint64 FileStream::Seek(tjs_int64 offset, tjs_int whence) {
    int re = fileop::fseek(file, offset, whence);
    if (re) {
        std::string errmsg;
        if (!err::get_errno_message(errmsg, errno)) {
            errmsg = "Unknown error";
        }
        throw std::runtime_error("Failed to seek in file stream：" + errmsg + "(" + std::to_string(re) + ")");
    }
    auto loc = fileop::ftell(file);
    if (loc == -1) {
        std::string errmsg;
        if (!err::get_errno_message(errmsg, errno)) {
            errmsg = "Unknown error";
        }
        throw std::runtime_error("Failed to get file position: " + errmsg);
    }
    return loc;
}

tjs_uint FileStream::Read(void* buffer, tjs_uint read_size) {
    auto readed = fread(buffer, 1, read_size, file);
    return readed;
}

tjs_uint FileStream::Write(const void* buffer, tjs_uint write_size) {
    auto writed = fwrite(buffer, 1, write_size, file);
    return writed;
}