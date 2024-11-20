#include <types.h>

#include <debug.h>

int Entry(void *Info);

void _start(void *Raw)
{
    error("ERROR! INVALID BOOT PROTOCOL!");
    while (1)
        asmv("hlt");
    Entry(NULL);
    return;
}
// C stuff