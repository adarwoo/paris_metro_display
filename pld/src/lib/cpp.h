#ifndef cpp_h_HAS_ALREADY_BEEN_INCLUDED
#define cpp_h_HAS_ALREADY_BEEN_INCLUDED
/**
 * @addtogroup service
 * @{
 * @addtogroup preprocessor
 * @{
 * Preprocessor crazy macros
 * @file
 * Crazy pre-processor helping macros
 */
#include <stdlib.h> // For size_t on the AVR

/** En empty macro @hideinitializer */
#define EMPTY()

/** Do not expand just yet @hideinitializer */
#define DEFER(id) id EMPTY()

/** Bound the list @hideinitializer */
#define OBSTRUCT(...) __VA_ARGS__ DEFER(EMPTY)()

/** Take a variadic list and expand it @hideinitializer */
#define EXPAND(...) __VA_ARGS__

/** Check the Nth is not empty @hideinitializer */
#define CHECK_N(x, n, ...) n

/** Check the list @hideinitializer */
#define CHECK(...) CHECK_N(__VA_ARGS__, 0,)

/** Probe @hideinitializer */
#define PROBE(x) x, 1,

/** Add to a list @hideinitializer */
#define CAT(a, ...) PRIMITIVE_CAT(a, __VA_ARGS__)

/** Add to a list @hideinitializer */
#define PRIMITIVE_CAT(a, ...) a ## __VA_ARGS__

/** Condition if @hideinitializer */
#define IIF(c) PRIMITIVE_CAT(IIF_, c)

/** False expansion for IFF @hideinitializer */
#define IIF_0(t, ...) __VA_ARGS__

/** True expansion for IIF @hideinitializer */
#define IIF_1(t, ...) t

/** Test x is parenthesis @hideinitializer */
#define IS_PAREN(x) CHECK(IS_PAREN_PROBE x)

/** Probe parenthesis in list @hideinitializer */
#define IS_PAREN_PROBE(...) PROBE(~)

/** Count number of elements in the list */
#define COUNTOF(x) (sizeof(x)/sizeof(x[0]))

// Alternative type safe C++ template macros
#ifdef __cplusplus

/** Same as COUNTOF but type safe. For C++ only. */
template <typename T, size_t N>
constexpr size_t countof(T const (&)[N]) noexcept
{
   return N;
}
#endif

/**@}*//**@}*/
#endif /* ndef cpp_h_HAS_ALREADY_BEEN_INCLUDED */
