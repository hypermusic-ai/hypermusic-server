#include "registry.hpp"

namespace hm
{   
    Registry::Registry( asio::io_context & io_context)
    : _strand(asio::make_strand(io_context))
    {

    }
}