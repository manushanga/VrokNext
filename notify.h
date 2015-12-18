#pragma once

#include <string>

namespace Vrok
{
class Notify
{
public:
    class Notifier
    {
    public:
        virtual void OnError(int level, std::string msg) {}
        virtual void OnWarning(int level, std::string msg) {}
        virtual void OnInformation(std::string msg) {}
        virtual void OnDebug(int level, std::string msg) {}
    };
    Notify() :
        _elevel(0),
        _wlevel(0),
        _dlevel(0),
        _notifier(nullptr)
    {}
    static Notify *GetInstance()
    {
        static Notify notify;
        return &notify;
    }
    void setNotifier(Notifier *notifier) { _notifier = notifier; }
    void setErrorLevel(int level) { _elevel = level; }
    void setWarningLevel(int level) { _wlevel = level; }
    void setDebugLevel(int level) { _dlevel = level; }
    void NotifyError(int level, std::string msg);
    void NotifyWarning(int level, std::string msg);
    void NotifyInformation(std::string msg);
    void NotifyDebug(int level, std::string msg);

private:
    Notifier *_notifier;
    int _elevel, _dlevel, _wlevel;
};

}
