#if !defined(FILEFUNC_HPP)
#define FILEFUNC_HPP
#include "llcppheaders/llanylib/traits_base/type_traits.hpp"

#include <sys/stat.h>

__LL_NODISCARD__ __LL_INLINE__ int ll_stat(struct _stat64& st, ::llcpp::ll_wstring_t filename) noexcept {
    return _wstat64(filename, &st);
}
__LL_NODISCARD__ __LL_INLINE__ int ll_stat(struct stat& st, ::llcpp::ll_string_t filename) noexcept {
    return stat(filename, &st);
}

template<class T>
concept IsStat = requires (struct stat st, T t) { { ll_stat(st, t) } noexcept; };
template<class T>
concept IswStat = requires (struct _stat64 st, T t) { { ll_stat(st, t) } noexcept; };

template<class T>
using StatType = typename ::std::disjunction<
    ::llcpp::meta::traits::BoolConstantContainer<IsStat<T>, struct stat>,
    ::llcpp::meta::traits::BoolConstantContainer<IswStat<T>, struct _stat64>,
    ::llcpp::meta::traits::TrueContainerEmptyClass
>::U;

using stat_type_t = StatType<::llcpp::string>;

static_assert(!::std::is_same_v<stat_type_t, ::llcpp::Emptyclass>,
    "Error generating stat type");

enum class FileType : ::llcpp::u8 {
#if defined(__LL_WINDOWS_SYSTEM) || defined(__LL_MINGW)
    Directory,
    CHR,
    Pipe,
    Regular,
    RegularRead,
    RegularWrite,
    RegularExec,
#elif defined(__LL_POSIX_SYSTEM) || defined(__LL_UNIX_SYSTEM)
    FIFO, CHR, Directory, BLK, REG, LNK, SOCK, WHT,
#endif // OS

    Error,
    Unknown
};

template<class T>
__LL_NODISCARD__ __LL_INLINE__ FileType convert(const T d_name) noexcept { return FileType::Unknown; }

#if defined(__LL_POSIX_SYSTEM) || defined(__LL_UNIX_SYSTEM)
template<>
__LL_NODISCARD__ __LL_INLINE__ FileType convert(const char d_name) noexcept {
	switch (d_name) {
		case DT_FIFO: 		return FileType::FIFO;
		case DT_CHR: 		return FileType::CHR;
		case DT_DIR: 		return FileType::Directory;
		case DT_BLK: 		return FileType::BLK;
		case DT_REG: 		return FileType::REG;
		case DT_LNK: 		return FileType::LNK;
		case DT_SOCK: 		return FileType::SOCK;
		case DT_WHT: 		return FileType::WHT;
		case DT_UNKNOWN:
		default: 			return FileType::Unknown;
	}
}
#elif defined(__LL_WINDOWS_SYSTEM) || defined(__LL_MINGW)
using mode_t = unsigned short;

#endif // __LL_POSIX_SYSTEM || __LL_UNIX_SYSTEM

template<>
__LL_NODISCARD__ __LL_INLINE__ FileType convert(const mode_t st_mode) noexcept {
#if defined(__LL_WINDOWS_SYSTEM) || defined(__LL_MINGW)
	/*if(S_ISDIR(st_mode)) 		return FileType::Directory;
	else if(S_ISCHR(st_mode)) 	return FileType::CHR;
	else if(S_ISBLK(st_mode)) 	return FileType::BLK;
	else if(S_ISREG(st_mode)) 	return FileType::REG;
	else if(S_ISFIFO(st_mode))	return FileType::FIFO;
	//else if(S_ISLNK(st_mode))	return FileType::LNK;
	//else if(S_ISSOCK(st_mode))	return FileType::SOCK;
	else 						return FileType::Unknown;*/

	if(st_mode & _S_IFDIR) return FileType::Directory;
	else if(st_mode & _S_IFIFO) return FileType::Pipe;
	else if(st_mode & _S_IFREG) {
		if (st_mode & _S_IREAD) return FileType::RegularRead;
		else if (st_mode & _S_IWRITE) return FileType::RegularWrite;
		else if (st_mode & _S_IEXEC) return FileType::RegularExec;
		return FileType::Regular;
	}
	else if(st_mode & _S_IFCHR) return FileType::CHR;
	else return FileType::Unknown;
#elif defined(__LL_POSIX_SYSTEM) || defined(__LL_UNIX_SYSTEM)
	if(S_ISDIR(st_mode)) 		return FileType::Directory;
	else if(S_ISCHR(st_mode)) 	return FileType::CHR;
	else if(S_ISBLK(st_mode)) 	return FileType::BLK;
	else if(S_ISREG(st_mode)) 	return FileType::REG;
	else if(S_ISFIFO(st_mode))	return FileType::FIFO;
	else if(S_ISLNK(st_mode))	return FileType::LNK;
	else if(S_ISSOCK(st_mode))	return FileType::SOCK;
	else 						return FileType::Unknown;
#endif // OS
}

__LL_NODISCARD__ __LL_INLINE__ FileType indentifyFile(const ::llcpp::string filename, ::llcpp::u64& size) noexcept {
    stat_type_t st;
	if(ll_stat(st, filename) != 0) return FileType::Error;
	else {
		size = st.st_size;
		return convert<mode_t>(st.st_mode);
	}
}


#endif // FILEFUNC_HPP
