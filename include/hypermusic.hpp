#pragma once

#include "native.h"
#include <asio.hpp>

#include "server.hpp"
#include "http.hpp"
#include "session.hpp"
#include "evm.hpp"
#include "pt.hpp"
#include "runner.hpp"
#include "file.hpp"
#include "api.hpp"
#include "auth.hpp"
#include "utils.hpp"

namespace hm
{
    const short int MAJOR_VERSION = 0;
    const short int MINOR_VERSION = 0;
    const short int PATCH_VERSION = 1;

    const asio::ip::port_type DEFAULT_PORT = 54321;
    const asio::ip::port_type DEFAULT_TLS_PORT = 54322;
}