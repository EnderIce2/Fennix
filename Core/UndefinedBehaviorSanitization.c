/*
   This file is part of Fennix Kernel.

   Fennix Kernel is free software: you can redistribute it and/or
   modify it under the terms of the GNU General Public License as
   published by the Free Software Foundation, either version 3 of
   the License, or (at your option) any later version.

   Fennix Kernel is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with Fennix Kernel. If not, see <https://www.gnu.org/licenses/>.
*/

#include "ubsan.h"

#include <convert.h>
#include <debug.h>

#ifdef DEBUG

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

void __asan_report_load1(void *unknown)
{
    ubsan("load1");
    UNUSED(unknown);
}

void __asan_report_load2(void *unknown)
{
    ubsan("load2");
    UNUSED(unknown);
}

void __asan_report_load4(void *unknown)
{
    ubsan("load4");
    UNUSED(unknown);
}

void __asan_report_load8(void *unknown)
{
    ubsan("load8");
    UNUSED(unknown);
}

void __asan_report_load16(void *unknown)
{
    ubsan("load16");
    UNUSED(unknown);
}

void __asan_report_load_n(void *unknown, uintptr_t size)
{
    ubsan("loadn");
    UNUSED(unknown);
    UNUSED(size);
}

void __asan_report_store1(void *unknown)
{
    ubsan("store1");
    UNUSED(unknown);
}

void __asan_report_store2(void *unknown)
{
    ubsan("store2");
    UNUSED(unknown);
}

void __asan_report_store4(void *unknown)
{
    ubsan("store4");
    UNUSED(unknown);
}

void __asan_report_store8(void *unknown)
{
    ubsan("store8");
    UNUSED(unknown);
}

void __asan_report_store16(void *unknown)
{
    ubsan("store16");
    UNUSED(unknown);
}

void __asan_report_store_n(void *unknown, uintptr_t size)
{
    ubsan("storen");
    UNUSED(unknown);
    UNUSED(size);
}

void __asan_report_load1_noabort(void *unknown)
{
    ubsan("load1");
    UNUSED(unknown);
}

void __asan_report_load2_noabort(void *unknown)
{
    ubsan("load2");
    UNUSED(unknown);
}

void __asan_report_load4_noabort(void *unknown)
{
    ubsan("load4");
    UNUSED(unknown);
}

void __asan_report_load8_noabort(void *unknown)
{
    ubsan("load8");
    UNUSED(unknown);
}

void __asan_report_load16_noabort(void *unknown)
{
    ubsan("load16");
    UNUSED(unknown);
}

void __asan_report_load_n_noabort(void *unknown, uintptr_t size)
{
    ubsan("loadn");
    UNUSED(unknown);
    UNUSED(size);
}

void __asan_report_store1_noabort(void *unknown)
{
    ubsan("store1");
    UNUSED(unknown);
}

void __asan_report_store2_noabort(void *unknown)
{
    ubsan("store2");
    UNUSED(unknown);
}

void __asan_report_store4_noabort(void *unknown)
{
    ubsan("store4");
    UNUSED(unknown);
}

void __asan_report_store8_noabort(void *unknown)
{
    ubsan("store8");
    UNUSED(unknown);
}

void __asan_report_store16_noabort(void *unknown)
{
    ubsan("store16");
    UNUSED(unknown);
}

void __asan_report_store_n_noabort(void *unknown, uintptr_t size)
{
    ubsan("storen");
    UNUSED(unknown);
    UNUSED(size);
}

void __asan_stack_malloc_0(uintptr_t size)
{
    ubsan("stack malloc 0");
    UNUSED(size);
}

void __asan_stack_malloc_1(uintptr_t size)
{
    ubsan("stack malloc 1");
    UNUSED(size);
}

void __asan_stack_malloc_2(uintptr_t size)
{
    ubsan("stack malloc 2");
    UNUSED(size);
}

void __asan_stack_malloc_3(uintptr_t size)
{
    ubsan("stack malloc 3");
    UNUSED(size);
}

void __asan_stack_malloc_4(uintptr_t size)
{
    ubsan("stack malloc 4");
    UNUSED(size);
}

void __asan_stack_malloc_5(uintptr_t size)
{
    ubsan("stack malloc 5");
    UNUSED(size);
}

void __asan_stack_malloc_6(uintptr_t size)
{
    ubsan("stack malloc 6");
    UNUSED(size);
}

void __asan_stack_malloc_7(uintptr_t size)
{
    ubsan("stack malloc 7");
    UNUSED(size);
}

void __asan_stack_malloc_8(uintptr_t size)
{
    ubsan("stack malloc 8");
    UNUSED(size);
}

void __asan_stack_malloc_9(uintptr_t size)
{
    ubsan("stack malloc 9");
    UNUSED(size);
}

void __asan_stack_free_0(void *ptr, uintptr_t size)
{
    ubsan("stack free 0");
    UNUSED(ptr);
    UNUSED(size);
}

void __asan_stack_free_1(void *ptr, uintptr_t size)
{
    ubsan("stack free 1");
    UNUSED(ptr);
    UNUSED(size);
}

void __asan_stack_free_2(void *ptr, uintptr_t size)
{
    ubsan("stack free 2");
    UNUSED(ptr);
    UNUSED(size);
}

void __asan_stack_free_3(void *ptr, uintptr_t size)
{
    ubsan("stack free 3");
    UNUSED(ptr);
    UNUSED(size);
}

void __asan_stack_free_4(void *ptr, uintptr_t size)
{
    ubsan("stack free 4");
    UNUSED(ptr);
    UNUSED(size);
}

void __asan_stack_free_5(void *ptr, uintptr_t size)
{
    ubsan("stack free 5");
    UNUSED(ptr);
    UNUSED(size);
}

void __asan_stack_free_6(void *ptr, uintptr_t size)
{
    ubsan("stack free 6");
    UNUSED(ptr);
    UNUSED(size);
}

void __asan_stack_free_7(void *ptr, uintptr_t size)
{
    ubsan("stack free 7");
    UNUSED(ptr);
    UNUSED(size);
}

void __asan_stack_free_8(void *ptr, uintptr_t size)
{
    ubsan("stack free 8");
    UNUSED(ptr);
    UNUSED(size);
}

void __asan_stack_free_9(void *ptr, uintptr_t size)
{
    ubsan("stack free 9");
    UNUSED(ptr);
    UNUSED(size);
}

void __asan_poison_stack_memory(void *addr, uintptr_t size)
{
    ubsan("poison stack memory");
    UNUSED(addr);
    UNUSED(size);
}

void __asan_unpoison_stack_memory(void *addr, uintptr_t size)
{
    ubsan("unpoison stack memory");
    UNUSED(addr);
    UNUSED(size);
}

void __asan_before_dynamic_init(const char *module_name)
{
    ubsan("before dynamic init");
    UNUSED(module_name);
}

void __asan_after_dynamic_init(void) { ubsan("after dynamic init"); }

void __asan_register_globals(void *unknown, size_t size)
{
    ubsan("register_globals");
    UNUSED(unknown);
    UNUSED(size);
}

void __asan_unregister_globals(void) { ubsan("unregister_globals"); }

void __asan_init(void) { ubsan("init"); }
void __asan_version_mismatch_check_v8(void) { ubsan("version_mismatch_check_v8"); }
void __asan_option_detect_stack_use_after_return(void) { ubsan("stack use after return"); }

__noreturn void __asan_handle_no_return(void)
{
    ubsan("no_return");
    while (1)
        ;
}

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

void __ubsan_handle_type_mismatch_v1(struct type_mismatch_v1_data *type_mismatch, uintptr_t pointer)
{
    struct source_location *location = &type_mismatch->location;
    if (pointer == 0)
    {
        ubsan("\t\tIn File: %s:%i:%i", location->file, location->line, location->column);
        ubsan("Null pointer access.");
    }
    else if (type_mismatch->alignment != 0 && is_aligned(pointer, type_mismatch->alignment))
    {
        ubsan("\t\tIn File: %s:%i:%i", location->file, location->line, location->column);
        ubsan("Unaligned memory access %#lx.", pointer);
    }
    else
    {
        ubsan("\t\tIn File: %s:%i:%i", location->file, location->line, location->column);
        ubsan("%s address %#lx with insufficient space for object of type %s",
              Type_Check_Kinds[type_mismatch->type_check_kind],
              (void *)pointer, type_mismatch->type->name);
    }
}

void __ubsan_handle_add_overflow(struct overflow_data *data)
{
    ubsan("\t\tIn File: %s:%i:%i", data->location.file, data->location.line, data->location.column);
    ubsan("Addition overflow.");
}

void __ubsan_handle_sub_overflow(struct overflow_data *data)
{
    ubsan("\t\tIn File: %s:%i:%i", data->location.file, data->location.line, data->location.column);
    ubsan("Subtraction overflow.");
}

void __ubsan_handle_mul_overflow(struct overflow_data *data)
{
    ubsan("\t\tIn File: %s:%i:%i", data->location.file, data->location.line, data->location.column);
    ubsan("Multiplication overflow.");
}

void __ubsan_handle_divrem_overflow(struct overflow_data *data)
{
    ubsan("\t\tIn File: %s:%i:%i", data->location.file, data->location.line, data->location.column);
    ubsan("Division overflow.");
}

void __ubsan_handle_negate_overflow(struct overflow_data *data)
{
    ubsan("\t\tIn File: %s:%i:%i", data->location.file, data->location.line, data->location.column);
    ubsan("Negation overflow.");
}

void __ubsan_handle_pointer_overflow(struct overflow_data *data)
{
    ubsan("\t\tIn File: %s:%i:%i", data->location.file, data->location.line, data->location.column);
    ubsan("Pointer overflow.");
}

void __ubsan_handle_shift_out_of_bounds(struct shift_out_of_bounds_data *data)
{
    ubsan("\t\tIn File: %s:%i:%i", data->location.file, data->location.line, data->location.column);
    ubsan("Shift out of bounds.");
}

void __ubsan_handle_load_invalid_value(struct invalid_value_data *data)
{
    ubsan("\t\tIn File: %s:%i:%i", data->location.file, data->location.line, data->location.column);
    ubsan("Invalid load value.");
}

void __ubsan_handle_out_of_bounds(struct array_out_of_bounds_data *data)
{
    ubsan("\t\tIn File: %s:%i:%i", data->location.file, data->location.line, data->location.column);
    ubsan("Array out of bounds.");
}

void __ubsan_handle_vla_bound_not_positive(struct negative_vla_data *data)
{
    ubsan("\t\tIn File: %s:%i:%i", data->location.file, data->location.line, data->location.column);
    ubsan("Variable-length argument is negative.");
}

void __ubsan_handle_nonnull_return(struct nonnull_return_data *data)
{
    ubsan("\t\tIn File: %s:%i:%i", data->location.file, data->location.line, data->location.column);
    ubsan("Non-null return is null.");
}

void __ubsan_handle_nonnull_return_v1(struct nonnull_return_data *data)
{
    ubsan("\t\tIn File: %s:%i:%i", data->location.file, data->location.line, data->location.column);
    ubsan("Non-null return is null.");
}

void __ubsan_handle_nonnull_arg(struct nonnull_arg_data *data)
{
    ubsan("\t\tIn File: %s:%i:%i", data->location.file, data->location.line, data->location.column);
    ubsan("Non-null argument is null.");
}

void __ubsan_handle_builtin_unreachable(struct unreachable_data *data)
{
    ubsan("\t\tIn File: %s:%i:%i", data->location.file, data->location.line, data->location.column);
    ubsan("Unreachable code reached.");
}

void __ubsan_handle_invalid_builtin(struct invalid_builtin_data *data)
{
    ubsan("\t\tIn File: %s:%i:%i", data->location.file, data->location.line, data->location.column);
    ubsan("Invalid builtin.");
}

void __ubsan_handle_missing_return(struct unreachable_data *data)
{
    ubsan("\t\tIn File: %s:%i:%i", data->location.file, data->location.line, data->location.column);
    ubsan("Missing return.");
}

void __ubsan_vptr_type_cache(uintptr_t *cache, uintptr_t ptr)
{
    ubsan("Vptr type cache.");
    *cache = ptr;
}

void __ubsan_handle_dynamic_type_cache_miss(struct dynamic_type_cache_miss_data *data, uintptr_t ptr)
{
    ubsan("\t\tIn File: %s:%i:%i", data->location.file, data->location.line, data->location.column);
    ubsan("Dynamic type cache miss.");
    UNUSED(ptr);
}

#endif
