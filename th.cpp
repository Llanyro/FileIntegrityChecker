#include "llcppheaders/llanylib/types/Boolean.hpp"
#include "llcppheaders/llanylib/utils_base/ArrayBase.hpp"
#include "llcppheaders/llanylib/utils_base/hash/LlanyHash.hpp"
#include "llcppheaders/llanylib/utils_base/constant_friendly.hpp"
#include "llcppheaders/llanylib/utils_base/AtomicLIFO.hpp"

#include "printers.hpp"
#include "mydirent.hpp"
#include "utils.hpp"
#include "filefunc.hpp"

#include <cstdlib>
#include <chrono>
#include <thread>

#define __LL_DEBUG

#if !defined(__LL_MAX_THREAD)
	#define __LL_MAX_THREAD 50
#elif __LL_MAX_THREAD < 0
	#undef __LL_MAX_THREAD
	#define __LL_MAX_THREAD 50
#endif // __LL_MAX_THREAD

#if !defined(__LL_MAX_PATH)
	#define __LL_MAX_PATH _MAX_PATH
#elif __LL_MAX_PATH < 0
	#undef __LL_MAX_PATH
	#define __LL_MAX_PATH _MAX_PATH
#endif // __LL_MAX_PATH

#if defined(__LL_DEBUG)
	#define ll_print print
#else
	#define ll_print(...)
#endif // __LL_DEBUG

// MACROS:
//	__LL_DEBUG			(optional define)				[Enables debug traces										]
//	__TRUST_MAX_PATH	(optional define)				[Does not check out of bounds max path						]
//	__LL_MAX_PATH		(optional define unsigned)		[Sets max path number in bytes								]
//	__LL_MAX_THREAD		(optional define unsigned)		[Sets max number of threads, not confuse with thread used	]
//	__LL_ATOMIC_MODE	(optional define unsigned[0-2])	[Sets atomic mode											]
//		0: std::atomic postincrement
//		1: std::atomic preincrement
//		2: llcpp manual preincrement/postincrement

constexpr ::llcpp::meta::utils::hash::LlanyHash<
	::llcpp::u8,
	::llcpp::meta::utils::hash::Algorithm<
		::llcpp::u64,
		::llcpp::meta::utils::hash::AlgorithmMode::CRC
	>
> HASHER;
// 1 ms = 1000000 ns
// 0.1 ms = 100000 ns
constexpr ::std::chrono::nanoseconds SLEEP_TIME(100000);

// Easy buffer manager of paths by char type
class StringBuffer {
	public:
		using String		=  ::llcpp::meta::utils::ArrayBase<::llcpp::char_type>;

	private:
		String path;
		String::iterator last;

	public:
		template<::llcpp::usize N>
		__LL_INLINE__ constexpr StringBuffer(String::value_type buffer[N]) noexcept
			: path(buffer, N)
			, last(buffer)
		{}
		__LL_INLINE__ constexpr StringBuffer(String::iterator begin, const ::llcpp::usize end, const ::llcpp::usize filled) noexcept
			: path(begin, end)
			, last(begin + filled)
		{}
		__LL_INLINE__ constexpr ~StringBuffer() noexcept = default;

		__LL_NODISCARD__ __LL_INLINE__ ::llcpp::usize getFilled() const noexcept { return this->last - this->path.data(); }
		__LL_NODISCARD__ __LL_INLINE__ String::iterator data() noexcept { return this->path.data(); }
		__LL_NODISCARD__ __LL_INLINE__ String::const_iterator data() const noexcept { return this->path.data(); }

		__LL_NODISCARD__ __LL_INLINE__ String::iterator getLast() noexcept { return this->last; }
		__LL_NODISCARD__ __LL_INLINE__ String::const_iterator getLast() const noexcept { return this->last; }

		__LL_INLINE__ void resetLast() noexcept {
			*(this->last) = ::llcpp::ZERO_VALUE<String::value_type>;
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
				ll_print(this->getFilled());
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

void iterateOverDirectory(StringBuffer& directory) noexcept;

class Job {
	private:
		::llcpp::char_type file_path[__LL_MAX_PATH];
		::llcpp::Boolean is_directory;
		::llcpp::u16 name_bytes;

	public:
		constexpr Job() noexcept
			: file_path()
			, is_directory(::llcpp::boolean::BOOLEAN_UNKNOWN)
			, name_bytes()
		{}
		constexpr ~Job() noexcept {}

		constexpr Job(const Job& other) noexcept = delete;
		constexpr Job& operator=(const Job& other) noexcept = delete;
		constexpr Job(Job&& other) noexcept = delete;
		constexpr Job& operator=(Job&& other) noexcept = delete;

		constexpr Job(volatile const Job& other) noexcept = delete;
		constexpr Job& operator=(volatile const Job& other) noexcept = delete;
		constexpr Job(volatile Job&& other) noexcept = delete;
		constexpr Job& operator=(volatile Job&& other) noexcept = delete;

		constexpr void setIsDirectory(const ::llcpp::ll_bool_t is_directory) noexcept { this->is_directory = is_directory; }

		constexpr void fill(const StringBuffer& filename) noexcept {
			this->name_bytes = filename.getFilled();
			::llcpp::meta::utils::hash::LlanyHash<::llcpp::char_type>().memcopy(this->file_path, filename.data(), filename.getFilled() * sizeof(::llcpp::char_type));
		}
		constexpr void fill(const StringBuffer& filename, const ::llcpp::ll_bool_t is_directory) noexcept {
			this->fill(filename);
			this->setIsDirectory(is_directory);
		}

		// Process this file
		constexpr void process() noexcept {
			switch(this->is_directory.as_enum()) {
				case ::llcpp::Boolean::enum_bool::True: {
					StringBuffer buff(this->file_path, __LL_MAX_PATH, static_cast<::llcpp::u64>(this->name_bytes));
					// If is NOT a directory, we should process it with iterateOverDirectory
					iterateOverDirectory(buff);
				}
					break;
				case ::llcpp::Boolean::enum_bool::False: {
					// Hash and process file
				}
					break;
				case ::llcpp::Boolean::enum_bool::Unknown: {

				}
					break;
				// Log error bool type
				case ::llcpp::Boolean::enum_bool::Invalid:
				default:
					ll_print(__LL_L "Error enum of file '");
					ll_print(this->file_path, this->file_path + this->name_bytes);
					ll_print(__LL_L "' :: ");
					ll_print(static_cast<::llcpp::u8>(this->is_directory.as_enum()));
					ll_print(__LL_L "\n");
					break;
			}
		}
};

constinit ::llcpp::ll_bool_t KEEP_ALIVE												= ::llcpp::LL_TRUE;
constinit ::llcpp::usize NUMBER_OF_DIRECTORIES_PROCESSED							= 0;
constinit ::llcpp::usize NUMBER_OF_FILES_FOUND										= 0;
constinit ::llcpp::usize NUMBER_OF_FILES_PROCESSED									= 0;
constinit ::llcpp::meta::utils::atomic::AtomitLIFO<Job*, ::llcpp::i16> LIFO			= {};
constinit ::llcpp::meta::utils::atomic::AtomitLIFO<Job*, ::llcpp::i16> LIFO_FREE	= {};
constinit ::llcpp::meta::utils::atomic::AtomitLIFO<Job*, ::llcpp::i16> LIFO_DIR		= {};
constinit Job jobs[decltype(LIFO)::NUMBER_OF_OBJECTS]								= {};
// By default this program supports
::llcpp::u8 th[__LL_MAX_THREAD * sizeof(::std::thread)]								= {};

void iterateOverDirectory(StringBuffer& directory) noexcept {
	DirType* dir = ll_opendir(directory.data());
	if(!dir) {
		ll_print(__LL_L "Error opening directory '");
		ll_print(directory.data(), directory.getLast());
		ll_print(__LL_L "'\n");
		return;
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

#if defined(__LL_POSIX_SYSTEM) || defined(__LL_UNIX_SYSTEM)
		// If entry has file type data, we continue normally
		if(entry->d_type != DT_UNKNOWN) {
			if (!directory.concatOrContinue(entry->d_name, entry->d_namlen)) {
				ll_print(__LL_L "Error to concatenate filename: '");
				ll_print(entry->d_name);
				ll_print(__LL_L "' with path: '");
				ll_print(directory.data(), directory.getLast());
				ll_print(__LL_L "'\n");
				continue;
			}
			// If file is concatenated, we check its type
			else {
				// Get job pointer to add to list
				Job* job = ::llcpp::NULL_VALUE<Job*>;
				while (!LIFO.pop(job)) continue;
				job->fill(directory);
				job->setIsDirectory(ll_is_directory(entry->d_type))
				directory.resetPreviousFileName(entry->d_namlen);
			}
		}
		else

#endif // __LL_POSIX_SYSTEM || __LL_UNIX_SYSTEM
		// If buffer could not be concatenated
		if (!directory.concatOrContinue(entry->d_name, entry->d_namlen)) {
			ll_print(__LL_L "Error to concatenate filename: '");
			ll_print(entry->d_name);
			ll_print(__LL_L "' with path: '");
			ll_print(directory.data(), directory.getLast());
			ll_print(__LL_L "'\n");
			continue;
		}
		// If file is concatenated, we check its type
		else {
			// Get job pointer to add to list
			Job* job = ::llcpp::NULL_VALUE<Job*>;
			// If there is no free jobs
			if(!LIFO_FREE.pop(job)) {
				// Get file stat, while waiting to get a free job
				auto is_dir = isDirectory(directory.data());
				// Now we cant do much more, so we wait to get a free job
				while(!LIFO_FREE.pop(job))
					::std::this_thread::sleep_for(SLEEP_TIME);
				// Now we have a job, an we will set its params
				job->fill(directory, is_dir);

				// Insert job in its specific place
				if(is_dir) while (!LIFO_DIR.push(job))
						::std::this_thread::sleep_for(SLEEP_TIME);
				else while (!LIFO.push(job))
						::std::this_thread::sleep_for(SLEEP_TIME);
			}
			else {
				// Now we have a job, an we will set its params
				job->fill(directory);

				// Insert job to be processed
				while (!LIFO.push(job))
					::std::this_thread::sleep_for(SLEEP_TIME);
			}

			// Reset buffer of directory
			directory.resetPreviousFileName(entry->d_namlen);
		}
	}
}

void main_dispatcher(::llcpp::string root) noexcept {

}
void main_consumer() noexcept {
	// Keep working until jobs are finished, and there is no more jobs
	while (KEEP_ALIVE) {
		Job* job = ::llcpp::NULL_VALUE<Job*>;
		// Try to get a job, and sleep if couldnt
		while (!LIFO.pop(job))
			::std::this_thread::sleep_for(SLEEP_TIME);

		job->process();
	}
}
int main(int argc, const char** argv) {
	// Fill LIFO with avaibles jobs
	for(auto& i : jobs) (void)LIFO_FREE.push(&i);

	// Run threads
	::llcpp::usize num_threads = 2;
	::std::thread* begin = reinterpret_cast<::std::thread*>(th);
	const ::std::thread* end = begin + num_threads;
	for(::std::thread* current = begin; current < end; ++current)
		(new (current) ::std::thread(main_consumer))->detach();

	// Start dispatcher
	main_dispatcher(reinterpret_cast<::llcpp::string*>(argv)[1]);
	return 0;
}
