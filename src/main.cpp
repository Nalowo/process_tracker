#include <windows.h>
#include <tlhelp32.h>
#include <psapi.h>
#include <tchar.h>

#include <iostream>
#include <vector>
#include <algorithm>

struct ProcessInfo {
    DWORD processId;
    DWORD parentProcessId;
    std::string name;
    FILETIME creationTime;
    FILETIME exitTime;
    FILETIME kernelTime;
    FILETIME userTime;
};

std::vector<ProcessInfo> GetRunningProcesses() {
    std::vector<ProcessInfo> processes;

    DWORD processIds[1024];
    DWORD bytesReturned;

    if (!EnumProcesses(processIds, sizeof(processIds), &bytesReturned)) {
        // Handle error
        return processes;
    }

    int numProcesses = bytesReturned / sizeof(DWORD);

    for (int i = 0; i < numProcesses; i++) {
        ProcessInfo process;
        process.processId = processIds[i];

        HANDLE processHandle = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, processIds[i]);
        if (processHandle != NULL) {
            HMODULE hMod;
            DWORD cbNeeded;

            if (EnumProcessModules(processHandle, &hMod, sizeof(hMod), &cbNeeded)) {
                GetModuleBaseName(processHandle, hMod, process.name.data(), static_cast<DWORD>(process.name.size()));
            }

            FILETIME creationTime, exitTime, kernelTime, userTime;
            if (GetProcessTimes(processHandle, &creationTime, &exitTime, &kernelTime, &userTime)) {
                process.creationTime = creationTime;
                process.exitTime = exitTime;
                process.kernelTime = kernelTime;
                process.userTime = userTime;
            }

            CloseHandle(processHandle);
        }

        processes.push_back(process);
    }

    return processes;
}

void PrintProcessNameAndID( DWORD processID )
{
    TCHAR szProcessName[MAX_PATH] = TEXT("<unknown>");

    // Get a handle to the process.

    HANDLE hProcess = OpenProcess( PROCESS_QUERY_INFORMATION |
                                   PROCESS_VM_READ,
                                   FALSE, processID );

    // Get the process name.

    if (NULL != hProcess )
    {
        HMODULE hMod;
        DWORD cbNeeded;

        if ( EnumProcessModules( hProcess, &hMod, sizeof(hMod), 
             &cbNeeded) )
        {
            GetModuleBaseName( hProcess, hMod, szProcessName, 
                               sizeof(szProcessName)/sizeof(TCHAR) );
        }
    }

    // Print the process name and identifier.

    _tprintf( TEXT("%s  (PID: %u)\n"), szProcessName, processID );

    // Release the handle to the process.

    CloseHandle( hProcess );
}

int main() {
    std::vector<ProcessInfo> processes = GetRunningProcesses();

    // Sort the processes by CPU usage
    std::sort(processes.begin(), processes.end(), [](const ProcessInfo& a, const ProcessInfo& b) {
        return a.kernelTime.dwLowDateTime < b.kernelTime.dwLowDateTime;
    });

    // Print the sorted list of processes
    // for (const auto& process : processes) {
    //     std::cout << "Process ID: " << process.processId << std::endl;
    //     std::cout << "Process Name: " << process.name << std::endl;
    //     std::cout << "CPU Usage: " << process.kernelTime.dwLowDateTime << std::endl;
    //     std::cout << std::endl;
    // }

    for (const auto& process : processes) {
        PrintProcessNameAndID(process.processId);
    }

    return 0;
}