#include <task.hpp>

#include <vector.hpp>
#include <rand.hpp>
#include <debug.h>

namespace Tasking
{
    struct TokenData
    {
        Token token;
        enum TokenTrustLevel TrustLevel;
        uint64_t OwnerID;
        bool Process;
    };

    Vector<TokenData> Tokens;

    Token Security::CreateToken()
    {
        uint64_t ret = Random::rand64();
        Tokens.push_back({ret, UnknownTrustLevel, 0, false});
        debug("Created token %#lx", ret);
        return ret;
    }

    bool Security::TrustToken(Token token,
                              TokenTrustLevel TrustLevel)
    {
        enum TokenTrustLevel Level = static_cast<enum TokenTrustLevel>(TrustLevel);

        foreach (auto var in Tokens)
        {
            if (var.token == token)
            {
                var.TrustLevel = Level;
                debug("Trusted token %#lx", token);
                return true;
            }
        }
        debug("Failed to trust token %#lx", token);
        return false;
    }

    bool Security::UntrustToken(Token token)
    {
        fixme("UntrustToken->false");
        return false;
    }

    bool Security::DestroyToken(Token token)
    {
        fixme("DestroyToken->false");
        return false;
    }

    Security::Security()
    {
        trace("Initializing Tasking Security");
    }

    Security::~Security()
    {
        trace("Destroying Tasking Security");
        for (uint64_t i = 0; i < Tokens.size(); i++)
            Tokens.remove(i);
    }
}
