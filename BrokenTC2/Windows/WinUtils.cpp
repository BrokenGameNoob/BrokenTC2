#include "WinUtils.hpp"

#include <QDebug>
#include <bitset>
#include <filesystem>
#include <iostream>
#include <optional>
#include <psapi.h>
#include <tchar.h>
#include <windows.h>

namespace {

std::string to_string(wchar_t const* wcstr) {
  auto s = std::mbstate_t();
  auto const target_char_count = std::wcsrtombs(nullptr, &wcstr, 0, &s);
  if (target_char_count == static_cast<std::size_t>(-1)) {
    throw std::logic_error("Illegal byte sequence");
  }

  // +1 because std::string adds a null terminator which isn't part of size
  auto str = std::string(target_char_count, '\0');
  std::wcsrtombs(str.data(), &wcstr, str.size() + 1, &s);
  return str;
}

template <typename T>
std::bitset<sizeof(T) * 8> tob(const T& v) {
  std::bitset<sizeof(T) * 8> rVal{v};
  return rVal;
}

template <typename T>
QString tobs(const T& v) {
  auto bin{tob(v)};
  return QString::fromStdString(bin.to_string());
}

}  // namespace

namespace win {

std::string getProcessName(DWORD processID) {
  HANDLE Handle = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ,
                              FALSE,
                              processID /* This is the PID, you can find one from windows task manager */
  );
  if (Handle) {
    TCHAR Buffer[MAX_PATH];
    auto success{GetModuleFileNameEx(Handle, 0, Buffer, MAX_PATH)};
    CloseHandle(Handle);
    if (success) {
      namespace fs = std::filesystem;
      // At this point, buffer contains the full path to the executable
      std::string tmp{to_string(Buffer)};
      tmp = fs::path(tmp).filename().string();
      return tmp;
    } else {
      // You better call GetLastError() here
    }
  }
  return {};
}

bool terminateProcess(DWORD processID) {
  HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ | PROCESS_TERMINATE, FALSE, processID);
  qDebug() << __PRETTY_FUNCTION__ << "-> kill :" << processID;
  bool success = TerminateProcess(hProcess, 1);

  CloseHandle(hProcess);
  return success;
}

std::optional<DWORD> findProcessId(const QString& pName) {
  DWORD aProcesses[1024], cbNeeded, cProcesses;
  unsigned int i;

  if (!EnumProcesses(aProcesses, sizeof(aProcesses), &cbNeeded)) {
    return {};
  }

  // Calculate how many process identifiers were returned.
  cProcesses = cbNeeded / sizeof(DWORD);

  // Print the name and process identifier for each process.
  for (i = 0; i < cProcesses; i++) {
    if (aProcesses[i] != 0) {
      auto id{aProcesses[i]};

      if (QString::fromStdString(getProcessName(id)) == pName) {
        return {id};
      }
    }
  }
  return {};
}

std::vector<DWORD> findProcessesId(const QString& pName) {
  std::vector<DWORD> out{};
  out.reserve(100);

  DWORD aProcesses[1024], cbNeeded, cProcesses;
  unsigned int i;

  if (!EnumProcesses(aProcesses, sizeof(aProcesses), &cbNeeded)) {
    return {};
  }

  // Calculate how many process identifiers were returned.
  cProcesses = cbNeeded / sizeof(DWORD);

  // Print the name and process identifier for each process.
  for (i = 0; i < cProcesses; i++) {
    auto id{aProcesses[i]};
    if (id != 0) {
      if (QString::fromStdString(getProcessName(id)) == pName) {
        out.emplace_back(aProcesses[i]);
      }
    }
  }
  out.shrink_to_fit();
  return out;
}

std::vector<DWORD> getAllProcesses() {
  std::vector<DWORD> out{};
  out.reserve(300);

  DWORD aProcesses[1024], cbNeeded, cProcesses;
  unsigned int i;

  if (!EnumProcesses(aProcesses, sizeof(aProcesses), &cbNeeded)) {
    return {};
  }

  // Calculate how many process identifiers were returned.
  cProcesses = cbNeeded / sizeof(DWORD);

  // Print the name and process identifier for each process.
  for (i = 0; i < cProcesses; i++) {
    auto id{aProcesses[i]};
    if (id != 0) {
      out.emplace_back(id);
    }
  }
  out.shrink_to_fit();
  return out;
}

int32_t processCount(const QString& pName) {
  return static_cast<int32_t>(findProcessesId(pName).size());
}

int32_t getCoreCount() {
  SYSTEM_INFO SystemInfo;
  GetSystemInfo(&SystemInfo);
  return static_cast<int32_t>(SystemInfo.dwNumberOfProcessors);
}

bool setCoreCountAffinity(int32_t coreCountToUse, bool throwOnFail, bool verbose) {
  auto lambdaFail{[&](const QString& err) -> bool {
    if (throwOnFail) {
      throw std::runtime_error(err.toStdString());
    }
    return false;
  }};

  if (coreCountToUse > getCoreCount() || coreCountToUse < 0) {
    QString err{QString{"%0 : Invalid coreCountToUse=%1, its value must be 0 < coreCountToUse > %2"}
                    .arg(__PRETTY_FUNCTION__)
                    .arg(coreCountToUse)
                    .arg(getCoreCount())};
    return lambdaFail(err);
  }

  HANDLE hProcess = GetCurrentProcess();
  ULONG_PTR appAff;
  ULONG_PTR sysAff;
  if (!GetProcessAffinityMask(hProcess, &appAff, &sysAff))
    return lambdaFail(QString{"%0 : Can't get processor core affinity"}.arg(__PRETTY_FUNCTION__));

  ULONG_PTR aff{};
  for (int32_t i{}; i < coreCountToUse; ++i) {
    aff = (aff << 1u) | 1u;
    //        qDebug() << "   " << aff;
  }
  ULONG_PTR curAff = aff & appAff;

  //    qDebug() << tobs(appAff) << "  " << appAff;
  //    qDebug() << tobs(sysAff) << "  " << sysAff;
  //    qDebug() << tobs(aff) << "  " << aff;
  //    qDebug() << tobs(curAff);

  if (!curAff) {
    CloseHandle(hProcess);
    if (throwOnFail) {
      QString err{QString{"%0 : Invalid processor core affinity mask <%1> | app : <%2> | system : <%3>"}.arg(
          __PRETTY_FUNCTION__, tobs(aff), tobs(appAff), tobs(sysAff))};
      return lambdaFail(err);
    }
    return false;
  }

  auto success{SetProcessAffinityMask(hProcess, curAff)};
  CloseHandle(hProcess);

  if (!success) {
    QString err{
        QString{"%0 : Can't set processor core affinity to <%1> cores"}.arg(__PRETTY_FUNCTION__).arg(coreCountToUse)};
    return lambdaFail(err);
  }

  if (verbose) {
    qDebug() << __PRETTY_FUNCTION__ << " : Successfully set core count affinity to" << coreCountToUse << "cores";
  }

  return true;
}

QString vkCodeToStr(int32_t keyCode) {
  static bool firstCall{true};
  static bool canProceed{false};

  static HKL lpList[2];
  static HKL kb;
  static BYTE uKeyboardState[256];

  if (firstCall) {
    auto kbCount = GetKeyboardLayoutList(2, lpList);
    if (kbCount != 0) {
      canProceed = true;
    } else {
      return {};
    }
    kb = lpList[0];

    // Initialization of KeyBoardState
    for (int i = 0; i < 256; ++i) {
      uKeyboardState[i] = 0;
    }
  }
  if (!canProceed) return {};

  auto ScanCode = MapVirtualKeyExW(keyCode, MAPVK_VK_TO_VSC, kb);
  auto VKCode2 = MapVirtualKeyExW(ScanCode, MAPVK_VSC_TO_VK, kb);
  TCHAR ch1 = MapVirtualKeyExW(VKCode2, MAPVK_VK_TO_CHAR, kb);
  std::ignore = ch1;

  TCHAR buffer[1024];
  auto charCount = ToUnicodeEx(keyCode, ScanCode, uKeyboardState, buffer, 1024, 0, kb);
  if (charCount < 1) {
    return {};
  } else if (charCount > 2) {
    QString out{};
    for (int i{}; i < charCount; ++i) {
      out.append(buffer[i]);
    }
    return out;
  }
  return QString{buffer[0]};
}

bool IsNumLockEnabled() {
  // Get the state of the Num Lock key
  return (GetKeyState(VK_NUMLOCK) & 0x0001) != 0;
}

void SetNumLock(bool enable) {
  // Check the current state of Num Lock
  const bool kNumLockEnabled{IsNumLockEnabled()};

  // If the current state is different from the desired state, simulate a key press
  if (kNumLockEnabled != enable) {
    // Simulate a key press
    keybd_event(VK_NUMLOCK, 0x45, KEYEVENTF_EXTENDEDKEY | 0, 0);
    keybd_event(VK_NUMLOCK, 0x45, KEYEVENTF_EXTENDEDKEY | KEYEVENTF_KEYUP, 0);
  }
}

}  // namespace win
