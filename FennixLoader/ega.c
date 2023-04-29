/* https://wiki.osdev.org/Bare_Bones */
#include <types.h>

enum vga_color
{
    VGA_COLOR_BLACK = 0,
    VGA_COLOR_BLUE = 1,
    VGA_COLOR_GREEN = 2,
    VGA_COLOR_CYAN = 3,
    VGA_COLOR_RED = 4,
    VGA_COLOR_MAGENTA = 5,
    VGA_COLOR_BROWN = 6,
    VGA_COLOR_LIGHT_GREY = 7,
    VGA_COLOR_DARK_GREY = 8,
    VGA_COLOR_LIGHT_BLUE = 9,
    VGA_COLOR_LIGHT_GREEN = 10,
    VGA_COLOR_LIGHT_CYAN = 11,
    VGA_COLOR_LIGHT_RED = 12,
    VGA_COLOR_LIGHT_MAGENTA = 13,
    VGA_COLOR_LIGHT_BROWN = 14,
    VGA_COLOR_WHITE = 15,
};

void outb(uint16_t port, uint8_t val)
{
    asm volatile("outb %0, %1"
                 :
                 : "a"(val), "Nd"(port));
}

static inline uint8_t vga_entry_color(enum vga_color fg, enum vga_color bg)
{
    return fg | bg << 4;
}

static inline uint16_t vga_entry(unsigned char uc, uint8_t color)
{
    return (uint16_t)uc | (uint16_t)color << 8;
}

size_t ega_strlen(const char *str)
{
    size_t len = 0;
    while (str[len])
        len++;
    return len;
}

static const size_t VGA_WIDTH = 80;
static const size_t VGA_HEIGHT = 25;

int terminal_initialized;
size_t terminal_row;
size_t terminal_column;
uint8_t terminal_color;
uint16_t *terminal_buffer;

void terminal_initialize(void)
{
    terminal_initialized = 1;
    terminal_row = 0;
    terminal_column = 0;
    terminal_color = vga_entry_color(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK);
    terminal_buffer = (uint16_t *)0xB8000;
    for (size_t y = 0; y < VGA_HEIGHT; y++)
    {
        for (size_t x = 0; x < VGA_WIDTH; x++)
        {
            const size_t index = y * VGA_WIDTH + x;
            terminal_buffer[index] = vga_entry(' ', terminal_color);
        }
    }
}

void terminal_setcolor(uint8_t color)
{
    terminal_color = color;
}

void terminal_putentryat(char c, uint8_t color, size_t x, size_t y)
{
    if (!terminal_initialized)
        return;
    const size_t index = y * VGA_WIDTH + x;
    terminal_buffer[index] = vga_entry(c, color);
}

void terminal_putchar(char c)
{
    if (!terminal_initialized)
        return;

    switch (c)
    {
    case '\n':
    case '\r':
        terminal_column = 0;
        if (++terminal_row >= VGA_HEIGHT)
        {
            // terminal_row = 0;
            // scroll the screen
            for (size_t y = 0; y < VGA_HEIGHT - 1; y++)
            {
                for (size_t x = 0; x < VGA_WIDTH; x++)
                {
                    const size_t index = y * VGA_WIDTH + x;
                    terminal_buffer[index] = terminal_buffer[index + VGA_WIDTH];
                }
            }
            for (size_t x = 0; x < VGA_WIDTH; x++)
            {
                const size_t index = (VGA_HEIGHT - 1) * VGA_WIDTH + x;
                terminal_buffer[index] = vga_entry(' ', terminal_color);
            }
            terminal_row = VGA_HEIGHT - 1;
        }
        // Update the cursor
        uint16_t cursorLocation = terminal_row * 80 + terminal_column;
        outb(0x3D4, 14);                  // Tell the VGA board we are setting the high cursor byte.
        outb(0x3D5, cursorLocation >> 8); // Send the high cursor byte.
        outb(0x3D4, 15);                  // Tell the VGA board we are setting the low cursor byte.
        outb(0x3D5, cursorLocation);      // Send the low cursor byte.
        return;
    case '\t':
        terminal_column += 4;
        if (terminal_column >= VGA_WIDTH)
        {
            terminal_column = 0;
            if (++terminal_row == VGA_HEIGHT)
                terminal_row = 0;
        }
        return;
    }

    terminal_putentryat(c, terminal_color, terminal_column, terminal_row);
    if (++terminal_column >= VGA_WIDTH)
    {
        terminal_column = 0;
        if (++terminal_row >= VGA_HEIGHT)
        {
            // terminal_row = 0;
            // scroll the screen
            for (size_t y = 0; y < VGA_HEIGHT - 1; y++)
            {
                for (size_t x = 0; x < VGA_WIDTH; x++)
                {
                    const size_t index = y * VGA_WIDTH + x;
                    terminal_buffer[index] = terminal_buffer[index + VGA_WIDTH];
                }
            }
            for (size_t x = 0; x < VGA_WIDTH; x++)
            {
                const size_t index = (VGA_HEIGHT - 1) * VGA_WIDTH + x;
                terminal_buffer[index] = vga_entry(' ', terminal_color);
            }
            terminal_row = VGA_HEIGHT - 1;
        }
    }

    // Update the cursor
    uint16_t cursorLocation = terminal_row * 80 + terminal_column;
    outb(0x3D4, 14);                  // Tell the VGA board we are setting the high cursor byte.
    outb(0x3D5, cursorLocation >> 8); // Send the high cursor byte.
    outb(0x3D4, 15);                  // Tell the VGA board we are setting the low cursor byte.
    outb(0x3D5, cursorLocation);      // Send the low cursor byte.
}
