#include "auth.hpp"

namespace hm
{
    AuthManager::AuthManager(asio::io_context & io_context)
    :   _strand(asio::make_strand(io_context)),
        _rng(std::random_device{}()),
        _dist(100000, 999999),
        _SECRET{"SUPERSECRETKEY123"}
    {

    }

    asio::awaitable<std::string> AuthManager::generateNonce(const std::string & address)
    {
        std::string nonce =  std::to_string(_dist(_rng));
        
        co_await asio::dispatch(_strand, asio::use_awaitable);

        _nonces[address] = nonce;
        co_return nonce;
    }

    asio::awaitable<std::string> AuthManager::getNonce(const std::string& address)
    {
        co_await asio::dispatch(_strand, asio::use_awaitable);
        if(_nonces.contains(address) == false)
        {
            co_return "";
        }

        co_return _nonces.at(address);
    }

    asio::awaitable<bool> AuthManager::verifySignature(const std::string& address, const std::string& sig_hex, const std::string& message) const
    {
        co_await asio::dispatch(_strand, asio::use_awaitable);
        std::vector<uint8_t> signature_bytes = hexToBytes(sig_hex.substr(2, sig_hex.size() - 2)); // remove 0x prefix
        if (signature_bytes.size() != 65) co_return false;

        uint8_t hash[Keccak256::HASH_LEN];
        std::string prefix(1, '\x19');
        prefix += "Ethereum Signed Message:\n" + std::to_string(message.size()) + message;
        hm::Keccak256::getHash((const uint8_t*)prefix.data(), prefix.size(), hash);

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

        std::string recovered = pubkeyToAddress(pubkey_serialized, pubkey_len);

        // Case-insensitive compare
        std::string lower1 = address, lower2 = recovered;
        std::transform(lower1.begin(), lower1.end(), lower1.begin(), ::tolower);
        std::transform(lower2.begin(), lower2.end(), lower2.begin(), ::tolower);

        co_return lower1 == lower2;
    }

    asio::awaitable<std::string> AuthManager::generateToken(const std::string& address)
    {
        co_await asio::dispatch(_strand, asio::use_awaitable);

        auto token = jwt::create()
            .set_issuer("eth-auth-demo")
            .set_type("JWS")
            .set_subject(address)
            .set_issued_at(std::chrono::system_clock::now())
            .set_expires_at(std::chrono::system_clock::now() + std::chrono::minutes{10})
            .sign(jwt::algorithm::hs256{_SECRET});

        _tokens[address] = token;

        co_return token;
    }

    asio::awaitable<std::expected<std::string, std::string>> AuthManager::verifyToken(const std::string& token) const
    {
        try 
        {
            auto decoded = jwt::decode(token);

            auto verifier = jwt::verify()
                .allow_algorithm(jwt::algorithm::hs256{_SECRET})
                .with_issuer("eth-auth-demo");

            verifier.verify(decoded);

            co_return decoded.get_subject(); // the Ethereum address

        } catch (const std::exception& e) 
        {
            co_return std::unexpected(std::string("Token verification failed: ") + std::string(e.what()));
        }
    }
}

namespace hm
{
    // Get Ethereum address from public key (last 20 bytes of Keccak256(pubkey))
    std::string pubkeyToAddress(const std::uint8_t* pubkey, std::size_t len) 
    {
        uint8_t hash[Keccak256::HASH_LEN];
        // skip 0x04 prefix
        hm::Keccak256::getHash(pubkey + 1, len - 1, hash);
        return "0x" + bytesToHex(hash + 12, 20); // last 20 bytes
    }
}