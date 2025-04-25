#include "llcppheaders/llanylib/types/Boolean.hpp"
#include "llcppheaders/llanylib/utils_base/ArrayBase.hpp"
#include "llcppheaders/llanylib/utils_base/hash/LlanyHash.hpp"
#include "llcppheaders/llanylib/utils_base/constant_friendly.hpp"

#include "printers.hpp"
#include "mydirent.hpp"
#include "utils.hpp"
#include "filefunc.hpp"

#include <chrono>

#define __LL_DEBUG

#if !defined(__LL_MAX_PATH) || __LL_MAX_PATH <= 0
	#define __LL_MAX_PATH _MAX_PATH
#endif // __LL_MAX_PATH

#if defined(__LL_DEBUG)
	#define ll_print print
#else
	#define ll_print(...)
#endif // __LL_DEBUG

// MACROS:
//	__LL_DEBUG
//	__TRUST_MAX_PATH
//	__LL_MAX_PATH

struct Job {
	::llcpp::char_type file_path[__LL_MAX_PATH];
	::llcpp::Boolean is_directory;

	constexpr Job() noexcept {}
	constexpr ~Job() noexcept {}
};

using String =  ::llcpp::meta::utils::ArrayBase<::llcpp::char_type>;

// Easy buffer manager of paths by char type
class StringBuffer {
	private:
		String path;
		String::iterator last;

	public:
		template<::llcpp::usize N>
		__LL_INLINE__ constexpr StringBuffer(String::value_type buffer[N]) noexcept
			: path(buffer, N)
			, last(buffer)
		{}
		__LL_INLINE__ constexpr ~StringBuffer() noexcept = default;

		__LL_NODISCARD__ __LL_INLINE__ void resetLast() noexcept {
			*(this->last) = ::llcpp::ZERO_VALUE<decltype(*(this->last))>;
		}
		__LL_NODISCARD__ __LL_INLINE__ ::llcpp::ll_bool_t firstInser(String::const_iterator str) noexcept {
			// Concat path
			size_t size = ll_strlen(str);
			if (!this->concatOrContinueCheck(str, size))
				return -1;
			::llcpp::meta::utils::hash::LlanyHash<::llcpp::char_type>().memcopy(this->last, str, size * sizeof(String::value_type));
			//::std::memcpy(this->last, str, size * sizeof(String::value_type));
			this->last += size;
			this->resetLast();
		}
		__LL_NODISCARD__ __LL_INLINE__ ::llcpp::ll_bool_t concatOrContinueCheck(::llcpp::string filename, const size_t size) noexcept {
			// Concat file name into buffer
			// If filename does not fit in current buffer
			// We display an error, and skip this file
			if(!this->path.inRange(this->last + size + 1)) {
				ll_print(__LL_L "Filename '");
				ll_print(filename, filename + size);
				ll_print(__LL_L "' does not fit in current buffer. \n\tBuffer size: ");
				ll_print(this->path.size());
				ll_print(__LL_L "\n\tFilled: ");
				ll_print(this->last - this->path.data());
				ll_print(__LL_L "\n\tFilename size: ");
				ll_print(size);
				ll_print(__LL_L "\n");
				return ::llcpp::LL_FALSE;
			}
			else return ::llcpp::LL_TRUE;
		}
		__LL_NODISCARD__ __LL_INLINE__  ::llcpp::ll_bool_t concatOrContinue(::llcpp::string filename, const size_t size) noexcept {
	#if !defined(__TRUST_MAX_PATH)
			// Concat file name into buffer
			// If filename does not fit in current buffer
			// We display an error, and skip this file
			if(!this->concatOrContinueCheck(filename, size))
				return ::llcpp::LL_FALSE;
	#endif // __TRUST_MAX_PATH
		
			// Copy filename into buffer
			*(this->last++) = (__LL_L "/")[0];		// [TOCHECK] Optimize this?
			::std::memcpy(this->last, filename, size * sizeof(String::value_type));
			this->last += size;
			this->resetLast();
		
			return ::llcpp::LL_TRUE;
		}
		__LL_INLINE__ void resetPreviousFileName(const size_t size) noexcept {
			this->last -= (size + 1);			// Remove filename + "\\"
			this->resetLast();
		}
};

namespace llcpp {

template<::llcpp::usize N, class _T>
class AtomitLIFO {
	#pragma region Types
	public:
		// Class related
		using _MyType		= AtomitLIFO;

		// Types and enums
		using T				= _T;
		using type			= T;	// standard
		using value_type	= T;	// standard

	#pragma endregion
	#pragma region Expresions
	public:
		static constexpr ll_bool_t N	= _N;

	#pragma endregion
	#pragma region Attributes
	private:
		T lifo[_MyType::N];
		::std::atomic usize pos;

	#pragma endregion
	#pragma region Functions
		#pragma region Constructors
	public:
		constexpr AtomitLIFO() noexcept = default;	// Init zeroized
		constexpr ~AtomitLIFO() noexcept = default;

		#pragma endregion
		#pragma region CopyMove
	public:
		constexpr AtomitLIFO(const AtomitLIFO& other) noexcept = delete;
		constexpr AtomitLIFO& operator=(const AtomitLIFO& other) noexcept = delete;
		constexpr AtomitLIFO(AtomitLIFO&& other) noexcept = delete;
		constexpr AtomitLIFO& operator=(AtomitLIFO&& other) noexcept = delete;

		constexpr AtomitLIFO(volatile const AtomitLIFO& other) noexcept = delete;
		constexpr AtomitLIFO& operator=(volatile const AtomitLIFO& other) noexcept = delete;
		constexpr AtomitLIFO(volatile AtomitLIFO&& other) noexcept = delete;
		constexpr AtomitLIFO& operator=(volatile AtomitLIFO&& other) noexcept = delete;

		#pragma endregion
		#pragma region ClassReferenceOperators
	public:
		__LL_NODISCARD__ constexpr explicit operator const AtomitLIFO*() const noexcept { return this; }
		__LL_NODISCARD__ constexpr explicit operator AtomitLIFO*() noexcept { return this; }

		#pragma endregion
		#pragma region ClassFunctions
	public:
		__LL_NODISCARD__ constexpr T pop() noexcept requires(::std::is_pointer<T>) {
			usize pos = this->pos++;


		}

		#pragma endregion

	#pragma endregion
};

} //



constexpr ::llcpp::meta::utils::hash::LlanyHash<
	::llcpp::u8,
	::llcpp::meta::utils::hash::Algorithm<
		::llcpp::u64,
		::llcpp::meta::utils::hash::AlgorithmMode::CRC
	>
> HASHER;

thread_local ::llcpp::char_type PATH_JOB[_MAX_PATH];































