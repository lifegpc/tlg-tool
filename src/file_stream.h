#include "stream.h"
#include <string>
#include <stdio.h>

class FileStream : public tTJSBinaryStream {
public:
    /**
     * @brief Opens a file stream with the specified file name and mode. If the file cannot be opened, it throws a runtime error.
     * @param fileName File name
     * @param mode Mode used by fopen
     */
    FileStream(std::string fileName, std::string mode);
    ~FileStream();
    virtual tjs_uint64 Seek(tjs_int64 offset, tjs_int whence) override;
    virtual tjs_uint Read(void* buffer, tjs_uint read_size) override;
    virtual tjs_uint Write(const void* buffer, tjs_uint write_size) override;
private:
    FILE* file = nullptr;
};
