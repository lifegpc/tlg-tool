#include <string>
#include <map>

class DictFile {
public:
    DictFile(std::string filepath);
    std::map<std::string, std::string> maps;
    bool HasError = false;
};