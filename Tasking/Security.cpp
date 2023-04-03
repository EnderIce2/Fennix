/*
   This file is part of Fennix Kernel.

   Fennix Kernel is free software: you can redistribute it and/or
   modify it under the terms of the GNU General Public License as
   published by the Free Software Foundation, either version 3 of
   the License, or (at your option) any later version.

   Fennix Kernel is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with Fennix Kernel. If not, see <https://www.gnu.org/licenses/>.
*/

#include <task.hpp>

#include <vector.hpp>
#include <rand.hpp>
#include <debug.h>

namespace Tasking
{
    Token Security::CreateToken()
    {
        uint64_t ret = 0;
    Retry:
        ret = Random::rand64();
        foreach (auto t in Tokens)
            if (t.token == ret)
                goto Retry;

        Tokens.push_back({ret, UnknownTrustLevel, 0, false});
        debug("Created token %#lx", ret);
        return ret;
    }

    bool Security::TrustToken(Token token, TTL TrustLevel)
    {
        foreach (auto &t in Tokens)
        {
            if (t.token == token)
            {
                t.TrustLevel = TrustLevel;
                debug("Trusted token %#lx to level %d", token, t.TrustLevel);
                return true;
            }
        }
        warn("Failed to trust token %#lx", token);
        return false;
    }

    bool Security::UntrustToken(Token token)
    {
        foreach (auto &t in Tokens)
        {
            if (t.token == token)
            {
                t.TrustLevel = Untrusted;
                debug("Untrusted token %#lx", token);
                return true;
            }
        }
        warn("Failed to untrust token %#lx", token);
        return false;
    }

    bool Security::AddTrustLevel(Token token, TTL TrustLevel)
    {
        foreach (auto &t in Tokens)
        {
            if (t.token == token)
            {
                t.TrustLevel |= TrustLevel;
                debug("Added trust level %d to token %#lx", t.TrustLevel, token);
                return true;
            }
        }
        warn("Failed to add trust level %d to token %#lx", TrustLevel, token);
        return false;
    }

    bool Security::RemoveTrustLevel(Token token, TTL TrustLevel)
    {
        foreach (auto &t in Tokens)
        {
            if (t.token == token)
            {
                t.TrustLevel &= ~TrustLevel;
                debug("Removed trust level %d from token %#lx", t.TrustLevel, token);
                return true;
            }
        }
        warn("Failed to remove trust level %d from token %#lx", TrustLevel, token);
        return false;
    }

    bool Security::DestroyToken(Token token)
    {
        fixme("DestroyToken->true");
        UNUSED(token);
        return true;
    }

    bool Security::IsTokenTrusted(Token token, TTL TrustLevel)
    {
        foreach (auto t in Tokens)
            if (t.token == token)
            {
                if (t.TrustLevel == TrustLevel)
                    return true;
                else
                    return false;
            }

        warn("Failed to check trust level of token %#lx", token);
        return false;
    }

    bool Security::IsTokenTrusted(Token token, int TrustLevel)
    {
        foreach (auto t in Tokens)
            if (t.token == token)
            {
                if (t.TrustLevel & TrustLevel)
                    return true;
                else
                    return false;
            }

        warn("Failed to check trust level of token %#lx", token);
        return false;
    }

    int Security::GetTokenTrustLevel(Token token)
    {
        foreach (auto t in Tokens)
            if (t.token == token)
                return t.TrustLevel;

        warn("Failed to get trust level of token %#lx", token);
        return UnknownTrustLevel;
    }

    Security::Security() {}

    Security::~Security()
    {
        for (size_t i = 0; i < Tokens.size(); i++)
            Tokens.remove(i);
    }
}
