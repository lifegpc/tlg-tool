#include "dict_file.h"
#include "fileop.h"
#include "file_reader.h"
#include <malloc.h>
#include "str_util.h"

DictFile::DictFile(std::string path) {
    FILE* fp = fileop::fopen(path, "rb");
    if (!fp) {
        HasError = true;
        return;
    }
    auto reader = create_file_reader(fp, 0);
    if (!reader) {
        HasError = true;
        fclose(fp);
        return;
    }
    char* line = nullptr;
    size_t line_size;
    while (!file_reader_read_line(reader, &line, &line_size)) {
        std::string l(line, line_size);
        free(line);
        auto re = str_util::str_splitv(l, "=", 2);
        if (re.size() > 1) {
            maps[re[0]] = re[1];
        }
    }
    free_file_reader(reader);
    fclose(fp);
}
