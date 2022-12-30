#ifndef WIN_WINUTILS_HPP
#define WIN_WINUTILS_HPP

#include <QString>

#include <windows.h>

#include <vector>

namespace win {

std::string getProcessName(DWORD processID);

bool terminateProcess(DWORD processID);

std::optional<DWORD> findProcessId(const QString& pName);

inline
bool isProcessRunning(const QString& pName){
    return findProcessId(pName).has_value();
}

int32_t processCount(const QString& pName);
std::vector<DWORD> findProcessesId(const QString& pName);

} // namespace win

#endif // WIN_WINUTILS_HPP
