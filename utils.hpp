#if !defined(UTILS_HPP)
#define UTILS_HPP

#include "llcppheaders/llanylib/types/types.hpp"

#include <cstring>

#include <functional>   // To include corecrt_wstring.h as std namespace in windows

__LL_NODISCARD__ __LL_INLINE__ ::llcpp::usize ll_strlen(::llcpp::ll_wstring_t str) noexcept { return ::std::wcslen(str); }
__LL_NODISCARD__ __LL_INLINE__ ::llcpp::usize ll_strlen(::llcpp::ll_string_t str) noexcept { return ::std::strlen(str); }

__LL_NODISCARD__ __LL_INLINE__ int ll_strcmp(::llcpp::ll_wstring_t str, ::llcpp::ll_wstring_t str2, ::llcpp::usize size) noexcept { return ::std::wcsncmp(str, str2, size); }
__LL_NODISCARD__ __LL_INLINE__ int ll_strcmp(::llcpp::ll_string_t str, ::llcpp::ll_string_t str2, ::llcpp::usize size) noexcept { return ::std::strncmp(str, str2, size); }

template<::llcpp::usize N, ::llcpp::usize N2>
__LL_NODISCARD__ __LL_INLINE__ int ll_strcmp(const ::llcpp::ll_wchar_t (&str)[N], const ::llcpp::ll_wchar_t (&str2)[N2]) noexcept {
	constexpr ::llcpp::usize SIZE = (N > N2 ? N2 : N) - 1;
	return ::std::wcsncmp(str, str2, SIZE);
}
template<::llcpp::usize N, ::llcpp::usize N2>
__LL_NODISCARD__ __LL_INLINE__ int ll_strcmp(const ::llcpp::ll_char_t (&str)[N], const ::llcpp::ll_char_t (&str2)[N2]) noexcept {
	constexpr ::llcpp::usize SIZE = (N > N2 ? N2 : N) - 1;
	return ::std::strncmp(str, str2, SIZE);
}

#endif // UTILS_HPP
