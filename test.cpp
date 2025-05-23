#include "llcppheaders/llanylib/utils_base/ArrayBase.hpp"
#include "llcppheaders/llanylib/utils_base/hash/LlanyHash.hpp"
#include "llcppheaders/magic_enum/include/magic_enum/magic_enum.hpp"

#include "printers.hpp"
#include "mydirent.hpp"
#include "utils.hpp"
#include "filefunc.hpp"

#include <chrono>

using String =  ::llcpp::meta::utils::ArrayBase<::llcpp::char_type>;

struct StringBuffer {
	String path;
	String::iterator last;
};

using DirType = ::std::remove_pointer_t<decltype(ll_opendir(::std::declval<String::iterator>()))>;
using EntryType = ::std::remove_pointer_t<decltype(ll_readdir(::std::declval<DirType*>()))>;
using FolderIterator = std::function<::llcpp::ll_bool_t(StringBuffer& full_filename, const FileType type, ::llcpp::u64 size)>;

// Helper to close directory on function normal exit/early exit
class DirHandler {
	private:
		DirType* dir;
	public:
		DirHandler(DirType* dir) noexcept : dir(dir) {}
		~DirHandler() noexcept { (void)ll_closedir(this->dir); }

		DirHandler(const DirHandler&) noexcept = delete;
		DirHandler& operator=(const DirHandler&) noexcept = delete;
		DirHandler(DirHandler&&) noexcept = delete;
		DirHandler& operator=(DirHandler&&) noexcept = delete;
};

constexpr ::llcpp::char_type filename[] = __LL_L ".\\.patata.exe";

__LL_NODISCARD__ __LL_INLINE__ ::llcpp::ll_bool_t concatOrContinueCheck(StringBuffer& directory, ::llcpp::string filename, const size_t size) noexcept {
	// Concat file name into buffer
	// If filename does not fit in current buffer
	// We display an error, and skip this file
	if(!directory.path.inRange(directory.last + size + 1)) {
		print(__LL_L "Filename '");
		print(filename, filename + size);
		print(__LL_L "' does not fit in current buffer. \n\tBuffer size: ");
		print(directory.path.size());
		print(__LL_L "\n\tFilled: ");
		print(directory.last - directory.path.data());
		print(__LL_L "\n\tFilename size: ");
		print(size);
		print(__LL_L "\n");
		return ::llcpp::LL_FALSE;
	}
	else return ::llcpp::LL_TRUE;
}
__LL_NODISCARD__ __LL_INLINE__  ::llcpp::ll_bool_t concatOrContinue(StringBuffer& directory, ::llcpp::string filename, const size_t size) noexcept {
	// Concat file name into buffer
	// If filename does not fit in current buffer
	// We display an error, and skip this file
	if(!concatOrContinueCheck(directory, filename, size))
		return ::llcpp::LL_FALSE;

	// Copy filename into buffer
	*(directory.last++) = (__LL_L "/")[0];		// [TOCHECK] Optimize this?
	::std::memcpy(directory.last, filename, size * sizeof(String::value_type));
	directory.last += size;
	*(directory.last) = (__LL_L "")[0];			// [TOCHECK] Optimize this?

	return ::llcpp::LL_TRUE;
}
__LL_INLINE__ void resetPreviousFileName(StringBuffer& directory, const size_t size) noexcept {
	directory.last -= (size + 1);			// Remove filename + "\\"
	*(directory.last) = (__LL_L "")[0];		// [TOCHECK] Optimize this?
}
__LL_NODISCARD__ ::llcpp::ll_bool_t iterateOverDirectory(StringBuffer& directory, FolderIterator iter) noexcept {
	DirType* dir = ll_opendir(directory.path.data());
	if(!dir) {
		print(__LL_L "Error opening directory '");
		print(directory.path.data(), directory.last);
		print(__LL_L "'\n");
		return false;
	}

	DirHandler handler(dir);

	for(EntryType* entry = ll_readdir(dir); entry; entry = ll_readdir(dir)) {
		// Insert black list here!
		if(
			(entry->d_namlen == 1 && ll_strcmp(__LL_L ".", entry->d_name) == 0)
			|| (entry->d_namlen == 2 && ll_strcmp(__LL_L "..", entry->d_name) == 0)
			//|| (entry->d_namlen == 14 && ll_strcmp(filename, entry->d_name) == 0)
		)
			continue;

		// Get filename size for further operations
		size_t size = ll_strlen(entry->d_name);

#if defined(__LL_POSIX_SYSTEM) || defined(__LL_UNIX_SYSTEM)

		// If entry has file type data, we continue normally
		if(entry->d_type != DT_UNKNOWN) {
			if (!concatOrContinue(directory, entry->d_name, size))
				continue;
			else if (iter(directory, convert<char>(entry->d_type)))
				return true;
			else resetPreviousFileName(directory, size);
		}
		else

#endif // __LL_POSIX_SYSTEM || __LL_UNIX_SYSTEM

		// If buffer could not be concatenated
		if (!concatOrContinue(directory, entry->d_name, size))
			continue;
		// If file is concatenated, we check its type
		else {
			// Identify file type with stat
			::llcpp::u64 sss;
			FileType type = indentifyFile(directory.path.data(), sss);
			if(type == FileType::Error || type == FileType::Unknown) {
				::extra::print2(__LL_L "Could not stat filename '");
				::extra::print2(directory.path.data(), directory.last);
				::extra::print2(__LL_L "'\n");
				resetPreviousFileName(directory, size);
				continue;
			}
			else if (iter(directory, type, sss))
				return true;
			else resetPreviousFileName(directory, size);
		}
	}
	return false;
}

constexpr ::llcpp::meta::utils::hash::LlanyHash<::llcpp::u8> HASHER;

int main(int argc, const char** argv) {
	FILE* out = ll_fopen(filename, __LL_L "w");

	// Check parameters and directory to check
	//::llcpp::string directory_base = reinterpret_cast<::llcpp::string*>(argv)[1];
	::llcpp::string directory_base = __LL_L "./";

	// Set path buffer
	String::value_type buffer[512]{};
	StringBuffer buffer_holder = { { buffer, ::llcpp::meta::traits::array_size<decltype(buffer)> }, buffer };

	// Concat path
	size_t size = ll_strlen(directory_base);
	if (!concatOrContinueCheck(buffer_holder, directory_base, size))
		return -1;
	::std::memcpy(buffer_holder.last, directory_base, size * sizeof(String::value_type));
	buffer_holder.last += size;
	*(buffer_holder.last) = (__LL_L "")[0];			// [TOCHECK] Optimize this?

	::llcpp::u64 bytes_read{};
	auto start = ::std::chrono::high_resolution_clock::now();

	FolderIterator itself = nullptr;
	auto f = [&itself, &out, &bytes_read](StringBuffer& full_filename, const FileType type, ::llcpp::u64 size) -> ::llcpp::ll_bool_t {
		if (type == FileType::Regular ||type == FileType::RegularRead) {
			FILE* f = ll_fopen(full_filename.path.data(), __LL_L "rb");
			::std::fseek(f, 0, SEEK_END);
			size = ::std::ftell(f);

			decltype(HASHER)::byte* data = new (::std::nothrow) decltype(HASHER)::byte[size];

			if(data) {
				::std::fseek(f, 0, SEEK_SET);
				::std::fread(data, 1, size, f);
				auto hash = HASHER.llanyHash64Combine(data, size);
				print(__LL_L "Object found: ");
				print(full_filename.path.data(), full_filename.last);
				print(__LL_L "\t::");
				print(static_cast<::llcpp::u8>(type));
				print(__LL_L "\t::");
				print(hash);
				print(__LL_L "\n");
				::std::fwrite(full_filename.path.data(), 1, full_filename.last - full_filename.path.data(), out);
				::std::fwrite(&hash, sizeof(hash), 1, out);
				delete[] data;
				bytes_read += size;
			}
			else {
				::extra::print2(__LL_L "No space avaible to hash: ");
				::extra::print2(static_cast<::llcpp::i64>(size));
				::extra::print2(__LL_L "\n");
			}
			::std::fclose(f);
		}
		else if(type != FileType::Directory) {
			::extra::print2(__LL_L "Object found: ");
			::extra::print2(full_filename.path.data(), full_filename.last);
			::extra::print2(__LL_L "\t::");
			::extra::print2(static_cast<::llcpp::u8>(type));
			::extra::print2(__LL_L "\n");
		}

		if(type == FileType::Directory) {
			return iterateOverDirectory(full_filename, itself);
		}
		// Tell to dont exit the iterations
		else return false;
	};
	itself = f;
	(void)iterateOverDirectory(buffer_holder, f);

	::extra::print2(__LL_L "Time: ");
	::extra::print2(static_cast<::llcpp::u64>(::std::chrono::duration_cast<::std::chrono::milliseconds>(::std::chrono::high_resolution_clock::now() - start).count()));
	::extra::print2(__LL_L "\n Bytes read: ");
	::extra::print2(bytes_read);
	::extra::print2(__LL_L "\n");

	fclose(out);

	return 0;
}

