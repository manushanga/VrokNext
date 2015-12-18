#include "notify.h"

void Vrok::Notify::NotifyError(int level, std::string msg)
{
    if (level >= _elevel)
        _notifier->OnError(level, msg);
}

void Vrok::Notify::NotifyWarning(int level, std::string msg)
{
    if (level >= _wlevel)
        _notifier->OnWarning(level, msg);
}

void Vrok::Notify::NotifyInformation(std::string msg)
{
    _notifier->OnInformation(msg);
}

void Vrok::Notify::NotifyDebug(int level, std::string msg)
{
    if (level >= _dlevel)
        _notifier->OnDebug(level, msg);
}
