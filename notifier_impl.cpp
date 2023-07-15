#include "notifier_impl.h"
#include <iostream>

void CNotifier::OnError(int level, std::string msg) {
    _guard.lock();
    std::cout << "ERR: " << msg << std::endl;
    _guard.unlock();
}
void CNotifier::OnWarning(int level, std::string msg) {
    _guard.lock();
    std::cout << "WRN: " << msg << std::endl;
    _guard.unlock();
}
void CNotifier::OnDebug(int level, std::string msg) {
    _guard.lock();
    std::cout << "DBG: " << msg << std::endl;
    _guard.unlock();
}
void CNotifier::OnInformation(std::string msg) {
    _guard.lock();
    std::cout << "INF: " << msg << std::endl;
    _guard.unlock();
}
