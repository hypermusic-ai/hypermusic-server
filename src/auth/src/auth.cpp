#include "auth.hpp"

namespace dcn::parse
{
    // Get Ethereum address from public key (last 20 bytes of Keccak256(pubkey))
    static evmc::address _parseEthAddressFromPublicKey(const std::uint8_t* pubkey, std::size_t len) 
    {
        uint8_t hash[Keccak256::HASH_LEN];
        // skip 0x04 prefix
        dcn::Keccak256::getHash(pubkey + 1, len - 1, hash);
        evmc::address address;
        // last 20 bytes
        std::copy(hash + 12, hash + 32, address.bytes);
        return address; 
    }

    std::string parseNonceFromMessage(const std::string& nonce_str) 
    {
        const std::string prefix = "Login nonce: ";
        // Check that str is at least as long as the prefix
        if (nonce_str.size() <= prefix.size()) {
            return {};
        }
        // Does it start with prefix?
        if (nonce_str.compare(0, prefix.size(), prefix) != 0) {
            return {};
        }
        // Extract everything after prefix
        return nonce_str.substr(prefix.size());
    }

    static const std::string ACCESS_TOKEN_PREFIX = "access_token=";  

    std::optional<std::string> parseAccessTokenFromCookieHeader(const std::string& cookie_str) 
    {
        static const std::regex token_regex(ACCESS_TOKEN_PREFIX+"([^;]+)");
        // Check if the cookie string is empty
        if (cookie_str.empty()) {
            return {};
        }
        // Use regex to find the token in the cookie string
        std::smatch match;
        if (std::regex_search(cookie_str, match, token_regex) == false) {
            return {};
        }
        return match[1].str();
    }

    std::string parseAccessTokenToCookieHeader(const std::string & token_str)
    {
        return ACCESS_TOKEN_PREFIX + token_str + "; HttpOnly; Secure; SameSite=None; Path=/;";
    }

    static const std::string REFRESH_TOKEN_PREFIX = "refresh_token=";  

    std::optional<std::string> parseRefreshTokenFromCookieHeader(const std::string& cookie_str) 
    {
        static const std::regex token_regex(REFRESH_TOKEN_PREFIX+"([^;]+)");
        // Check if the cookie string is empty
        if (cookie_str.empty()) {
            return {};
        }
        // Use regex to find the token in the cookie string
        std::smatch match;
        if (std::regex_search(cookie_str, match, token_regex) == false) {
            return {};
        }
        return match[1].str();
    }

    std::string parseRefreshTokenToCookieHeader(const std::string & token_str)
    {
        return REFRESH_TOKEN_PREFIX + token_str + "; HttpOnly; Secure; SameSite=None; Path=/refresh;";
    }
}

namespace dcn
{
    AuthManager::AuthManager(asio::io_context & io_context)
    :   _strand(asio::make_strand(io_context)),
        _rng(std::random_device{}()),
        _dist(100000, 999999),
        _SECRET{"SUPERSECRETKEY123"}
    {

    }

    asio::awaitable<std::string> AuthManager::generateNonce(const evmc::address & address)
    {
        std::string nonce =  std::to_string(_dist(_rng));
        
        co_await utils::ensureOnStrand(_strand);

        _nonces[address] = nonce;
        co_return nonce;
    }

    asio::awaitable<bool> AuthManager::verifyNonce(const evmc::address & address, const std::string & nonce)
    {
        co_await utils::ensureOnStrand(_strand);

        auto it = _nonces.find(address);
        if (it == _nonces.end()) co_return false;

        if (it->second != nonce)
        {
            // remove after failed verification (?)
            _nonces.erase(it);
            co_return false;
        }

        // remove nonce after successful verification
        _nonces.erase(it);
        co_return true;
    }

    asio::awaitable<bool> AuthManager::verifySignature(const evmc::address & address, const std::string& sig_hex, const std::string& message)
    {
        co_await utils::ensureOnStrand(_strand);
        
        // verify signature
        const std::optional<evmc::bytes> signature_bytes_result = evmc::from_hex(sig_hex);
        if(!signature_bytes_result) co_return false;
        const auto & signature_bytes = *signature_bytes_result;

        if (signature_bytes.size() != 65) co_return false;

        uint8_t hash[Keccak256::HASH_LEN];
        std::string prefix(1, '\x19');
        prefix += "Ethereum Signed Message:\n" + std::to_string(message.size()) + message;
        dcn::Keccak256::getHash((const uint8_t*)prefix.data(), prefix.size(), hash);

        // Adjust v
        int recid = signature_bytes[64];
        if (recid >= 27) recid -= 27;

        secp256k1_context* ctx = secp256k1_context_create(SECP256K1_CONTEXT_VERIFY);
        secp256k1_ecdsa_recoverable_signature signature;

        if (!secp256k1_ecdsa_recoverable_signature_parse_compact(ctx, &signature, signature_bytes.data(), recid)) {
            secp256k1_context_destroy(ctx);
            co_return false;
        }

        secp256k1_pubkey pubkey;
        if (!secp256k1_ecdsa_recover(ctx, &pubkey, &signature, hash)) {
            secp256k1_context_destroy(ctx);
            co_return false;
        }

        uint8_t pubkey_serialized[65];
        size_t pubkey_len = 65;
        secp256k1_ec_pubkey_serialize(ctx, pubkey_serialized, &pubkey_len, &pubkey, SECP256K1_EC_UNCOMPRESSED);
        secp256k1_context_destroy(ctx);

        co_return address == parse::_parseEthAddressFromPublicKey(pubkey_serialized, pubkey_len);
    }

    asio::awaitable<std::string> AuthManager::generateAccessToken(const evmc::address & address)
    {
        co_await utils::ensureOnStrand(_strand);

        auto token = jwt::create()
            .set_issuer("eth-auth-demo")
            .set_type("JWS")
            .set_subject(evmc::hex(address))
            .set_issued_at(std::chrono::system_clock::now())
            .set_expires_at(std::chrono::system_clock::now() + std::chrono::minutes{10})
            .sign(jwt::algorithm::hs256{_SECRET});

        _access_tokens[address] = token;

        co_return token;
    }

    asio::awaitable<std::expected<evmc::address, AuthenticationError>> AuthManager::verifyAccessToken(std::string token) const
    {
        co_await utils::ensureOnStrand(_strand);

        if(token.empty()) co_return std::unexpected(AuthenticationError::InvalidToken);
        if(token.back() == '\r')token.pop_back(); // remove \r if present

        try 
        {
            auto decoded = jwt::decode(token);

            auto verifier = jwt::verify()
                .allow_algorithm(jwt::algorithm::hs256{_SECRET})
                .with_issuer("eth-auth-demo");

            verifier.verify(decoded);

            auto address_result = evmc::from_hex<evmc::address>(decoded.get_subject());
            if(!address_result)
            {
                co_return std::unexpected(AuthenticationError::InvalidToken);
            }
            const auto &  address = *address_result;

            if(_access_tokens.contains(address) == false)
            {
                co_return std::unexpected(AuthenticationError::MissingToken);
            }

            if(token != _access_tokens.at(address))
            {
                co_return std::unexpected(AuthenticationError::InvalidToken);
            }

            co_return address;

        } catch (const std::exception& e) 
        {
            spdlog::error("Access token verification failed: {}", e.what());
            co_return std::unexpected(AuthenticationError::Unknown);
        }
    }

    asio::awaitable<bool> AuthManager::compareAccessToken(const evmc::address & address, std::string token) const
    {
        co_await utils::ensureOnStrand(_strand);

        if(token.empty()) co_return false;
        if(token.back() == '\r')token.pop_back(); // remove \r if present
        if(_access_tokens.contains(address) == false)
        {
            co_return false;
        }
        if(token != _access_tokens.at(address))
        {
            co_return false;
        }
        co_return true;
    }

    asio::awaitable<std::string> AuthManager::generateRefreshToken(const evmc::address & address)
    {
        co_await utils::ensureOnStrand(_strand);

        auto token = jwt::create()
            .set_issuer("eth-auth-demo")
            .set_type("JWS")
            .set_subject(evmc::hex(address))
            .set_issued_at(std::chrono::system_clock::now())
            .set_expires_at(std::chrono::system_clock::now() + std::chrono::days{7})
            .sign(jwt::algorithm::hs256{_SECRET});

        _refresh_tokens[address] = token;

        co_return token;
    }

    asio::awaitable<std::expected<evmc::address, AuthenticationError>> AuthManager::verifyRefreshToken(std::string token) const
    {
        co_await utils::ensureOnStrand(_strand);

        if(token.empty()) co_return std::unexpected(AuthenticationError::InvalidToken);
        if(token.back() == '\r')token.pop_back(); // remove \r if present

        try 
        {
            auto decoded = jwt::decode(token);

            auto verifier = jwt::verify()
                .allow_algorithm(jwt::algorithm::hs256{_SECRET})
                .with_issuer("eth-auth-demo");

            verifier.verify(decoded);

            auto address_result = evmc::from_hex<evmc::address>(decoded.get_subject());
            if(!address_result)
            {
                co_return std::unexpected(AuthenticationError::InvalidToken);
            }
            const auto &  address = *address_result;

            if(_refresh_tokens.contains(address) == false)
            {
                co_return std::unexpected(AuthenticationError::MissingToken);
            }

            if(token != _refresh_tokens.at(address))
            {
                co_return std::unexpected(AuthenticationError::InvalidToken);
            }

            co_return address;

        } catch (const std::exception& e) 
        {
            spdlog::error("Refresh token verification failed: {}", e.what());
            co_return std::unexpected(AuthenticationError::Unknown);
        }
    }
}