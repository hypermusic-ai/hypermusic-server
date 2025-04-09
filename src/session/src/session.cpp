#include "session.hpp"

namespace hm
{
    Session::Session(asio::ip::tcp::socket & socket, SessionManager & session_mgr)
    : _socket(socket), _session_mgr(session_mgr) 
    {

    } 
    asio::awaitable<void> Session::stop()
    {
        if(co_await _session_mgr.destroySession(_session_id))
        {
            spdlog::info("Session {} ended", _session_id);
        }
        co_return;
    }
}