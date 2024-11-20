#include <types.h>

#include <memory.hpp>
#include <debug.h>

// TODO: complete implementation for everything
// TODO: https://wiki.osdev.org/C%2B%2B

#define ATEXIT_MAX_FUNCS 128

typedef unsigned uarch_t;

struct atexit_func_entry_t
{
    /*
     * Each member is at least 4 bytes large. Such that each entry is 12bytes.
     * 128 * 12 = 1.5KB exact.
     **/
    void (*destructor_func)(void *);
    void *obj_ptr;
    void *dso_handle;
};

typedef enum
{
    _URC_NO_REASON = 0,
    _URC_FOREIGN_EXCEPTION_CAUGHT = 1,
    _URC_FATAL_PHASE2_ERROR = 2,
    _URC_FATAL_PHASE1_ERROR = 3,
    _URC_NORMAL_STOP = 4,
    _URC_END_OF_STACK = 5,
    _URC_HANDLER_FOUND = 6,
    _URC_INSTALL_CONTEXT = 7,
    _URC_CONTINUE_UNWIND = 8
} _Unwind_Reason_Code;

struct _Unwind_Context;
typedef unsigned _Unwind_Ptr __attribute__((__mode__(__pointer__)));
typedef unsigned _Unwind_Exception_Class __attribute__((__mode__(__DI__)));
typedef unsigned _Unwind_Word __attribute__((__mode__(__unwind_word__)));
typedef void (*_Unwind_Exception_Cleanup_Fn)(_Unwind_Reason_Code, struct _Unwind_Exception *);
typedef int _Unwind_Action;

struct type_info
{
    const char *name;
};

struct unexpected_handler
{
    void (*unexpected)();
};

struct terminate_handler
{
    void (*handler)();
};

struct _Unwind_Exception
{
    _Unwind_Exception_Class exception_class;
    _Unwind_Exception_Cleanup_Fn exception_cleanup;
#if !defined(__USING_SJLJ_EXCEPTIONS__) && defined(__SEH__)
    _Unwind_Word private_[6];
#else
    _Unwind_Word private_1;
    _Unwind_Word private_2;
#endif
} __attribute__((__aligned__));

struct __cxa_exception
{
#if __LP64__
    size_t referenceCount;
#endif
    type_info *exceptionType;
    void (*exceptionDestructor)(void *);
    unexpected_handler unexpectedHandler;
    terminate_handler terminateHandler;
    __cxa_exception *nextException;
    int handlerCount;

#ifdef __ARM_EABI_UNWINDER__
    __cxa_exception *nextPropagatingException;
    int propagationCount;
#else
    int handlerSwitchValue;
    const unsigned char *actionRecord;
    const unsigned char *languageSpecificData;
    _Unwind_Ptr catchTemp;
    void *adjustedPtr;
#endif
#if !__LP64__
    size_t referenceCount;
#endif
    _Unwind_Exception unwindHeader;
};

extern void *__dso_handle = 0;
atexit_func_entry_t __atexit_funcs[ATEXIT_MAX_FUNCS];
uarch_t __atexit_func_count = 0;

extern "C" int __cxa_atexit(void (*f)(void *), void *objptr, void *dso)
{
    fixme("__cxa_atexit( %p %p %p ) triggered.", f, objptr, dso);
    if (__atexit_func_count >= ATEXIT_MAX_FUNCS)
        return -1;
    __atexit_funcs[__atexit_func_count].destructor_func = f;
    __atexit_funcs[__atexit_func_count].obj_ptr = objptr;
    __atexit_funcs[__atexit_func_count].dso_handle = dso;
    __atexit_func_count++;
    return 0;
}

extern "C" void __cxa_finalize(void *f)
{
    fixme("__cxa_finalize( %p ) triggered.", f);
    uarch_t i = __atexit_func_count;
    if (!f)
    {
        while (i--)
            if (__atexit_funcs[i].destructor_func)
                (*__atexit_funcs[i].destructor_func)(__atexit_funcs[i].obj_ptr);

        return;
    }

    while (i--)
        if (__atexit_funcs[i].destructor_func == f)
        {
            (*__atexit_funcs[i].destructor_func)(__atexit_funcs[i].obj_ptr);
            __atexit_funcs[i].destructor_func = 0;
        }
}

extern "C" _Unwind_Reason_Code __gxx_personality_v0(int version, _Unwind_Action actions, _Unwind_Exception_Class exception_class, _Unwind_Exception *ue_header, _Unwind_Context *context)
{
    fixme("__gxx_personality_v0( %d %p %p %p %p ) triggered.", version, actions, exception_class, ue_header, context);
    return _URC_NO_REASON;
}

extern "C" void _Unwind_Resume(struct _Unwind_Exception *exc) { fixme("_Unwind_Resume( %p ) triggered.", exc); }

static inline size_t align_exception_allocation_size(size_t s, size_t a) { return (s + a - 1) & ~(a - 1); }

void unexpected_header_stub() { fixme("unexpected() called."); }
void terminate_header_stub() { fixme("terminate() called."); }

extern "C" void *__cxa_allocate_exception(size_t thrown_size) throw()
{
    fixme("__cxa_allocate_exception( %d ) triggered.", thrown_size);

    size_t real_size = align_exception_allocation_size(thrown_size + sizeof(__cxa_exception), alignof(__cxa_exception));

    __cxa_exception *header = (__cxa_exception *)kmalloc(real_size);
    if (!header)
    {
        error("Failed to allocate exception.");
        return nullptr;
    }

    header->referenceCount = 1;
    header->exceptionType = nullptr;
    header->exceptionDestructor = nullptr;
    header->unexpectedHandler = {.unexpected = unexpected_header_stub};
    header->terminateHandler = {.handler = terminate_header_stub};
    header->nextException = nullptr;
    header->handlerCount = -1;
    header->handlerSwitchValue = 0;
    header->actionRecord = nullptr;
    header->languageSpecificData = nullptr;
    header->catchTemp = 0;
    header->adjustedPtr = nullptr;

    return header + 1;
}

extern "C" void _Unwind_RaiseException(_Unwind_Exception *exc)
{
    fixme("_Unwind_RaiseException( %p ) triggered.", exc);

    __cxa_exception *header = ((__cxa_exception *)exc) - 1;
    if (header->terminateHandler.handler)
    {
        debug("Calling terminate handler.");
        header->terminateHandler.handler();
    }
    else
    {
        error("Unhandled exception.");
        CPU::Stop();
    }

    CPU::Halt(true);
}

extern "C" void __cxa_throw(void *thrown_object, void *tinfo, void (*dest)(void *))
{
    fixme("__cxa_throw( %p %p %p ) triggered.", thrown_object, tinfo, dest);

    __cxa_exception *header = ((__cxa_exception *)thrown_object) - 1;
    header->exceptionType = (type_info *)tinfo;
    header->exceptionDestructor = dest;

    _Unwind_RaiseException(&header->unwindHeader);
}

extern "C" void __cxa_rethrow() { fixme("__cxa_rethrow() triggered."); }

extern "C" void __cxa_pure_virtual() { fixme("__cxa_pure_virtual() triggered."); }

extern "C" void __cxa_throw_bad_array_new_length() { fixme("__cxa_throw_bad_array_new_length() triggered."); }

extern "C" void __cxa_free_exception(void *thrown_exception) { fixme("__cxa_free_exception( %p ) triggered.", thrown_exception); }

#if __GNUC__ > 4 || (__GNUC__ == 4 && __GNUC_MINOR__ >= 4)
extern "C" void *__cxa_begin_catch(void *e) throw()
#else
extern "C" void *__cxa_begin_catch(void *e)
#endif
{
    fixme("__cxa_begin_catch( %p ) triggered.", e);
    return (void *)0;
}

extern "C" void __cxa_end_catch() { fixme("__cxa_end_catch() triggered."); }

__extension__ typedef int __guard __attribute__((mode(__DI__)));

extern "C" int __cxa_guard_acquire(__guard *g)
{
    fixme("__cxa_guard_acquire( %p ) triggered.", g);
    return !*(char *)(g);
}

extern "C" void __cxa_guard_release(__guard *g)
{
    fixme("__cxa_guard_release( %p ) triggered.", g);
    *(char *)g = 1;
}

extern "C" void __cxa_guard_abort(__guard *g) { fixme("__cxa_guard_abort( %p ) triggered.", g); }

extern "C" void *_ZTVN10__cxxabiv117__class_type_infoE(void)
{
    fixme("_ZTVN10__cxxabiv117__class_type_infoE() triggered.");
    return (void *)0;
}

extern "C" void *_ZTVN10__cxxabiv120__si_class_type_infoE(void)
{
    fixme("_ZTVN10__cxxabiv120__si_class_type_infoE() triggered.");
    return (void *)0;
}

extern "C" void *_ZTIPh(void)
{
    fixme("_ZTIPh() triggered.");
    return (void *)0;
}

extern "C" void *_ZTIPKc(void)
{
    fixme("_ZTIPKc() triggered.");
    return (void *)0;
}

extern "C" void *_ZTIi(void)
{
    fixme("_ZTIi() triggered.");
    return (void *)0;
}
