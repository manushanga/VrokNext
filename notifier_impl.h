#pragma once
#include <mutex>

#include "notify.h"

class CNotifier : public Vrok::Notify::Notifier
{
public:
    void OnError(int level, std::string msg);

    void OnWarning(int level, std::string msg);

    void OnDebug(int level, std::string msg);

    void OnInformation(std::string msg);
private:
    std::mutex _guard;
};
