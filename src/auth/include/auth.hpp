#pragma once

#include <random>
#include <string>
#include <expected>
#include <format>
#include <regex>

#include "native.h"
#include <asio.hpp>
#include <asio/experimental/awaitable_operators.hpp>
using namespace asio::experimental::awaitable_operators;

#include <absl/container/flat_hash_map.h>
#include <spdlog/spdlog.h>
#include <secp256k1.h>
#include <secp256k1_recovery.h>
#include <jwt-cpp/jwt.h>
#include <evmc/hex.hpp>

#include "utils.hpp"
#include "keccak256.hpp"

namespace dcn
{
    enum class AuthenticationError : std::uint8_t
    {
        Unknown = 0,
        MissingCookie,
        InvalidCookie,
        MissingToken,
        InvalidToken,
        InvalidSignature,
        InvalidNonce,
        InvalidAddress
    };

    class AuthManager
    {
        public:
            AuthManager() = delete;
            AuthManager(asio::io_context & io_context);

            AuthManager(const AuthManager&) = delete;
            AuthManager& operator=(const AuthManager&) = delete;
            
            ~AuthManager() = default;

            asio::awaitable<std::string> generateNonce(const std::string & address);

            asio::awaitable<bool> verifyNonce(const std::string& address, const std::string & nonce);

            asio::awaitable<bool> verifySignature(const std::string& address, const std::string& signature, const std::string& message);

            asio::awaitable<std::string> generateAccessToken(const std::string& address);

            asio::awaitable<std::expected<std::string, AuthenticationError>> verifyAccessToken(std::string token) const;

            asio::awaitable<bool> compareAccessToken(std::string address, std::string token) const;

            asio::awaitable<std::string> generateRefreshToken(const std::string& address);

            asio::awaitable<std::expected<std::string, AuthenticationError>> verifyRefreshToken(std::string token) const;

        protected:


        private:
            asio::strand<asio::io_context::executor_type> _strand;

            const std::string _SECRET; // !!! TODO !!! use secure secret in production

            std::mt19937 _rng;
            std::uniform_int_distribution<int> _dist;
            absl::flat_hash_map<std::string, std::string> _nonces;

            absl::flat_hash_map<std::string, std::string> _refresh_tokens;
            absl::flat_hash_map<std::string, std::string> _access_tokens;
    };
}

namespace dcn::parse
{
    std::string parseEthAddressFromPublicKey(const std::uint8_t* pubkey, std::size_t len);
    std::string parseNonceFromMessage(const std::string & msg);

    std::optional<std::string> parseAccessTokenFromCookieHeader(const std::string & cookie_str);
    std::string parseAccessTokenToCookieHeader(const std::string & token_str);

    std::optional<std::string> parseRefreshTokenFromCookieHeader(const std::string & cookie_str);
    std::string parseRefreshTokenToCookieHeader(const std::string & token_str);
}

template <>
struct std::formatter<dcn::AuthenticationError> : std::formatter<std::string> {
    auto format(const dcn::AuthenticationError & err, format_context& ctx) const {
        switch(err)
        {
            case dcn::AuthenticationError::MissingCookie : return formatter<string>::format("MissingCookie", ctx);
            case dcn::AuthenticationError::InvalidCookie : return formatter<string>::format("InvalidCookie", ctx);
            case dcn::AuthenticationError::MissingToken : return formatter<string>::format("MissingToken", ctx);
            case dcn::AuthenticationError::InvalidToken : return formatter<string>::format("InvalidToken", ctx);
            case dcn::AuthenticationError::InvalidSignature : return formatter<string>::format("InvalidSignature", ctx);
            case dcn::AuthenticationError::InvalidNonce : return formatter<string>::format("InvalidNonce", ctx);
            case dcn::AuthenticationError::InvalidAddress : return formatter<string>::format("InvalidAddress", ctx);

            default:  return formatter<string>::format("Unknown", ctx);
        }
        return formatter<string>::format("", ctx);
    }
};