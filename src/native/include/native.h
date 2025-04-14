#if defined(WIN32)
#include "windows/windows.h"
#elif defined(UNIX)
#include "unix/unix.h"
#elif defined(__APPLE__)
#include "mac/mac.h"
#else
#   error "Error, unsupported platform"
#endif

#include <string>

namespace hm::native
{
    /**
    * Spawns a new process with the given command.
    * The command must be a valid shell command.
    *
    * @param command The command to execute in the new process
    */
    void spawnProcess(const std::string & command);
}