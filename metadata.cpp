#include "metadata.h"

void vrok::Metadata::SetMetadata(const std::string &key, const std::string &value) {
    _entries.insert(std::pair<std::string, std::string>(key, value));
}

bool vrok::Metadata::GetMetadata(const std::string &key, std::string &value) const {
    auto it = _entries.find(key);
    if (it != _entries.end()) {
        value = it->second;
        return true;
    }
    return false;
}

vrok::Metadata::Iterator vrok::Metadata::begin() const {
    return _entries.cbegin();
}

vrok::Metadata::Iterator vrok::Metadata::end() const {
    return _entries.cend();
}

vrok::Metadata *vrok::Metadata::Create() {
    return new Metadata();
}

void vrok::Metadata::Destory(vrok::Metadata *metadata) {
    delete metadata;
}
