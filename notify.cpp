#include "notify.h"

void vrok::Notify::NotifyError(int level, std::string msg) {
    if (!_notifier)
        return;

    if (level >= _elevel)
        _notifier->OnError(level, msg);
}

void vrok::Notify::NotifyWarning(int level, std::string msg) {
    if (!_notifier)
        return;
    if (level >= _wlevel)
        _notifier->OnWarning(level, msg);
}

void vrok::Notify::NotifyInformation(std::string msg) {
    if (!_notifier)
        return;
    _notifier->OnInformation(msg);
}

void vrok::Notify::NotifyDebug(int level, std::string msg) {
    if (!_notifier)
        return;
    if (level >= _dlevel)
        _notifier->OnDebug(level, msg);
}
