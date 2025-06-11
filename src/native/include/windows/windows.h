#pragma once

#ifndef WIN32
    #error "Error, WIN32 not defined"
#endif

#ifndef _WIN32_WINNT
#define _WIN32_WINNT	0xa00
#endif
#ifdef UNICODE
  #undef UNICODE
#endif

#define NOMINMAX

#include <winsock2.h>
#include <windows.h>