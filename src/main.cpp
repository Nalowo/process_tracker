#define _WIN32_WINNT 0x0600

#include <sdkddkver.h>
#include <windows.h>
// #include <tlhelp32.h>
#include <realtimeapiset.h>
#include <tchar.h>
#include <psapi.h>

#include <boost/asio/steady_timer.hpp>

#include <iostream>
#include <vector>
#include <algorithm>
#include <string>

namespace net = boost::asio;
namespace sys = boost::system;

// struct ProcessInfo
// {
//     DWORD processId; // Идентификатор процесса
//     DWORD parentProcessId; // Идентификатор родительского процесса
//     // std::string name;
//     TCHAR name[MAX_PATH]; // Имя процесса
//     FILETIME creationTime; // Время создания процесса
//     FILETIME exitTime; // Время завершения процесса
//     FILETIME kernelTime; // Время работы процессора в режиме ядра
//     FILETIME userTime; // Время работы процессора в режиме пользователя
//     SYSTEMTIME cpuTime; // сюда хотелось бы записать общее время работы процессора
// };

// std::vector<ProcessInfo> GetRunningProcesses()
// {
//     std::vector<ProcessInfo> processes; // Список процессов

//     DWORD processIds[1024]; // Массив идентификаторов процессов
//     DWORD bytesReturned; // Количество байтов, выделенных под идентификаторы процессов

//     if (!EnumProcesses(processIds, sizeof(processIds), &bytesReturned))
//     {
//         // Handle error
//         return processes;
//     }

//     int numProcesses = bytesReturned / sizeof(DWORD);

//     for (int i = 0; i < numProcesses; i++)
//     {
//         ProcessInfo process;
//         process.processId = processIds[i];

//         HANDLE processHandle = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, processIds[i]);
//         if (processHandle != NULL)
//         {
//             HMODULE hMod;
//             DWORD cbNeeded;

//             if (EnumProcessModules(processHandle, &hMod, sizeof(hMod), &cbNeeded))
//             {
//                 // GetModuleBaseName(processHandle, hMod, process.name.data(), static_cast<DWORD>(process.name.size()));
//                 GetModuleBaseName(processHandle, hMod, process.name, sizeof(process.name) / sizeof(TCHAR));
//             }

//             FILETIME creationTime, exitTime, kernelTime, userTime;
//             if (GetProcessTimes(processHandle, &creationTime, &exitTime, &kernelTime, &userTime))
//             {
//                 process.creationTime = creationTime;
//                 process.exitTime = exitTime;
//                 process.kernelTime = kernelTime;
//                 process.userTime = userTime;
//                 FileTimeToSystemTime(&process.kernelTime, &process.cpuTime);
//             }

//             CloseHandle(processHandle);
//         }

//         processes.push_back(process);
//     }

//     return processes;
// }

struct ProcessInfo
{
    DWORD processId;      // Идентификатор процесса
    TCHAR name[MAX_PATH]; // Имя процесса
    size_t cpuCycles = 0;
};

size_t GetCpuUsageCycles(DWORD pid)
{
    DWORD processId{pid};
    ULONG64 CycleTime1{0};

    HANDLE hProcess = OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION, FALSE, processId);
    if (hProcess == NULL)
    {
        return 0;
    }

    QueryProcessCycleTime(hProcess, &CycleTime1);
    CloseHandle(hProcess);

    return CycleTime1;
}

void PrintProcessNameAndID(DWORD processID)
{
    TCHAR szProcessName[MAX_PATH] = TEXT("<unknown>");

    // Get a handle to the process.
    HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION |
                                      PROCESS_VM_READ,
                                  FALSE, processID);

    // Get the process name.
    if (NULL != hProcess)
    {
        HMODULE hMod;
        DWORD cbNeeded;

        if (EnumProcessModules(hProcess, &hMod, sizeof(hMod),
                               &cbNeeded))
        {
            GetModuleBaseName(hProcess, hMod, szProcessName,
                              sizeof(szProcessName) / sizeof(TCHAR));
        }
    }

    // Print the process name and identifier.
    _tprintf(TEXT("%s  (PID: %u)\n"), szProcessName, processID);

    // Release the handle to the process.
    CloseHandle(hProcess);
}

std::vector<ProcessInfo> GetCpuUsageList()
{
    std::vector<ProcessInfo> processes;

    DWORD processIds[1024]; // Массив идентификаторов процессов
    DWORD bytesReturned;    // Количество байтов, выделенных под идентификаторы процессов

    if (!EnumProcesses(processIds, sizeof(processIds), &bytesReturned))
    {
        throw std::runtime_error("EnumProcesses failed");
    }

    int numProcesses = bytesReturned / sizeof(DWORD);
    processes.reserve(numProcesses);

    for (int i = 0; i < numProcesses; i++)
    {
        ProcessInfo process;
        process.processId = processIds[i];

        HANDLE processHandle = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, processIds[i]);
        if (processHandle != NULL)
        {
            HMODULE hMod;
            DWORD cbNeeded;

            if (EnumProcessModules(processHandle, &hMod, sizeof(hMod), &cbNeeded))
            {
                GetModuleBaseName(processHandle, hMod, process.name, sizeof(process.name) / sizeof(TCHAR));
            }
        }
    }

    return processes;
}

int main()
{
    // std::vector<ProcessInfo> processes = GetRunningProcesses();

    // // Sort the processes by CPU usage
    // std::sort(processes.begin(), processes.end(), [](const ProcessInfo &a, const ProcessInfo &b)
    //           { return a.kernelTime.dwLowDateTime > b.kernelTime.dwLowDateTime; });

    // std::cout << "Total number of processes: " << processes.size() << std::endl;

    // // Print the sorted list of processes
    // for (const auto &process : processes)
    // {
    //     std::cout << "Process ID: " << process.processId << std::endl;
    //     // std::cout << "Process Name: " << process.name << std::endl;
    //     _tprintf(TEXT("Process Name: %s \n"), process.name);
    //     std::cout << "CPU Usage: " << process.kernelTime.dwLowDateTime << std::endl;
    //     std::cout << std::endl;
    // }

    return 0;
}