#include "ubsan.h"

#include <convert.h>
#include <debug.h>

// TODO: implement:
/*
__ubsan_handle_type_mismatch_v1_abort
__ubsan_handle_add_overflow_abort
__ubsan_handle_sub_overflow_abort
__ubsan_handle_mul_overflow_abort
__ubsan_handle_negate_overflow_abort
__ubsan_handle_divrem_overflow_abort
__ubsan_handle_shift_out_of_bounds_abort
__ubsan_handle_out_of_bounds_abort
__ubsan_handle_vla_bound_not_positive_abort
__ubsan_handle_float_cast_overflow
__ubsan_handle_float_cast_overflow_abort
__ubsan_handle_load_invalid_value_abort
__ubsan_handle_invalid_builtin_abort
__ubsan_handle_function_type_mismatch_abort
__ubsan_handle_nonnull_return_v1
__ubsan_handle_nonnull_return_v1_abort
__ubsan_handle_nullability_return_v1
__ubsan_handle_nullability_return_v1_abort
__ubsan_handle_nonnull_arg_abort
__ubsan_handle_nullability_arg
__ubsan_handle_nullability_arg_abort
__ubsan_handle_pointer_overflow_abort
__ubsan_handle_cfi_check_fail
*/

extern void __asan_report_load1(void *unknown) { ubsan("load1"); }
extern void __asan_report_load2(void *unknown) { ubsan("load2"); }
extern void __asan_report_load4(void *unknown) { ubsan("load4"); }
extern void __asan_report_load8(void *unknown) { ubsan("load8"); }
extern void __asan_report_load16(void *unknown) { ubsan("load16"); }
extern void __asan_report_load_n(void *unknown, uintptr_t size) { ubsan("loadn"); }

extern void __asan_report_store1(void *unknown) { ubsan("store1"); }
extern void __asan_report_store2(void *unknown) { ubsan("store2"); }
extern void __asan_report_store4(void *unknown) { ubsan("store4"); }
extern void __asan_report_store8(void *unknown) { ubsan("store8"); }
extern void __asan_report_store16(void *unknown) { ubsan("store16"); }
extern void __asan_report_store_n(void *unknown, uintptr_t size) { ubsan("storen"); }

extern void __asan_report_load1_noabort(void *unknown) { ubsan("load1"); }
extern void __asan_report_load2_noabort(void *unknown) { ubsan("load2"); }
extern void __asan_report_load4_noabort(void *unknown) { ubsan("load4"); }
extern void __asan_report_load8_noabort(void *unknown) { ubsan("load8"); }
extern void __asan_report_load16_noabort(void *unknown) { ubsan("load16"); }
extern void __asan_report_load_n_noabort(void *unknown, uintptr_t size) { ubsan("loadn"); }

extern void __asan_report_store1_noabort(void *unknown) { ubsan("store1"); }
extern void __asan_report_store2_noabort(void *unknown) { ubsan("store2"); }
extern void __asan_report_store4_noabort(void *unknown) { ubsan("store4"); }
extern void __asan_report_store8_noabort(void *unknown) { ubsan("store8"); }
extern void __asan_report_store16_noabort(void *unknown) { ubsan("store16"); }
extern void __asan_report_store_n_noabort(void *unknown, uintptr_t size) { ubsan("storen"); }

extern void __asan_stack_malloc_0(uintptr_t size) { ubsan("stack malloc 0"); }
extern void __asan_stack_malloc_1(uintptr_t size) { ubsan("stack malloc 1"); }
extern void __asan_stack_malloc_2(uintptr_t size) { ubsan("stack malloc 2"); }
extern void __asan_stack_malloc_3(uintptr_t size) { ubsan("stack malloc 3"); }
extern void __asan_stack_malloc_4(uintptr_t size) { ubsan("stack malloc 4"); }
extern void __asan_stack_malloc_5(uintptr_t size) { ubsan("stack malloc 5"); }
extern void __asan_stack_malloc_6(uintptr_t size) { ubsan("stack malloc 6"); }
extern void __asan_stack_malloc_7(uintptr_t size) { ubsan("stack malloc 7"); }
extern void __asan_stack_malloc_8(uintptr_t size) { ubsan("stack malloc 8"); }
extern void __asan_stack_malloc_9(uintptr_t size) { ubsan("stack malloc 9"); }

extern void __asan_stack_free_0(void *ptr, uintptr_t size) { ubsan("stack free 0"); }
extern void __asan_stack_free_1(void *ptr, uintptr_t size) { ubsan("stack free 1"); }
extern void __asan_stack_free_2(void *ptr, uintptr_t size) { ubsan("stack free 2"); }
extern void __asan_stack_free_3(void *ptr, uintptr_t size) { ubsan("stack free 3"); }
extern void __asan_stack_free_4(void *ptr, uintptr_t size) { ubsan("stack free 4"); }
extern void __asan_stack_free_5(void *ptr, uintptr_t size) { ubsan("stack free 5"); }
extern void __asan_stack_free_6(void *ptr, uintptr_t size) { ubsan("stack free 6"); }
extern void __asan_stack_free_7(void *ptr, uintptr_t size) { ubsan("stack free 7"); }
extern void __asan_stack_free_8(void *ptr, uintptr_t size) { ubsan("stack free 8"); }
extern void __asan_stack_free_9(void *ptr, uintptr_t size) { ubsan("stack free 9"); }

extern void __asan_poison_stack_memory(void *addr, uintptr_t size) { ubsan("poison stack memory"); }
extern void __asan_unpoison_stack_memory(void *addr, uintptr_t size) { ubsan("unpoison stack memory"); }

extern void __asan_before_dynamic_init(const char *module_name) { ubsan("before dynamic init"); }
extern void __asan_after_dynamic_init(void) { ubsan("after dynamic init"); }

extern void __asan_register_globals(void *unknown, size_t size) { ubsan("register_globals"); }
extern void __asan_unregister_globals(void) { ubsan("unregister_globals"); }

extern void __asan_init(void) { ubsan("init"); }
extern void __asan_version_mismatch_check_v8(void) { ubsan("version_mismatch_check_v8"); }
extern void __asan_option_detect_stack_use_after_return(void) { ubsan("stack use after return"); }
extern __noreturn void __asan_handle_no_return(void) { ubsan("no_return"); }

#define is_aligned(value, alignment) !(value & (alignment - 1))

const char *Type_Check_Kinds[] = {
    "Load of",
    "Store to",
    "Reference binding to",
    "Member access within",
    "Member call on",
    "Constructor call on",
    "Downcast of",
    "Downcast of",
    "Upcast of",
    "Cast to virtual base of",
};

// Prevent huge spam from ubsan
bool UBSANMsg(const char *file, uint32_t line, uint32_t column)
{
    // blacklist
    if (strstr(file, "liballoc") || strstr(file, "cwalk"))
        return false;

    static char *onceFile[512] = {"\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"};
    static uint32_t onceLine[512] = {0};
    static uint32_t onceColumn[512] = {0};
    static uint32_t onceCount = 0;

    for (uint32_t i = 0; i < onceCount; i++)
        if ((!strcmp(onceFile[i], file)) && onceLine[i] == line && onceColumn[i] == column)
            return false;

    onceFile[onceCount] = (char *)file;
    onceLine[onceCount] = line;
    onceColumn[onceCount] = column;
    ubsan("\t\tIn File: %s:%i:%i", file, line, column);
    onceCount++;
    return true;
}

void __ubsan_handle_type_mismatch_v1(struct type_mismatch_v1_data *type_mismatch, uintptr_t pointer)
{
    struct source_location *location = &type_mismatch->location;
    if (pointer == 0)
    {
        if (UBSANMsg(location->file, location->line, location->column))
            ubsan("Null pointer access.");
    }
    else if (type_mismatch->alignment != 0 && is_aligned(pointer, type_mismatch->alignment))
    {
        if (UBSANMsg(location->file, location->line, location->column))
            ubsan("Unaligned memory access %#llx.", pointer);
    }
    else
    {
        if (UBSANMsg(location->file, location->line, location->column))
            ubsan("%s address %#llx with insufficient space for object of type %s",
                  Type_Check_Kinds[type_mismatch->type_check_kind], (void *)pointer, type_mismatch->type->name);
    }
}

void __ubsan_handle_add_overflow(struct overflow_data *data)
{
    if (UBSANMsg(data->location.file, data->location.line, data->location.column))
        ubsan("Addition overflow.");
}

void __ubsan_handle_sub_overflow(struct overflow_data *data)
{
    if (UBSANMsg(data->location.file, data->location.line, data->location.column))
        ubsan("Subtraction overflow.");
}

void __ubsan_handle_mul_overflow(struct overflow_data *data)
{
    if (UBSANMsg(data->location.file, data->location.line, data->location.column))
        ubsan("Multiplication overflow.");
}

void __ubsan_handle_divrem_overflow(struct overflow_data *data)
{
    if (UBSANMsg(data->location.file, data->location.line, data->location.column))
        ubsan("Division overflow.");
}

void __ubsan_handle_negate_overflow(struct overflow_data *data)
{
    if (UBSANMsg(data->location.file, data->location.line, data->location.column))
        ubsan("Negation overflow.");
}

void __ubsan_handle_pointer_overflow(struct overflow_data *data)
{
    if (UBSANMsg(data->location.file, data->location.line, data->location.column))
        ubsan("Pointer overflow.");
}

void __ubsan_handle_shift_out_of_bounds(struct shift_out_of_bounds_data *data)
{
    if (UBSANMsg(data->location.file, data->location.line, data->location.column))
        ubsan("Shift out of bounds.");
}

void __ubsan_handle_load_invalid_value(struct invalid_value_data *data)
{
    if (UBSANMsg(data->location.file, data->location.line, data->location.column))
        ubsan("Invalid load value.");
}

void __ubsan_handle_out_of_bounds(struct array_out_of_bounds_data *data)
{
    if (UBSANMsg(data->location.file, data->location.line, data->location.column))
        ubsan("Array out of bounds.");
}

void __ubsan_handle_vla_bound_not_positive(struct negative_vla_data *data)
{
    if (UBSANMsg(data->location.file, data->location.line, data->location.column))
        ubsan("Variable-length argument is negative.");
}

void __ubsan_handle_nonnull_return(struct nonnull_return_data *data)
{
    if (UBSANMsg(data->location.file, data->location.line, data->location.column))
        ubsan("Non-null return is null.");
}

void __ubsan_handle_nonnull_return_v1(struct nonnull_return_data *data)
{
    if (UBSANMsg(data->location.file, data->location.line, data->location.column))
        ubsan("Non-null return is null.");
}

void __ubsan_handle_nonnull_arg(struct nonnull_arg_data *data)
{
    if (UBSANMsg(data->location.file, data->location.line, data->location.column))
        ubsan("Non-null argument is null.");
}

void __ubsan_handle_builtin_unreachable(struct unreachable_data *data)
{
    if (UBSANMsg(data->location.file, data->location.line, data->location.column))
        ubsan("Unreachable code reached.");
}

void __ubsan_handle_invalid_builtin(struct invalid_builtin_data *data)
{
    if (UBSANMsg(data->location.file, data->location.line, data->location.column))
        ubsan("Invalid builtin.");
}

void __ubsan_handle_missing_return(struct unreachable_data *data)
{
    if (UBSANMsg(data->location.file, data->location.line, data->location.column))
        ubsan("Missing return.");
}
