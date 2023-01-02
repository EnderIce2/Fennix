#include <gui.hpp>
#include <driver.hpp>
#include <task.hpp>
#include <debug.h>

#include "../kernel.h"
#include "../DAPI.hpp"
#include "../Fex.hpp"

namespace GraphicalUserInterface
{
    char CursorArrow[] = {
        1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, // 12x19
        1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, //
        1, 2, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, //
        1, 2, 2, 1, 0, 0, 0, 0, 0, 0, 0, 0, //
        1, 2, 2, 2, 1, 0, 0, 0, 0, 0, 0, 0, //
        1, 2, 2, 2, 2, 1, 0, 0, 0, 0, 0, 0, //
        1, 2, 2, 2, 2, 2, 1, 0, 0, 0, 0, 0, //
        1, 2, 2, 2, 2, 2, 2, 1, 0, 0, 0, 0, //
        1, 2, 2, 2, 2, 2, 2, 2, 1, 0, 0, 0, //
        1, 2, 2, 2, 2, 2, 2, 2, 2, 1, 0, 0, //
        1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 1, 0, //
        1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 1, //
        1, 2, 2, 2, 2, 2, 2, 1, 1, 1, 1, 1, //
        1, 2, 2, 2, 1, 2, 2, 1, 0, 0, 0, 0, //
        1, 2, 2, 1, 0, 1, 2, 2, 1, 0, 0, 0, //
        1, 2, 1, 0, 0, 1, 2, 2, 1, 0, 0, 0, //
        1, 1, 0, 0, 0, 0, 1, 2, 2, 1, 0, 0, //
        1, 0, 0, 0, 0, 0, 1, 2, 2, 1, 0, 0, //
        0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, //
    };

    char CursorHand[] = {
        0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, // 17x24
        0, 0, 0, 0, 1, 2, 2, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, //
        0, 0, 0, 0, 1, 2, 2, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, //
        0, 0, 0, 0, 1, 2, 2, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, //
        0, 0, 0, 0, 1, 2, 2, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, //
        0, 0, 0, 0, 1, 2, 2, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, //
        0, 0, 0, 0, 1, 2, 2, 1, 2, 2, 1, 1, 1, 0, 0, 0, 0, //
        0, 0, 0, 0, 1, 2, 2, 1, 2, 2, 1, 2, 2, 1, 1, 0, 0, //
        0, 0, 0, 0, 1, 2, 2, 1, 2, 2, 1, 2, 2, 1, 2, 1, 0, //
        0, 0, 0, 0, 1, 2, 2, 1, 2, 2, 1, 2, 2, 1, 2, 2, 1, //
        1, 1, 1, 0, 1, 2, 2, 1, 2, 2, 1, 2, 2, 1, 2, 2, 1, //
        1, 2, 2, 1, 1, 2, 2, 2, 2, 2, 2, 2, 2, 1, 2, 2, 1, //
        1, 2, 2, 2, 1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 1, //
        0, 1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 1, //
        0, 0, 1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 1, //
        0, 0, 1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 1, //
        0, 0, 0, 1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 1, //
        0, 0, 0, 1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 1, 0, //
        0, 0, 0, 0, 1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 1, 0, //
        0, 0, 0, 0, 1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 1, 0, //
        0, 0, 0, 0, 0, 1, 2, 2, 2, 2, 2, 2, 2, 2, 1, 0, 0, //
        0, 0, 0, 0, 0, 1, 2, 2, 2, 2, 2, 2, 2, 2, 1, 0, 0, //
        0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, //
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, //
    };

    char CursorWait[] = {
        1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, // 13x22
        1, 1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 1, 1, //
        1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, //
        0, 1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 1, 0, //
        0, 1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 1, 0, //
        0, 1, 2, 2, 1, 2, 1, 2, 1, 2, 2, 1, 0, //
        0, 1, 2, 2, 2, 1, 2, 1, 2, 2, 2, 1, 0, //
        0, 1, 1, 2, 2, 2, 1, 2, 2, 2, 1, 1, 0, //
        0, 0, 1, 1, 2, 2, 2, 2, 2, 1, 1, 0, 0, //
        0, 0, 0, 1, 1, 2, 1, 2, 1, 1, 0, 0, 0, //
        0, 0, 0, 0, 1, 1, 2, 1, 1, 0, 0, 0, 0, //
        0, 0, 0, 0, 1, 1, 2, 1, 1, 0, 0, 0, 0, //
        0, 0, 0, 1, 1, 2, 2, 2, 1, 1, 0, 0, 0, //
        0, 0, 1, 1, 2, 2, 1, 2, 2, 1, 1, 0, 0, //
        0, 1, 1, 2, 2, 2, 2, 2, 2, 2, 1, 1, 0, //
        0, 1, 2, 2, 2, 2, 1, 2, 2, 2, 2, 1, 0, //
        0, 1, 2, 2, 2, 1, 2, 1, 2, 2, 2, 1, 0, //
        0, 1, 2, 2, 1, 2, 1, 2, 1, 2, 2, 1, 0, //
        0, 1, 2, 1, 2, 1, 2, 1, 2, 1, 2, 1, 0, //
        1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, //
        1, 1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 1, 1, //
        1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, //
    };

    char CursorIBeam[] = {
        1, 1, 1, 0, 1, 1, 1, // 7x17
        0, 0, 0, 1, 0, 0, 0, //
        0, 0, 0, 1, 0, 0, 0, //
        0, 0, 0, 1, 0, 0, 0, //
        0, 0, 0, 1, 0, 0, 0, //
        0, 0, 0, 1, 0, 0, 0, //
        0, 0, 0, 1, 0, 0, 0, //
        0, 0, 0, 1, 0, 0, 0, //
        0, 0, 0, 1, 0, 0, 0, //
        0, 0, 0, 1, 0, 0, 0, //
        0, 0, 0, 1, 0, 0, 0, //
        0, 0, 0, 1, 0, 0, 0, //
        0, 0, 0, 1, 0, 0, 0, //
        0, 0, 0, 1, 0, 0, 0, //
        0, 0, 0, 1, 0, 0, 0, //
        0, 0, 0, 1, 0, 0, 0, //
        1, 1, 1, 0, 1, 1, 1, //
    };

    char CursorResizeAll[] = {
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, // 23x23
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 2, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, //
        0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 2, 2, 2, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, //
        0, 0, 0, 0, 0, 0, 0, 0, 1, 2, 2, 2, 2, 2, 1, 0, 0, 0, 0, 0, 0, 0, 0, //
        0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 2, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, //
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 2, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, //
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 2, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, //
        0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 1, 2, 1, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, //
        0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 1, 2, 1, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, //
        0, 0, 1, 2, 1, 0, 0, 0, 0, 0, 1, 2, 1, 0, 0, 0, 0, 0, 1, 2, 1, 0, 0, //
        0, 1, 2, 2, 1, 1, 1, 1, 1, 1, 1, 2, 1, 1, 1, 1, 1, 1, 1, 2, 2, 1, 0, //
        1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 1, //
        0, 1, 2, 2, 1, 1, 1, 1, 1, 1, 1, 2, 1, 1, 1, 1, 1, 1, 1, 2, 2, 1, 0, //
        0, 0, 1, 2, 1, 0, 0, 0, 0, 0, 1, 2, 1, 0, 0, 0, 0, 0, 1, 2, 1, 0, 0, //
        0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 1, 2, 1, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, //
        0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 1, 2, 1, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, //
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 2, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, //
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 2, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, //
        0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, //
        0, 0, 0, 0, 0, 0, 0, 0, 1, 2, 2, 2, 2, 2, 1, 0, 0, 0, 0, 0, 0, 0, 0, //
        0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 2, 2, 2, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, //
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 2, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, //
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, //
    };

    uint32_t DesktopFadeEffect[] = {
        0xFF000000,
        0x010101,
        0x040404,
        0x080808,
        0x101010,
        0x121212,
        0x151515,
        0x181818,
        0x1A1A1A,
        0x1D1D1D,
        0x1F1F1F,
        0x222222,
        0x242424,
        0x262626,
        0x282828,
    };

    char CloseButton[] = {
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, //
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, //
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, //
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, //
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, //
        0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, //
        0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, //
        0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, //
        0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, //
        0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, //
        0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, //
        0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, //
        0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, //
        0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, //
        0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, //
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, //
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, //
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, //
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, //
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, //
    };

    char MinimizeButton[] = {
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, //
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, //
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, //
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, //
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, //
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, //
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, //
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, //
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, //
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, //
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, //
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, //
        0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, //
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, //
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, //
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, //
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, //
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, //
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, //
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, //
    };

    char MaximizeButtonNormal[] = {
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, //
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, //
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, //
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, //
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, //
        0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, //
        0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, //
        0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 0, 1, 0, 0, 0, 0, 0, //
        0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 1, 0, 1, 0, 0, 0, 0, 0, //
        0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 1, 0, 1, 0, 0, 0, 0, 0, //
        0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 1, 0, 1, 0, 0, 0, 0, 0, //
        0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 1, 0, 1, 0, 0, 0, 0, 0, //
        0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 1, 1, 1, 0, 0, 0, 0, 0, //
        0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, //
        0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, //
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, //
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, //
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, //
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, //
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, //
    };

    char MaximizeButtonMaximized[] = {
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, //
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, //
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, //
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, //
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, //
        0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, //
        0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, //
        0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, //
        0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, //
        0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, //
        0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, //
        0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, //
        0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, //
        0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, //
        0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, //
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, //
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, //
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, //
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, //
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, //
    };

    char ResizeHint[] = {
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, //
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, //
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, //
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, //
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, //
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, //
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, //
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, //
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, //
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, //
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, //
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 1, 0, 0, //
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 1, 0, 0, 0, //
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 1, 0, 0, 0, 0, //
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, //
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 0, //
        0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 0, 0, //
        0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 0, 0, 0, //
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, //
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, //
    };

    uint32_t CloseButtonFade[] = {
        0x404040,
        0x770000,
        0x990000,
        0xBB0000,
        0xDD0000,
        0xFF0000,
    };

    uint32_t MaximizeMinimizeButtonFade[] = {
        0x404040,
        0x454545,
        0x505050,
        0x5F5F5F,
    };

    O1 void GUI::FetchInputs()
    {
        KernelCallback callback;
        MouseData Mouse;
        bool FoundMouseDriver = false;
        if (likely(DriverManager->GetDrivers().size() > 0))
        {
            foreach (auto Driver in DriverManager->GetDrivers())
            {
                if (((FexExtended *)((uintptr_t)Driver->Address + EXTENDED_SECTION_ADDRESS))->Driver.Type == FexDriverType::FexDriverType_Input)
                {
                    static uint32_t DisplayWidth = 0;
                    static uint32_t DisplayHeight = 0;

                    if (DisplayWidth == 0)
                        DisplayWidth = Display->GetBuffer(200)->Width;

                    if (DisplayHeight == 0)
                        DisplayHeight = Display->GetBuffer(200)->Height;

                    memset(&callback, 0, sizeof(KernelCallback));
                    callback.Reason = FetchReason;
                    DriverManager->IOCB(Driver->DriverUID, (void *)&callback);
                    // TODO: I think I should check somehow what driver is the one that is mouse and not keyboard
                    Mouse.X = (callback.InputCallback.Mouse.X * DisplayWidth) / 0xFFFF;  // VMWARE mouse
                    Mouse.Y = (callback.InputCallback.Mouse.Y * DisplayHeight) / 0xFFFF; // VMWARE mouse
                    Mouse.Z = (callback.InputCallback.Mouse.Z);
                    Mouse.Left = callback.InputCallback.Mouse.Buttons.Left;
                    Mouse.Right = callback.InputCallback.Mouse.Buttons.Right;
                    Mouse.Middle = callback.InputCallback.Mouse.Buttons.Middle;
                    FoundMouseDriver = true;
                    break;
                }
            }
        }

        if (unlikely(!FoundMouseDriver))
        {
            Mouse.X = 0;
            Mouse.Y = 0;
            Mouse.Z = 0;
            Mouse.Left = false;
            Mouse.Right = false;
            Mouse.Middle = false;
        }

        Event eTemplate;
        memset(&eTemplate, 0, sizeof(Event));
        foreach (auto wnd in this->Windows)
        {
            /* On mouse down event */
            if (unlikely((!MouseArray[1].Left || !MouseArray[1].Right || !MouseArray[1].Middle) &&
                         (Mouse.Left || Mouse.Right || Mouse.Middle)))
            {
                eTemplate.MouseDown.X = Mouse.X;
                eTemplate.MouseDown.Y = Mouse.Y;
                eTemplate.MouseDown.Left = Mouse.Left;
                eTemplate.MouseDown.Right = Mouse.Right;
                eTemplate.MouseDown.Middle = Mouse.Middle;
                wnd->OnMouseDown(&eTemplate);
            }

            /* On mouse up event */
            if (unlikely((MouseArray[1].Left || MouseArray[1].Right || MouseArray[1].Middle) &&
                         (!Mouse.Left || !Mouse.Right || !Mouse.Middle)))
            {
                eTemplate.MouseUp.X = Mouse.X;
                eTemplate.MouseUp.Y = Mouse.Y;
                eTemplate.MouseUp.Left = Mouse.Left;
                eTemplate.MouseUp.Right = Mouse.Right;
                eTemplate.MouseUp.Middle = Mouse.Middle;
                wnd->OnMouseUp(&eTemplate);
            }

            if (likely(Mouse.X != MouseArray[1].X || Mouse.Y != MouseArray[1].Y))
            {
                Rect TopBarPos = wnd->GetPosition();
                TopBarPos.Top -= 20;
                TopBarPos.Height = 20;
                TopBarPos.Width -= 60; /* buttons */
                if (unlikely(TopBarPos.Top < 0))
                {
                    TopBarPos.Top = 0;
                    wnd->GetPositionPtr()->Top = 20;
                }

                Rect ResizeHintPos = wnd->GetPosition();
                ResizeHintPos.Left += ResizeHintPos.Width - 20;
                ResizeHintPos.Top += ResizeHintPos.Height - 20;
                ResizeHintPos.Width = 20;
                ResizeHintPos.Height = 20;

                if (unlikely(TopBarPos.Contains(Mouse.X, Mouse.Y) ||
                             TopBarPos.Contains(MouseArray[0].X, MouseArray[0].Y) ||
                             TopBarPos.Contains(MouseArray[1].X, MouseArray[1].Y)))
                {
                    if (likely(Mouse.Left))
                    {
                        if (likely(MouseArray[1].Left))
                        {
                            wnd->GetPositionPtr()->Left += Mouse.X - MouseArray[0].X;
                            wnd->GetPositionPtr()->Top += Mouse.Y - MouseArray[0].Y;
                            OverlayBufferRepaint = true;
                            OverlayFullRepaint = true;
                        }
                    }
                }

                if (ResizeHintPos.Contains(Mouse.X, Mouse.Y) ||
                    ResizeHintPos.Contains(MouseArray[0].X, MouseArray[0].Y) ||
                    ResizeHintPos.Contains(MouseArray[1].X, MouseArray[1].Y))
                {
                    if (Mouse.Left)
                    {
                        if (MouseArray[1].Left)
                        {
                            wnd->GetPositionPtr()->Width += Mouse.X - MouseArray[0].X;
                            wnd->GetPositionPtr()->Height += Mouse.Y - MouseArray[0].Y;

                            if (wnd->GetPositionPtr()->Width < 200)
                            {
                                wnd->GetPositionPtr()->Width = 200;
                                Mouse.X = MouseArray[0].X;
                            }

                            if (wnd->GetPositionPtr()->Height < 100)
                            {
                                wnd->GetPositionPtr()->Height = 100;
                                Mouse.Y = MouseArray[0].Y;
                            }

                            OverlayBufferRepaint = true;
                            OverlayFullRepaint = true;
                            eTemplate.Resize.Width = wnd->GetPosition().Width;
                            eTemplate.Resize.Height = wnd->GetPosition().Height;

                            wnd->OnResize(&eTemplate);
                        }
                    }

                    if (unlikely(Cursor != CursorType::ResizeAll))
                    {
                        Cursor = CursorType::ResizeAll;
                        CursorBufferRepaint = true;
                    }
                }
                else
                {
                    if (unlikely(Cursor != CursorType::Arrow))
                    {
                        Cursor = CursorType::Arrow;
                        CursorBufferRepaint = true;
                    }
                }

                eTemplate.MouseMove.X = Mouse.X;
                eTemplate.MouseMove.Y = Mouse.Y;
                eTemplate.MouseMove.Left = Mouse.Left;
                eTemplate.MouseMove.Right = Mouse.Right;
                eTemplate.MouseMove.Middle = Mouse.Middle;
                wnd->OnMouseMove(&eTemplate);
            }
        }

        foreach (auto wdg in this->Widgets)
        {
        }

        memmove(MouseArray + 1, MouseArray, sizeof(MouseArray) - sizeof(MouseArray[1]));
        MouseArray[0] = Mouse;

        LastCursor = Cursor;
    }

    Ofast void GUI::PaintDesktop()
    {
        if (DesktopBufferRepaint)
        {
            PutRect(this->DesktopBuffer, this->Desktop, 0x404040);
            DesktopBufferRepaint = false;
        }
        // Well... I have to do some code optimization on DrawOverBitmap. It's too slow and it's not even using SIMD
        memcpy(this->BackBuffer->Data, this->DesktopBuffer->Data, this->DesktopBuffer->Size);
    }

    Ofast void GUI::PaintWidgets()
    {
        Event eTemplate;
        memset(&eTemplate, 0, sizeof(Event));
        foreach (auto wdg in this->Widgets)
            wdg->OnPaint(nullptr);
    }

    O0 void GUI::PaintWindows()
    {
        foreach (auto wnd in this->Windows)
        {
            ScreenBitmap *wndBuffer = wnd->GetBuffer();
            if (unlikely(wndBuffer == nullptr)) // I think "unlikely" is not needed here
                continue;

            Rect WndPos = wnd->GetPosition();

            // Draw window content
            DrawOverBitmap(this->BackBuffer,
                           wndBuffer,
                           WndPos.Top,
                           WndPos.Left);

            /* We can't use memcpy because the window buffer
               is not the same size as the screen buffer
               https://i.imgur.com/OHfaYnS.png */
            // memcpy(this->BackBuffer->Data + wnd->GetPositionPtr()->Top * this->BackBuffer->Width + wnd->GetPositionPtr()->Left, wndBuffer->Data, wndBuffer->Size);

            Rect TopBarPos = WndPos;
            TopBarPos.Top -= 20;
            TopBarPos.Height = 20;
            if (TopBarPos.Top < 0)
            {
                TopBarPos.Top = 0;
                wnd->GetPositionPtr()->Top = 20;
            }

            Rect CloseButtonPos;
            Rect MinimizeButtonPos;

            CloseButtonPos.Left = TopBarPos.Left + TopBarPos.Width - 20;
            CloseButtonPos.Top = TopBarPos.Top;
            CloseButtonPos.Width = 20;
            CloseButtonPos.Height = 20;

            MinimizeButtonPos.Left = TopBarPos.Left + TopBarPos.Width - 60;

            if (unlikely(MouseArray[0].X >= MinimizeButtonPos.Left &&
                         MouseArray[0].X <= CloseButtonPos.Left + CloseButtonPos.Width &&
                         MouseArray[0].Y >= CloseButtonPos.Top &&
                         MouseArray[0].Y <= CloseButtonPos.Top + CloseButtonPos.Height))
            {
                OverlayBufferRepaint = true;
            }

            // Title bar
            if (unlikely(OverlayBufferRepaint))
            {
                if (OverlayFullRepaint)
                {
                    memset(this->OverlayBuffer->Data, 0, this->OverlayBuffer->Size);
                    OverlayFullRepaint = false;
                }

                static bool RepaintNeeded = false;
                DrawShadow(this->OverlayBuffer, wnd->GetPosition());

                Rect MaximizeButtonPos;
                MaximizeButtonPos.Left = TopBarPos.Left + TopBarPos.Width - 40;
                MaximizeButtonPos.Top = TopBarPos.Top;
                MaximizeButtonPos.Width = 20;
                MaximizeButtonPos.Height = 20;

                MinimizeButtonPos.Top = TopBarPos.Top;
                MinimizeButtonPos.Width = 20;
                MinimizeButtonPos.Height = 20;

                PutRect(this->OverlayBuffer, TopBarPos, 0x282828);
                // Title bar buttons (close, minimize, maximize) on the right
                if (MouseArray[0].X >= CloseButtonPos.Left &&
                    MouseArray[0].X <= CloseButtonPos.Left + CloseButtonPos.Width &&
                    MouseArray[0].Y >= CloseButtonPos.Top &&
                    MouseArray[0].Y <= CloseButtonPos.Top + CloseButtonPos.Height)
                {
                    PutRect(this->OverlayBuffer, CloseButtonPos, 0xFF0000);
                    RepaintNeeded = true;
                }
                else
                {
                    PutRect(this->OverlayBuffer, MaximizeButtonPos, 0x282828);
                }

                if (MouseArray[0].X >= MaximizeButtonPos.Left &&
                    MouseArray[0].X <= MaximizeButtonPos.Left + MaximizeButtonPos.Width &&
                    MouseArray[0].Y >= MaximizeButtonPos.Top &&
                    MouseArray[0].Y <= MaximizeButtonPos.Top + MaximizeButtonPos.Height)
                {
                    PutRect(this->OverlayBuffer, MaximizeButtonPos, 0x404040);
                    RepaintNeeded = true;
                }
                else
                {
                    PutRect(this->OverlayBuffer, MaximizeButtonPos, 0x282828);
                }

                if (MouseArray[0].X >= MinimizeButtonPos.Left &&
                    MouseArray[0].X <= MinimizeButtonPos.Left + MinimizeButtonPos.Width &&
                    MouseArray[0].Y >= MinimizeButtonPos.Top &&
                    MouseArray[0].Y <= MinimizeButtonPos.Top + MinimizeButtonPos.Height)
                {
                    PutRect(this->OverlayBuffer, MinimizeButtonPos, 0x404040);
                    RepaintNeeded = true;
                }
                else
                {
                    PutRect(this->OverlayBuffer, MinimizeButtonPos, 0x282828);
                }

                // Title bar icons (close, minimize, maximize) on the right
                for (short i = 0; i < 20; i++)
                {
                    for (short j = 0; j < 20; j++)
                    {
                        if (CloseButton[i * 20 + j] == 1)
                            SetPixel(this->OverlayBuffer,
                                     CloseButtonPos.Left + j,
                                     CloseButtonPos.Top + i,
                                     0xFFFFFF);

                        if ((MaximizeButtonMaximized[i * 20 + j] == 1) && !wnd->IsMaximized())
                            SetPixel(this->OverlayBuffer,
                                     MaximizeButtonPos.Left + j,
                                     MaximizeButtonPos.Top + i,
                                     0xFFFFFF);
                        else if ((MaximizeButtonNormal[i * 20 + j] == 1) && wnd->IsMaximized())
                            SetPixel(this->OverlayBuffer,
                                     MaximizeButtonPos.Left + j,
                                     MaximizeButtonPos.Top + i,
                                     0xFFFFFF);

                        if (MinimizeButton[i * 20 + j] == 1)
                            SetPixel(this->OverlayBuffer,
                                     MinimizeButtonPos.Left + j,
                                     MinimizeButtonPos.Top + i,
                                     0xFFFFFF);
                    }
                }

                Rect wndPos = wnd->GetPosition();

                // Resize hint
                for (short i = 0; i < 20; i++)
                    for (short j = 0; j < 20; j++)
                        if (ResizeHint[i * 20 + j] == 1)
                            SetPixel(this->OverlayBuffer, wndPos.Left + wndPos.Width - 20 + j, wndPos.Top + wndPos.Height - 20 + i, 0xFFFFFF);

                // Title bar border
                PutBorder(this->OverlayBuffer, TopBarPos, 0xFF000000);
                // Window border
                PutBorder(this->OverlayBuffer, wndPos, 0xFF000000);

                Rect TopBarTextPos = TopBarPos;
                TopBarTextPos.Left += 5;
                TopBarTextPos.Top += 5;

                // Title bar text
                DrawString(this->OverlayBuffer, TopBarTextPos, wnd->GetTitle(), 0xFFFFFFFF);

                if (!RepaintNeeded)
                {
                    OverlayBufferRepaint = false;
                    RepaintNeeded = false;
                }
            }
            wnd->OnPaint(nullptr);
        }
        DrawOverBitmap(this->BackBuffer, this->OverlayBuffer, 0, 0);
    }

/*
    "* 2" to increase the size of the cursor
    "/ 2" to decrease the size of the cursor
*/
#define ICON_SIZE

    Ofast void GUI::PaintCursor()
    {
        uint32_t CursorColorInner = 0xFFFFFFFF;
        uint32_t CursorColorOuter = 0xFF000000;

        if (MouseArray[0].X != MouseArray[1].X ||
            MouseArray[0].Y != MouseArray[1].Y ||
            MouseArray[0].Z != MouseArray[1].Z ||
            Cursor != LastCursor)
            CursorBufferRepaint = true;

        if (CursorBufferRepaint)
        {
            memset(this->CursorBuffer->Data, 0, this->CursorBuffer->Size);
            switch (this->Cursor)
            {
            case CursorType::Visible:
            {
                CursorVisible = true;
                break;
            }
            case CursorType::Hidden:
            {
                CursorVisible = false;
                break;
            }
            default:
                fixme("Unknown cursor type %d", this->Cursor);
                [[fallthrough]];
            case CursorType::Arrow:
            {
                if (CursorVisible)
                    for (int i = 0; i < 19; i++)
                    {
                        for (int j = 0; j < 12; j++)
                        {
                            if (CursorArrow[i * 12 + j] == 1)
                            {
                                SetPixel(this->CursorBuffer, j ICON_SIZE, i ICON_SIZE, CursorColorOuter);
                            }
                            else if (CursorArrow[i * 12 + j] == 2)
                            {
                                SetPixel(this->CursorBuffer, j ICON_SIZE, i ICON_SIZE, CursorColorInner);
                            }
                        }
                    }
                break;
            }
            case CursorType::Hand:
            {
                if (CursorVisible)
                    for (int i = 0; i < 24; i++)
                    {
                        for (int j = 0; j < 17; j++)
                        {
                            if (CursorHand[i * 17 + j] == 1)
                            {
                                SetPixel(this->CursorBuffer, j ICON_SIZE, i ICON_SIZE, CursorColorOuter);
                            }
                            else if (CursorHand[i * 17 + j] == 2)
                            {
                                SetPixel(this->CursorBuffer, j ICON_SIZE, i ICON_SIZE, CursorColorInner);
                            }
                        }
                    }
                break;
            }
            case CursorType::Wait:
            {
                if (CursorVisible)
                    for (int i = 0; i < 22; i++)
                    {
                        for (int j = 0; j < 13; j++)
                        {
                            if (CursorWait[i * 13 + j] == 1)
                            {
                                SetPixel(this->CursorBuffer, j ICON_SIZE, i ICON_SIZE, CursorColorOuter);
                            }
                            else if (CursorWait[i * 13 + j] == 2)
                            {
                                SetPixel(this->CursorBuffer, j ICON_SIZE, i ICON_SIZE, CursorColorInner);
                            }
                        }
                    }
                break;
            }
            case CursorType::IBeam:
            {
                if (CursorVisible)
                    for (int i = 0; i < 22; i++)
                    {
                        for (int j = 0; j < 13; j++)
                        {
                            if (CursorIBeam[i * 13 + j] == 1)
                            {
                                SetPixel(this->CursorBuffer, j ICON_SIZE, i ICON_SIZE, CursorColorOuter);
                            }
                            else if (CursorIBeam[i * 13 + j] == 2)
                            {
                                SetPixel(this->CursorBuffer, j ICON_SIZE, i ICON_SIZE, CursorColorInner);
                            }
                        }
                    }
                break;
            }
            case CursorType::ResizeAll:
            {
                if (CursorVisible)
                    for (int i = 0; i < 23; i++)
                    {
                        for (int j = 0; j < 23; j++)
                        {
                            if (CursorResizeAll[i * 23 + j] == 1)
                            {
                                SetPixel(this->CursorBuffer, j ICON_SIZE, i ICON_SIZE, CursorColorOuter);
                            }
                            else if (CursorResizeAll[i * 23 + j] == 2)
                            {
                                SetPixel(this->CursorBuffer, j ICON_SIZE, i ICON_SIZE, CursorColorInner);
                            }
                        }
                    }
                break;
            }
            }
            CursorBufferRepaint = false;
        }
        DrawOverBitmap(this->BackBuffer, this->CursorBuffer, MouseArray[0].Y, MouseArray[0].X);
    }

    Ofast void GUI::Loop()
    {
        /*
        Because we do not use a gpu to do the rendering, we need to do it manually.
        This is why the mouse is slow when we have to draw a bunch of things.
        */
        while (IsRunning)
        {
            FetchInputs();
            PaintDesktop();
            PaintWidgets();
            PaintWindows();
            PaintCursor();
            memcpy(Display->GetBuffer(200)->Buffer, this->BackBuffer->Data, this->BackBuffer->Size);
            Display->SetBuffer(200);
        }
    }

    void GUI::AddWindow(Window *window)
    {
        this->Windows.push_back(window);
    }

    void GUI::AddWidget(WidgetCollection *widget)
    {
        this->Widgets.push_back(widget);
    }

    GUI::GUI()
    {
        Display->CreateBuffer(0, 0, 200);
        this->mem = new Memory::MemMgr;
        this->Desktop.Top = 0;
        this->Desktop.Left = 0;
        this->Desktop.Width = Display->GetBuffer(200)->Width;
        this->Desktop.Height = Display->GetBuffer(200)->Height;

        this->BackBuffer = new ScreenBitmap;
        this->BackBuffer->Width = this->Desktop.Width;
        this->BackBuffer->Height = this->Desktop.Height;
        this->BackBuffer->BitsPerPixel = Display->GetBitsPerPixel();
        this->BackBuffer->Pitch = Display->GetPitch();
        this->BackBuffer->Size = Display->GetBuffer(200)->Size;
        this->BackBuffer->Data = (uint8_t *)this->mem->RequestPages(TO_PAGES(this->BackBuffer->Size));
        memset(this->BackBuffer->Data, 0, this->BackBuffer->Size);

        this->DesktopBuffer = new ScreenBitmap;
        this->DesktopBuffer->Width = this->Desktop.Width;
        this->DesktopBuffer->Height = this->Desktop.Height;
        this->DesktopBuffer->BitsPerPixel = Display->GetBitsPerPixel();
        this->DesktopBuffer->Pitch = Display->GetPitch();
        this->DesktopBuffer->Size = Display->GetBuffer(200)->Size;
        this->DesktopBuffer->Data = (uint8_t *)this->mem->RequestPages(TO_PAGES(this->DesktopBuffer->Size));
        memset(this->DesktopBuffer->Data, 0, this->DesktopBuffer->Size);

        this->OverlayBuffer = new ScreenBitmap;
        this->OverlayBuffer->Width = this->Desktop.Width;
        this->OverlayBuffer->Height = this->Desktop.Height;
        this->OverlayBuffer->BitsPerPixel = Display->GetBitsPerPixel();
        this->OverlayBuffer->Pitch = Display->GetPitch();
        this->OverlayBuffer->Size = Display->GetBuffer(200)->Size;
        this->OverlayBuffer->Data = (uint8_t *)this->mem->RequestPages(TO_PAGES(this->OverlayBuffer->Size));
        memset(this->OverlayBuffer->Data, 0, this->OverlayBuffer->Size);

        this->CursorBuffer = new ScreenBitmap;
        this->CursorBuffer->Width = 25;
        this->CursorBuffer->Height = 25;
        this->CursorBuffer->BitsPerPixel = Display->GetBitsPerPixel();
        this->CursorBuffer->Pitch = Display->GetPitch();
        this->CursorBuffer->Size = this->CursorBuffer->Width * this->CursorBuffer->Height * (this->CursorBuffer->BitsPerPixel / 8);
        this->CursorBuffer->Data = (uint8_t *)this->mem->RequestPages(TO_PAGES(this->CursorBuffer->Size));
        memset(this->CursorBuffer->Data, 0, this->CursorBuffer->Size);

        this->IsRunning = true;
    }

    GUI::~GUI()
    {
        delete this->mem;
        delete this->BackBuffer;
        delete this->DesktopBuffer;
        delete this->OverlayBuffer;
        delete this->CursorBuffer;
        Display->DeleteBuffer(200);
        for (size_t i = 0; i < this->Windows.size(); i++)
            this->Windows.remove(i);
    }
}
