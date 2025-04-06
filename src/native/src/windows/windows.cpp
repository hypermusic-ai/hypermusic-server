#include "windows/windows.h"

#include <spdlog/spdlog.h>

namespace hm::native{
    void spawnProcess(const std::string& command)
    {
        STARTUPINFOA si;
        PROCESS_INFORMATION pi;
        ZeroMemory(&si, sizeof(si));
        ZeroMemory(&pi, sizeof(pi));
        si.cb = sizeof(si);

        char cmd_line[1024];
        strcpy_s(cmd_line, command.c_str());

        if (!CreateProcessA(
            NULL,         // No module name (use command line)
            cmd_line,     // Command line
            NULL,         // Process handle not inheritable
            NULL,         // Thread handle not inheritable
            FALSE,        // Set handle inheritance to FALSE
            0,            // No creation flags
            NULL,         // Use parent's environment block
            NULL,         // Use parent's starting directory 
            &si,          // Pointer to STARTUPINFO structure
            &pi)          // Pointer to PROCESS_INFORMATION structure
        )
        {
            spdlog::error("CreateProcess failed: {}", GetLastError());
            return;
        }

        // Wait until child process exits
        WaitForSingleObject(pi.hProcess, INFINITE);

        CloseHandle(pi.hProcess);
        CloseHandle(pi.hThread);
    }
}