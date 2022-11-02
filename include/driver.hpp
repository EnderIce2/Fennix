#ifndef __FENNIX_KERNEL_DRIVER_H__
#define __FENNIX_KERNEL_DRIVER_H__

#include <types.h>

#include <interrupts.hpp>
#include <vector.hpp>
#include <debug.h>
#include <cpu.hpp>

namespace Driver
{
    enum DriverCode
    {
        ERROR,
        OK,
        INVALID_FEX_HEADER,
        INVALID_DRIVER_DATA,
        NOT_DRIVER,
        DRIVER_RETURNED_ERROR,
        UNKNOWN_DRIVER_TYPE,
        PCI_DEVICE_NOT_FOUND
    };

    class DriverInterruptHook : public Interrupts::Handler
    {
    private:
        void *Handle;
        void *Data;

#if defined(__amd64__)
        void OnInterruptReceived(CPU::x64::TrapFrame *Frame);
#elif defined(__i386__)
        void OnInterruptReceived(void *Frame);
#elif defined(__aarch64__)
        void OnInterruptReceived(void *Frame);
#endif

    public:
        DriverInterruptHook(int Interrupt, void *Address, void *ParamData);
        ~DriverInterruptHook();
    };

    struct DriverFile
    {
        unsigned long DriverUID;
        void *Address;
        DriverInterruptHook *InterruptHook;
    };

    class Driver
    {
    private:
        Vector<DriverFile *> Drivers;
        unsigned long DriverUIDs = 0;

        DriverCode CallDriverEntryPoint(void *fex);

    public:
        Vector<DriverFile *> GetDrivers() { return Drivers; }
        int IOCB(unsigned long DUID, /* KernelCallback */ void *KCB);
        DriverCode LoadDriver(uint64_t DriverAddress, uint64_t Size);
        DriverCode StartDrivers();
        Driver();
        ~Driver();
    };
}

#endif // !__FENNIX_KERNEL_DRIVER_H__
