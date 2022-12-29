#include <gui.hpp>

#include <driver.hpp>
#include <task.hpp>
#include <printf.h>
#include <debug.h>

#include "../kernel.h"
#include "../DAPI.hpp"
#include "../Fex.hpp"

namespace GraphicalUserInterface
{
    Handle WidgetCollection::CreatePanel(uint32_t Left, uint32_t Top, uint32_t Width, uint32_t Height)
    {
        return 0;
    }

    WidgetCollection::WidgetCollection(void *ParentWindow)
    {
        this->mem = new Memory::MemMgr;
    }

    WidgetCollection::~WidgetCollection()
    {
        delete this->mem;
    }
}
