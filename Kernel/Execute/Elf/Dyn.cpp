#include <exec.hpp>

#include <memory.hpp>
#include <lock.hpp>
#include <msexec.h>
#include <cwalk.h>
#include <elf.h>
#include <abi.h>

#include "../../kernel.h"
#include "../../Fex.hpp"

using namespace Tasking;

namespace Execute
{
    ELFBaseLoad ELFLoadDyn(void *BaseImage,
                           VirtualFileSystem::File *ExFile,
                           Tasking::PCB *Process)
    {
        fixme("Not implemented");
        return {};
    }
}
