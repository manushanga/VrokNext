#pragma once
#include <map>
#include <string>

namespace vrok {
class Metadata {
public:
    using Iterator = std::map<std::string, std::string>::const_iterator;
    static Metadata *Create();
    static void Destory(Metadata *metadata);
    void SetMetadata(const std::string &key, const std::string &value);
    bool GetMetadata(const std::string &key, std::string &value) const;
    Iterator begin() const;
    Iterator end() const;

private:
    std::map<std::string, std::string> _entries;
};
} // namespace vrok
