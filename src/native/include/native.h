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
    void spawnProcess(const std::string & command);
}