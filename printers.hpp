#if !defined(PRINTER_HPP)
#define PRINTER_HPP
#include "llcppheaders/llanylib/types/types.hpp"

#include <cstdio>
#include <functional>   // To include corecrt_wstring.h as std namespace in windows

FILE* out	= stdout;
FILE* err	= stderr;
FILE* in	= stdin;

#define PRINT_NUMBER(str, v)						\
	do {											\
		if constexpr (::llcpp::USE_WIDE_CHAR)		\
			(void)::std::fwprintf(out, L"" str, v);	\
		else (void)::std::fprintf(out, "%llu", v);	\
	} while(0)

template<::llcpp::usize N>
__LL_INLINE__ void print(const ::llcpp::char_type (&v)[N]) noexcept {
//	(void)::std::fwrite(v, 1, N - 1, out);
}

__LL_INLINE__ void print(const ::llcpp::char_type* begin, const ::llcpp::char_type* end) noexcept {
//	(void)::std::fwrite(begin, 1, end - begin, out);
}
	
__LL_INLINE__ void print(const ::llcpp::u64 v) noexcept {
	//PRINT_NUMBER("%llu", v);
}
__LL_INLINE__ void print(const ::llcpp::u32 v) noexcept {
	//PRINT_NUMBER("%u", v);
}

__LL_INLINE__ void print(const ::llcpp::i64 v) noexcept {
	//PRINT_NUMBER("%ll", v);
}
__LL_INLINE__ void print(const ::llcpp::i32 v) noexcept {
	//PRINT_NUMBER("%i", v);
}

namespace extra {
template<::llcpp::usize N>
__LL_INLINE__ void print2(const ::llcpp::char_type (&v)[N]) noexcept {
	(void)::std::fwrite(v, 1, N - 1, out);
}

__LL_INLINE__ void print2(const ::llcpp::char_type* begin, const ::llcpp::char_type* end) noexcept {
	(void)::std::fwrite(begin, 1, end - begin, out);
}
	
__LL_INLINE__ void print2(const ::llcpp::u64 v) noexcept {
	PRINT_NUMBER("%llu", v);
}
__LL_INLINE__ void print2(const ::llcpp::u32 v) noexcept {
	PRINT_NUMBER("%u", v);
}

__LL_INLINE__ void print2(const ::llcpp::i64 v) noexcept {
	PRINT_NUMBER("%ll", v);
}
__LL_INLINE__ void print2(const ::llcpp::i32 v) noexcept {
	PRINT_NUMBER("%i", v);
}
	
} // namespace extra






#undef PRINT_NUMBER

__LL_INLINE__ FILE* ll_fopen(::llcpp::ll_wstring_t filename, ::llcpp::ll_wstring_t mode) noexcept {
	return _wfopen(filename, mode);
}

__LL_INLINE__ FILE* ll_fopen(::llcpp::ll_string_t filename, ::llcpp::ll_string_t mode) noexcept {
	return fopen(filename, mode);
}

#endif // PRINTER_HPP
