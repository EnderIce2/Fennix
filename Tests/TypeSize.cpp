#ifdef DEBUG

#include <debug.h>

__constructor void TestTypeSize()
{
    debug("--- INT TYPES ---");
    debug("sizeof(__INT8_TYPE__) = %lld", sizeof(__INT8_TYPE__));
    debug("sizeof(__INT16_TYPE__) = %lld", sizeof(__INT16_TYPE__));
    debug("sizeof(__INT32_TYPE__) = %lld", sizeof(__INT32_TYPE__));
    debug("sizeof(__INT64_TYPE__) = %lld", sizeof(__INT64_TYPE__));

    debug("-- UINT TYPES ---");
    debug("sizeof(__UINT8_TYPE__) = %lld", sizeof(__INT8_TYPE__));
    debug("sizeof(__UINT16_TYPE__) = %lld", sizeof(__INT16_TYPE__));
    debug("sizeof(__UINT32_TYPE__) = %lld", sizeof(__INT32_TYPE__));
    debug("sizeof(__UINT64_TYPE__) = %lld", sizeof(__INT64_TYPE__));

    debug("--- INT LEAST TYPES ---");
    debug("sizeof(__INT_LEAST8_TYPE__) = %lld", sizeof(__INT_LEAST8_TYPE__));
    debug("sizeof(__INT_LEAST16_TYPE__) = %lld", sizeof(__INT_LEAST16_TYPE__));
    debug("sizeof(__INT_LEAST32_TYPE__) = %lld", sizeof(__INT_LEAST32_TYPE__));
    debug("sizeof(__INT_LEAST64_TYPE__) = %lld", sizeof(__INT_LEAST64_TYPE__));

    debug("--- UINT LEAST TYPES ---");
    debug("sizeof(__UINT_LEAST8_TYPE__) = %lld", sizeof(__UINT_LEAST8_TYPE__));
    debug("sizeof(__UINT_LEAST16_TYPE__) = %lld", sizeof(__UINT_LEAST16_TYPE__));
    debug("sizeof(__UINT_LEAST32_TYPE__) = %lld", sizeof(__UINT_LEAST32_TYPE__));
    debug("sizeof(__UINT_LEAST64_TYPE__) = %lld", sizeof(__UINT_LEAST64_TYPE__));

    debug("--- INT FAST TYPES ---");
    debug("sizeof(__INT_FAST8_TYPE__) = %lld", sizeof(__INT_FAST8_TYPE__));
    debug("sizeof(__INT_FAST16_TYPE__) = %lld", sizeof(__INT_FAST16_TYPE__));
    debug("sizeof(__INT_FAST32_TYPE__) = %lld", sizeof(__INT_FAST32_TYPE__));
    debug("sizeof(__INT_FAST64_TYPE__) = %lld", sizeof(__INT_FAST64_TYPE__));

    debug("--- UINT FAST TYPES ---");
    debug("sizeof(__UINT_FAST8_TYPE__) = %lld", sizeof(__UINT_FAST8_TYPE__));
    debug("sizeof(__UINT_FAST16_TYPE__) = %lld", sizeof(__UINT_FAST16_TYPE__));
    debug("sizeof(__UINT_FAST32_TYPE__) = %lld", sizeof(__UINT_FAST32_TYPE__));
    debug("sizeof(__UINT_FAST64_TYPE__) = %lld", sizeof(__UINT_FAST64_TYPE__));

    debug("--- INTPTR TYPES ---");
    debug("sizeof(__INTPTR_TYPE__) = %lld", sizeof(__INTPTR_TYPE__));
    debug("sizeof(__UINTPTR_TYPE__) = %lld", sizeof(__UINTPTR_TYPE__));

    debug("--- OTHER TYPES ---");
    debug("sizeof(__PTRDIFF_TYPE__) = %lld", sizeof(__PTRDIFF_TYPE__));
    debug("sizeof(__SIZE_TYPE__) = %lld", sizeof(__SIZE_TYPE__));
    debug("sizeof(__WCHAR_TYPE__) = %lld", sizeof(__WCHAR_TYPE__));
    debug("sizeof(__WINT_TYPE__) = %lld", sizeof(__WINT_TYPE__));
    debug("sizeof(__SIG_ATOMIC_TYPE__) = %lld", sizeof(__SIG_ATOMIC_TYPE__));

    debug("--- INTX MAX TYPES ---");
    debug("__INT8_MAX__ = %#llx", __INT8_MAX__);
    debug("__INT16_MAX__ = %#llx", __INT16_MAX__);
    debug("__INT32_MAX__ = %#llx", __INT32_MAX__);
    debug("__INT64_MAX__ = %#llx", __INT64_MAX__);

    debug("--- UINTX MAX TYPES ---");
    debug("__UINT8_MAX__ = %#llx", __UINT8_MAX__);
    debug("__UINT16_MAX__ = %#llx", __UINT16_MAX__);
    debug("__UINT32_MAX__ = %#llx", __UINT32_MAX__);
    debug("__UINT64_MAX__ = %#llx", __UINT64_MAX__);

    // debug("--- INTMAX TYPES ---");
    // debug("__INTMAX_TYPE__ = %#llx", __INTMAX_TYPE__);
    // debug("__UINTMAX_TYPE__ = %#llx", __UINTMAX_TYPE__);

    debug("--- INTLEASTX MAX ---");
    debug("__INT_LEAST8_MAX__ = %#llx", __INT_LEAST8_MAX__);
    debug("__INT_LEAST16_MAX__ = %#llx", __INT_LEAST16_MAX__);
    debug("__INT_LEAST32_MAX__ = %#llx", __INT_LEAST32_MAX__);
    debug("__INT_LEAST64_MAX__ = %#llx", __INT_LEAST64_MAX__);

    debug("--- UINTLEASTX MAX ---");
    debug("__UINT_LEAST8_MAX__ = %#llx", __UINT_LEAST8_MAX__);
    debug("__UINT_LEAST16_MAX__ = %#llx", __UINT_LEAST16_MAX__);

    debug("--- INTFASTX MAX ---");
    debug("__INT_FAST8_MAX__ = %#llx", __INT_FAST8_MAX__);
    debug("__INT_FAST16_MAX__ = %#llx", __INT_FAST16_MAX__);
    debug("__INT_FAST32_MAX__ = %#llx", __INT_FAST32_MAX__);
    debug("__INT_FAST64_MAX__ = %#llx", __INT_FAST64_MAX__);

    debug("--- UINTFASTX MAX ---");
    debug("__UINT_FAST8_MAX__ = %#llx", __UINT_FAST8_MAX__);
    debug("__UINT_FAST16_MAX__ = %#llx", __UINT_FAST16_MAX__);
    debug("__UINT_FAST32_MAX__ = %#llx", __UINT_FAST32_MAX__);
    debug("__UINT_FAST64_MAX__ = %#llx", __UINT_FAST64_MAX__);

    debug("--- INTPTR MAX ---");
    debug("__INTPTR_MAX__ = %#llx", __INTPTR_MAX__);
    debug("__UINTPTR_MAX__ = %#llx", __UINTPTR_MAX__);

    debug("--- OTHER MAX ---");
    debug("__PTRDIFF_MAX__ = %#llx", __PTRDIFF_MAX__);
    debug("__SIZE_MAX__ = %#llx", __SIZE_MAX__);
    debug("__WCHAR_MAX__ = %#llx", __WCHAR_MAX__);
    debug("__WINT_MAX__ = %#llx", __WINT_MAX__);
    debug("__SIG_ATOMIC_MAX__ = %#llx", __SIG_ATOMIC_MAX__);
    debug("__INTMAX_MAX__ = %#llx", __INTMAX_MAX__);
    debug("__UINTMAX_MAX__ = %#llx", __UINTMAX_MAX__);
}

#endif
