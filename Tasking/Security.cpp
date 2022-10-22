#include <task.hpp>
#include <debug.h>

namespace Tasking
{
    Token Security::CreateToken()
    {
        fixme("CreateToken->0");
        return 0;
    }

    bool Security::TrustToken(Token token,
                              TokenTrustLevel TrustLevel)
    {
        fixme("TrustToken->false");
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
    }
}
