#if !defined(MYDIRENT_HPP)
#define MYDIRENT_HPP
#include "llcppheaders/llanylib/types/types.hpp"

#include <cstdio>

#pragma region DIRFUNCTIONS
#if defined(PREP_)
	#include <dirent.h>
#else // This is only dummy definitions until compiled
	struct _WDIR;
	struct DIR;
	struct dirent_base {
		long			d_ino;		/* Always zero. */
		unsigned short	d_reclen;	/* Always zero. */
		unsigned short	d_namlen;	/* Length of name in d_name. */
	};
	struct dirent : dirent_base   { ::llcpp::ll_char_t	d_name[FILENAME_MAX]; /* filename */ };
	struct _wdirent : dirent_base { ::llcpp::ll_wchar_t	d_name[FILENAME_MAX]; /* filename */ };
#endif // PREP_

_WDIR* _wopendir(::llcpp::ll_wstring_t);
DIR* opendir(::llcpp::ll_string_t);

struct _wdirent* _wreaddir(_WDIR*);
struct dirent* readdir(DIR*);

int _wclosedir(_WDIR*);
int closedir(DIR*);
#pragma endregion

__LL_NODISCARD__ __LL_INLINE__ _WDIR* ll_opendir(::llcpp::ll_wstring_t directory) noexcept { return _wopendir(directory); }
__LL_NODISCARD__ __LL_INLINE__ DIR* ll_opendir(::llcpp::ll_string_t directory) noexcept { return opendir(directory); }

__LL_NODISCARD__ __LL_INLINE__ struct _wdirent* ll_readdir(_WDIR* directory) noexcept { return _wreaddir(directory); }
__LL_NODISCARD__ __LL_INLINE__ struct dirent* ll_readdir(DIR* directory) noexcept { return readdir(directory); }

__LL_NODISCARD__ __LL_INLINE__ int ll_closedir(_WDIR* directory) noexcept { return _wclosedir(directory); }
__LL_NODISCARD__ __LL_INLINE__ int ll_closedir(DIR* directory) noexcept { return closedir(directory); }

#endif // MYDIRENT_HPP
