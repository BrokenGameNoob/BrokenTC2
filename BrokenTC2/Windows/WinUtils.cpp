#include "WinUtils.hpp"

#include <windows.h>
#include <filesystem>
#include <psapi.h>
#include <tchar.h>
#include <optional>

#include <QDebug>

namespace{

std::string to_string(wchar_t const* wcstr){
    auto s = std::mbstate_t();
    auto const target_char_count = std::wcsrtombs(nullptr, &wcstr, 0, &s);
    if(target_char_count == static_cast<std::size_t>(-1)){
        throw std::logic_error("Illegal byte sequence");
    }

    // +1 because std::string adds a null terminator which isn't part of size
    auto str = std::string(target_char_count, '\0');
    std::wcsrtombs(str.data(), &wcstr, str.size() + 1, &s);
    return str;
}

}

namespace win {

std::string getProcessName(DWORD processID){
    HANDLE Handle = OpenProcess(
                PROCESS_QUERY_INFORMATION | PROCESS_VM_READ,
                FALSE,
                processID /* This is the PID, you can find one from windows task manager */
                );
    if (Handle)
    {
        TCHAR Buffer[MAX_PATH];
        auto success{GetModuleFileNameEx(Handle, 0, Buffer, MAX_PATH)};
        CloseHandle(Handle);
        if(success)
        {
            namespace fs = std::filesystem;
            // At this point, buffer contains the full path to the executable
            std::string tmp{to_string(Buffer)};
            tmp = fs::path(tmp).filename().string();
            return tmp;
        }
        else
        {
            // You better call GetLastError() here
        }
    }
    return {};
}


bool terminateProcess(DWORD processID){
    HANDLE hProcess = OpenProcess( PROCESS_QUERY_INFORMATION |
                                   PROCESS_VM_READ | PROCESS_TERMINATE,
                                   FALSE, processID );
    qDebug() << __PRETTY_FUNCTION__ << "-> kill :" << processID;
    bool success = TerminateProcess(hProcess, 1);

    CloseHandle( hProcess );
    return success;
}


std::optional<DWORD> findProcessId(const QString& pName){
    DWORD aProcesses[1024], cbNeeded, cProcesses;
    unsigned int i;

    if ( !EnumProcesses( aProcesses, sizeof(aProcesses), &cbNeeded ) )
    {
        return {};
    }


    // Calculate how many process identifiers were returned.
    cProcesses = cbNeeded / sizeof(DWORD);

    // Print the name and process identifier for each process.
    for ( i = 0; i < cProcesses; i++ )
    {
        if(aProcesses[i] != 0)
        {
            auto id{aProcesses[i]};

            if(QString::fromStdString(getProcessName(id)) == pName)
            {
                return {id};
            }
        }
    }
    return {};
}

} // namespace win
