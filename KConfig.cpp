#include <kconfig.hpp>

#include <convert.h>
#include <cargs.h>
#include <printf.h>

#include "kernel.h"

// TODO: Implement proper fprintf
EXTERNC void fprintf(FILE *stream, const char *Format, ...)
{
    va_list args;
    va_start(args, Format);
    vprintf_(Format, args);
    va_end(args);
}

// TODO: Implement proper fputs
EXTERNC void fputs(const char *s, FILE *stream) { printf_("%s", s); }

static struct cag_option ConfigOptions[] = {
    {.identifier = 'a',
     .access_letters = "aA",
     .access_name = "alloc",
     .value_name = "TYPE",
     .description = "Memory allocator to use"},

    {.identifier = 'c',
     .access_letters = "cC",
     .access_name = "cores",
     .value_name = "VALUE",
     .description = "Number of cores to use (0 = all; 1 is the first code, not 0)"},

    {.identifier = 't',
     .access_letters = "tT",
     .access_name = "tasking",
     .value_name = "MODE",
     .description = "Tasking mode (multi, single)"},

    {.identifier = 'd',
     .access_letters = "dD",
     .access_name = "drvdir",
     .value_name = "PATH",
     .description = "Directory to load drivers from"},

    {.identifier = 'i',
     .access_letters = "iI",
     .access_name = "init",
     .value_name = "PATH",
     .description = "Path to init program"},

    {.identifier = 'o',
     .access_letters = NULL,
     .access_name = "ioc",
     .value_name = "BOOL",
     .description = "Enable Interrupts On Crash. If enabled, the navigation keys will be enabled on crash."},

    {.identifier = 'h',
     .access_letters = "h",
     .access_name = "help",
     .value_name = NULL,
     .description = "Show help on screen and halt"}};

KernelConfig ParseConfig(char *Config)
{
    int argc = 0;
    char **argv = nullptr;

    struct KernelConfig config = {Memory::MemoryAllocatorType::Pages,
                                  0,
                                  {'/', 's', 'y', 's', 't', 'e', 'm', '/', 'd', 'r', 'i', 'v', 'e', 'r', 's', '\0'},
                                  {'/', 's', 'y', 's', 't', 'e', 'm', '/', 'i', 'n', 'i', 't', '\0'},
                                  0};

    if (Config == NULL)
    {
        KPrint("Warning! Empty kernel parameters! Using default values.");
        return config;
    }
    else
    {
        KPrint("Kernel parameters: %s", Config);
        goto Parse;
    }

Parse:
{
    // Based on https://stackoverflow.com/a/2568792
    char *DestinationBuffer;
    char *TempScan;

    char **TempBuffer = (char **)kmalloc(sizeof(char *) * 10);
    char **ArgvTemp = TempBuffer;

    while (isspace((unsigned char)*Config))
        Config++;

    Config = TempScan = strdup(Config);

    int Itr = 10;
    while (1)
    {
        while (isspace((unsigned char)*TempScan))
            TempScan++;
        if (*TempScan == '\0')
            break;

        if (++argc >= Itr)
        {
            Itr += Itr / 2;
            TempBuffer = (char **)krealloc(TempBuffer, Itr * sizeof(char *));
            ArgvTemp = TempBuffer + (argc - 1);
        }

        *(ArgvTemp++) = DestinationBuffer = TempScan;

        while (1)
        {
            char TempChar0 = *(TempScan++);
            switch (TempChar0)
            {
            case '\0':
                goto Completed;
            case ' ':
            case '\t':
            case '\n':
            case '\f':
            case '\r':
            case '\v':
            case '\b':
                goto EmptyBufferAndComplete;
            case '\\':
            {
                if ((*(DestinationBuffer++) = *(TempScan++)) == '\0')
                    goto Completed;
                break;
            }
            case '\'':
            {
                while (1)
                {
                    char TempChar1 = (*TempScan)++;
                    switch (TempChar1)
                    {
                    case '\0':
                    {
                        KPrint("\eFF2200Unterminated string constant in kernel parameters. (0)");
                        CPU::Stop();
                        break;
                    }
                    case '\'':
                        *DestinationBuffer = '\0';
                        goto OutsideLoop;
                    case '\\':
                        TempChar1 = (*TempScan)++;
                        switch (TempChar1)
                        {
                        case '\0':
                        {
                            KPrint("\eFF2200Unterminated string constant in kernel parameters. (1)");
                            CPU::Stop();
                            break;
                        }
                        default:
                            *(DestinationBuffer++) = '\\';
                            break;

                        case '\\':
                        case '\'':
                            break;
                        }
                        [[fallthrough]];
                    default:
                        *(DestinationBuffer++) = TempChar1;
                        break;
                    }
                }
            OutsideLoop:
                break;
            }
            case '"':
            {
                while (1)
                {
                    char TempChar2 = *(TempScan++);
                    switch (TempChar2)
                    {
                    case '\0':
                    {
                        KPrint("\eFF2200Unterminated string constant in kernel parameters. (2)");
                        CPU::Stop();
                        break;
                    }
                    case '"':
                        (*DestinationBuffer) = '\0';
                        goto OutsideLoop2;
                    case '\\':
                    {
                        char TempChar3 = (*TempScan)++;
                        switch (TempChar3)
                        {
                        case 'a':
                            TempChar3 = '\a';
                            break;
                        case 'b':
                            TempChar3 = '\b';
                            break;
                        case 't':
                            TempChar3 = '\t';
                            break;
                        case 'n':
                            TempChar3 = '\n';
                            break;
                        case 'v':
                            TempChar3 = '\v';
                            break;
                        case 'f':
                            TempChar3 = '\f';
                            break;
                        case 'r':
                            TempChar3 = '\r';
                            break;
                        case '\0':
                        {
                            KPrint("\eFF2200Unterminated string constant in kernel parameters. (3)");
                            CPU::Stop();
                        }
                        }
                        TempChar2 = TempChar3;
                        [[fallthrough]];
                    }
                    default:
                        *(DestinationBuffer++) = TempChar2;
                        break;
                    }
                OutsideLoop2:
                    break;
                }
                break;
            }
            default:
                *(DestinationBuffer++) = TempChar0;
            }
        }

    EmptyBufferAndComplete:
        *DestinationBuffer = '\0';
    }

    KPrint("Failed to parse kernel parameters string: %s", Config);
    CPU::Stop();

Completed:
    argv = TempBuffer;
    if (argc == 0)
        kfree((void *)Config);

    goto ParseSuccess;
}

ParseSuccess:
{
#ifdef DEBUG
    for (int i = 0; i < argc; i++)
        KPrint("\e22AAFFargv[%d] = %s", i, argv[i]);
    KPrint("\e22AAFFargc = %d", argc);
#endif
    char identifier;
    const char *value;
    cag_option_context context;

    cag_option_prepare(&context, ConfigOptions, CAG_ARRAY_SIZE(ConfigOptions), argc, argv);
    while (cag_option_fetch(&context))
    {
        identifier = cag_option_get(&context);
        switch (identifier)
        {
        case 'a':
        {
            value = cag_option_get_value(&context);
            if (strcmp(value, "xallocv1") == 0)
            {
                KPrint("\eAAFFAAUsing XallocV1 as memory allocator");
                config.AllocatorType = Memory::MemoryAllocatorType::XallocV1;
            }
            else if (strcmp(value, "liballoc11") == 0)
            {
                KPrint("\eAAFFAAUsing Liballoc11 as memory allocator");
                config.AllocatorType = Memory::MemoryAllocatorType::liballoc11;
            }
            else if (strcmp(value, "pages") == 0)
            {
                KPrint("\eAAFFAAUsing Pages as memory allocator");
                config.AllocatorType = Memory::MemoryAllocatorType::Pages;
            }
            else
            {
                KPrint("\eAAFFAAUnknown memory allocator: %s", value);
                config.AllocatorType = Memory::MemoryAllocatorType::None;
            }
            break;
        }
        case 'c':
        {
            value = cag_option_get_value(&context);
            KPrint("\eAAFFAAUsing %s cores", atoi(value) ? value : "all");
            config.Cores = atoi(value);
            break;
        }
        case 't':
        {
            value = cag_option_get_value(&context);
            if (strcmp(value, "multi") == 0)
            {
                KPrint("\eAAFFAAUsing Multi-Tasking Scheduler");
                config.SchedulerType = 1;
            }
            else if (strcmp(value, "single") == 0)
            {
                KPrint("\eAAFFAAUsing Mono-Tasking Scheduler");
                config.SchedulerType = 0;
            }
            else
            {
                KPrint("\eAAFFAAUnknown scheduler: %s", value);
                config.SchedulerType = 0;
            }
            break;
        }
        case 'd':
        {
            value = cag_option_get_value(&context);
            strcpy(config.DriverDirectory, value);
            KPrint("\eAAFFAAUsing %s as driver directory", value);
            break;
        }
        case 'i':
        {
            value = cag_option_get_value(&context);
            strcpy(config.InitPath, value);
            KPrint("\eAAFFAAUsing %s as init program", value);
            break;
        }
        case 'o':
        {
            value = cag_option_get_value(&context);
            strcmp(value, "true") ? config.InterruptsOnCrash = false : config.InterruptsOnCrash = true;
            KPrint("\eAAFFAAInterrupts on crash: %s", value);
            break;
        }
        case 'h':
        {
            KPrint("\n---------------------------------------------------------------------------\nUsage: kernel.fsys [OPTION]...\nKernel configuration.");
            cag_option_print(ConfigOptions, CAG_ARRAY_SIZE(ConfigOptions), nullptr);
            KPrint("\eFF2200System Halted.");
            CPU::Stop();
        }
        }
    }
    return config;
}
    return config;
}
