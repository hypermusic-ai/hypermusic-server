#if defined(WIN32)
#include "windows/windows.h"
#elif defined(UNIX)
#include "unix/unix.h"
#else
#   error "Error, both can't be undefined same time"
#endif

#include <string>

namespace hm::native
{
    void spawnProcess(const std::string & command);
}