#include "llcppheaders/llanylib/utils_base/ArrayBase.hpp"

#include "printers.hpp"
#include "mydirent.hpp"
#include "utils.hpp"
#include "filefunc.hpp"

using String =  ::llcpp::meta::utils::ArrayBase<::llcpp::char_type>;

struct StringBuffer {
	String path;
	String::iterator last;
};

using DirType = ::std::remove_pointer_t<decltype(ll_opendir(::std::declval<String::iterator>()))>;
using EntryType = ::std::remove_pointer_t<decltype(ll_readdir(::std::declval<DirType*>()))>;
using FolderIterator = std::function<::llcpp::ll_bool_t(StringBuffer& full_filename, const FileType type)>;

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
	*(directory.last++) = (__LL_L "\\")[0];		// [TOCHECK] Optimize this?
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
		if(ll_strcmp(__LL_L ".", entry->d_name) == 0 || ll_strcmp(__LL_L "..", entry->d_name) == 0)
			continue;

		// Get filename size for further operations
		size_t size = ll_strlen(entry->d_name);

#if defined(__LL_POSIX_SYSTEM) || defined(__LL_UNIX_SYSTEM)

		// If entry has file type data, we continue normally
		if(entry->d_type != DT_UNKNOWN) {
			if (!concatOrContinue(directory, entry->d_name, size))
				continue;
			else if (iter(directory, convert<char>(entry->d_type))) {
				resetPreviousFileName(directory, size);
				return true;
			}
		}
		else

#endif // __LL_POSIX_SYSTEM || __LL_UNIX_SYSTEM

		// If buffer could not be concatenated
		if (!concatOrContinue(directory, entry->d_name, size))
			continue;
		// If file is concatenated, we check its type
		else {
			// Identify file type with stat
			FileType type = indentifyFile(directory.path.data());
			if(type == FileType::Error) {
				print(__LL_L "Could not stat filename '");
				print(directory.path.data(), directory.last);
				print(__LL_L "'\n");
				continue;
			}
			else if (iter(directory, type)) {
				resetPreviousFileName(directory, size);
				return true;
			}
		}
	}
	return true;
}

int main(int argc, const char** argv) {
	// Check parameters and directory to check
	//::llcpp::string directory_base = reinterpret_cast<::llcpp::string*>(argv)[1];
	::llcpp::string directory_base = __LL_L ".\\";

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

	// Iterater over all folders
	::llcpp::u8 depth{};

	FolderIterator itself = nullptr;
	auto f = [&depth, &itself](StringBuffer& full_filename, const FileType type) -> ::llcpp::ll_bool_t {
		print(__LL_L "Object found: ");
		print(full_filename.path.begin(), full_filename.last);
		print(__LL_L "\n");

		if(type == FileType::Directory && depth < 3) {
			++depth;
			return iterateOverDirectory(full_filename, itself);
		}
		// Tell to dont exit the iterations
		else return false;
	};
	itself = f;
	(void)iterateOverDirectory(buffer_holder, f);

	return 0;
}