#pragma once

#include <random>
#include <string>
#include <expected>

#include "native.h"
#include <asio.hpp>
#include <asio/experimental/awaitable_operators.hpp>
using namespace asio::experimental::awaitable_operators;

#include <absl/container/flat_hash_map.h>
#include <spdlog/spdlog.h>
#include <secp256k1.h>
#include <secp256k1_recovery.h>
#include <jwt-cpp/jwt.h>

#include "utils.hpp"
#include "keccak256.hpp"

namespace hm
{
    class AuthManager
    {
        public:
            AuthManager() = delete;
            AuthManager(asio::io_context & io_context);

            AuthManager(const AuthManager&) = delete;
            AuthManager& operator=(const AuthManager&) = delete;
            
            ~AuthManager() = default;

            asio::awaitable<std::string> generateNonce(const std::string & address);

            asio::awaitable<bool> verifySignature(const std::string& address, const std::string& signature, const std::string& message) const;

            asio::awaitable<std::string> getNonce(const std::string& address);

            asio::awaitable<std::string> generateToken(const std::string& address);

            asio::awaitable<std::expected<std::string, std::string>> verifyToken(const std::string& token) const;

        private:
            asio::strand<asio::io_context::executor_type> _strand;

            const std::string _SECRET; // !!! TODO !!! use secure secret in production

            std::mt19937 _rng;
            std::uniform_int_distribution<int> _dist;
            absl::flat_hash_map<std::string, std::string> _nonces;
            absl::flat_hash_map<std::string, std::string> _tokens;
    };

    std::string pubkeyToAddress(const std::uint8_t* pubkey, std::size_t len);
}