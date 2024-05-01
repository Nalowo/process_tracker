#define _WIN32_WINNT 0x0600

#include <sdkddkver.h>
#include <realtimeapiset.h>
#include <tchar.h>
#include <psapi.h>

#include <boost/asio/steady_timer.hpp>

#include <iostream>
#include <vector>
#include <algorithm>
#include <string>
#include <numeric>
#include <execution>

namespace net = boost::asio;
namespace sys = boost::system;

static uint32_t _S_timeout_sec = 1;

/**
 * @brief Struct for process info
 *
 */
struct ProcessInfo
{
    DWORD processId;               // process id
    TCHAR name[MAX_PATH];          // process name
    size_t cpuCycles1 = 0;         // col in cycles of process on first launch
    size_t cpuCycles2 = 0;         // col in cycles of process on second launch
    size_t cpuCyclesDif = 0;       // difference between first and second launch
    uint32_t cpuCyclesDifPerc = 0; // precentage of difference between first and second launch
};

/**
 * @brief Print process vector
 *
 * @param processes vector of processes
 */
void PrintProcessVec(const std::vector<ProcessInfo> &processes)
{
    for (const auto &process : processes)
    {
        _tprintf(TEXT("%s %u\% (PID: %u)\n"), process.name, process.cpuCyclesDifPerc, process.processId);
    }
}

/**
 * @brief Get cpu usage cycles
 *
 * @param hProcess process descriptor
 * @return size_t count of cpu usage cycles
 */
size_t GetCpuUsageCycles(const HANDLE &hProcess)
{
    if (hProcess == NULL)
    {
        throw std::runtime_error("GetCpuUsageCycles failed: hProcess == NULL");
    }

    ULONG64 CycleTime1{0};

    QueryProcessCycleTime(hProcess, &CycleTime1);

    return CycleTime1;
}

/**
 * @brief Get cpu usage list
 *
 * @return std::vector<ProcessInfo> vector of processes with cpu usage
 */
std::vector<ProcessInfo> GetCpuUsageList()
{
    net::io_context io;
    std::vector<ProcessInfo> processes;

    DWORD processIds[1024];
    DWORD bytesReturned;

    if (!EnumProcesses(processIds, sizeof(processIds), &bytesReturned))
    {
        throw std::runtime_error("EnumProcesses failed");
    }

    int numProcesses = bytesReturned / sizeof(DWORD);
    processes.reserve(numProcesses);

    for (int i = 0; i < numProcesses; i++)
    {
        auto process = std::make_shared<ProcessInfo>();
        process->processId = processIds[i];

        HANDLE processHandle = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, processIds[i]);
        if (processHandle == NULL)
        {
            continue;
        }

        HMODULE hMod;
        DWORD cbNeeded;
        if (EnumProcessModules(processHandle, &hMod, sizeof(hMod), &cbNeeded))
        {
            GetModuleBaseName(processHandle, hMod, process->name, sizeof(process->name) / sizeof(TCHAR));
        }

        try
        {
            process->cpuCycles1 = GetCpuUsageCycles(processHandle);
        }
        catch (const std::runtime_error &e)
        {
            std::cout << "Process " << processIds[i] << " error:" << e.what() << std::endl;
            continue;
        }

        auto t = std::make_shared<net::steady_timer>(io, std::chrono::seconds(_S_timeout_sec));
        t->async_wait([t = std::move(t), processHandle = std::move(processHandle), process = std::move(process), &processes](const sys::error_code &ec)
                      {
            if (!ec)
            {
                process->cpuCycles2 = GetCpuUsageCycles(processHandle);
                process->cpuCyclesDif = process->cpuCycles2 - process->cpuCycles1;
                CloseHandle(processHandle);

                processes.push_back(std::move(*process));
            }
            else
            {
                std::cout << ec.message() << std::endl;
                CloseHandle(processHandle);
            } });
    }

    io.run();

    return processes;
}

int main()
{
    try
    {
        auto processes = GetCpuUsageList();
        std::sort(processes.begin(), processes.end(), [](const ProcessInfo &a, const ProcessInfo &b)
                { return a.cpuCyclesDif > b.cpuCyclesDif; });

        size_t cycles_summ = std::accumulate(processes.begin(), processes.end(), static_cast<size_t>(0), [](size_t a, const ProcessInfo &b) -> size_t
                                             { return a + b.cpuCyclesDif; });
        std::for_each(std::execution::par, processes.begin(), processes.end(), [cycles_summ = cycles_summ](ProcessInfo &b)
                      { b.cpuCyclesDifPerc = (static_cast<double>(b.cpuCyclesDif) / static_cast<double>(cycles_summ)) * 100; });

        PrintProcessVec(processes);
    }
    catch (const std::exception &e)
    {
        std::cerr << "Exception: " << e.what() << "\n";
    }
}