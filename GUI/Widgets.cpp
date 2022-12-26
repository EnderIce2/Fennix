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
    void Widget::Paint()
    {
    }

    void Widget::HandleEvent(Event *e)
    {
    }

    Widget::Widget(void *ParentWindow)
    {
        this->mem = new Memory::MemMgr;
    }

    Widget::~Widget()
    {
        delete this->mem;
    }
}
