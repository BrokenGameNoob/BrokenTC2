#ifndef WIN_WINUTILS_HPP
#define WIN_WINUTILS_HPP

#include <QString>

#include <windows.h>

namespace win {

std::string getProcessName(DWORD processID);

bool terminateProcess(DWORD processID);

std::optional<DWORD> findProcessId(const QString& pName);

inline
bool isProcessRunning(const QString& pName){
    return findProcessId(pName).has_value();
}

} // namespace win

#endif // WIN_WINUTILS_HPP
