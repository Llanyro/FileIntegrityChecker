
#include <llanylib/utils_base/ArrayBase.hpp>
#include <llanylib/utils_base/hash/LlanyHash.hpp>

#include <windows.h>
#include <tchar.h>
#include <cstdio>

#include <cwchar>

#include <list>
#include <vector>

#include <io.h>
#include <stdio.h>
#include <string.h>

#include <sys/stat.h>

#undef TRUE

using String = ::llcpp::meta::utils::wStr<::llcpp::TRUE>;

struct StringBuffer {
    String path;
    String::iterator last;
};

struct FileData {
    ::llcpp::u64 path_hash;
    ::llcpp::u64 file_hash;
};

using HashVector = ::std::vector<FileData>;
struct HashBuffer {
    static constexpr ::std::size_t VECTOR_SIZE = 500;
    ::std::list<HashVector> buffer;
    HashVector* vector;
    FileData* last;
};

/*// Function to process each found file
void ProcessFile(const TCHAR* filePath) {
    _tprintf(_T("Found file: %s\n"), filePath);
    // Add your file processing logic here
}

// Recursive function to iterate through directories
static void IterateFilesRecursive(const TCHAR* directory) noexcept {
    TCHAR searchPath[MAX_PATH];
    WIN32_FIND_DATA findFileData;
    HANDLE hFind = INVALID_HANDLE_VALUE;

    // Prepare the search path (e.g., "C:\path\*")
    _stprintf_s(searchPath, MAX_PATH, _T("%s\\*"), directory);

    // Find the first file in the directory
    hFind = FindFirstFile(searchPath, &findFileData);

    if (hFind == INVALID_HANDLE_VALUE) {
        _tprintf(_T("FindFirstFile failed for %s (Error: %d)\n"), directory, GetLastError());
        return;
    }

    do {
        // Skip "." and ".." directories
        if (_tcscmp(findFileData.cFileName, _T(".")) == 0 ||
            _tcscmp(findFileData.cFileName, _T("..")) == 0) {
            continue;
        }

        // Build the full path
        TCHAR fullPath[MAX_PATH];
        _stprintf_s(fullPath, MAX_PATH, _T("%s\\%s"), directory, findFileData.cFileName);

        if (findFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
            // It's a directory, recurse into it
            IterateFilesRecursive(fullPath);
        }
        else {
            // It's a file, process it
            ProcessFile(fullPath);
        }
    } while (FindNextFile(hFind, &findFileData) != 0);

    FindClose(hFind);
}

static void iterateFileRecursive(StringBuffer& path, HashBuffer& db) noexcept {
    WIN32_FIND_DATA findFileData{};
    HANDLE hFind = FindFirstFile(path.path.data(), &findFileData);

    if (hFind == INVALID_HANDLE_VALUE) {
        _tprintf(_T("FindFirstFile failed for %.*s (Error: %d)\n"), path.path.begin(), GetLastError());
        return;
    }

    do {
        // Skip "." and ".." directories
        if (_tcscmp(findFileData.cFileName, _T(".")) == 0 ||
            _tcscmp(findFileData.cFileName, _T("..")) == 0) {
            continue;
        }

        // Build the full path
        TCHAR fullPath[MAX_PATH];
        _stprintf_s(fullPath, MAX_PATH, _T("%s\\%s"), directory, findFileData.cFileName);

        if (findFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
            // It's a directory, recurse into it
            IterateFilesRecursive(fullPath);
        }
        else {
            // It's a file, process it
            ProcessFile(fullPath);
        }
    } while (FindNextFile(hFind, &findFileData) != 0);
}*/

#include <windows.h>
#include <shlobj.h>
#include <stdio.h>

void IterateShellFolder(IShellFolder* folder, LPITEMIDLIST pidl) {
    IEnumIDList* enumIdList;
    if (SUCCEEDED(folder->EnumObjects(NULL, SHCONTF_FOLDERS | SHCONTF_NONFOLDERS, &enumIdList))) {
        LPITEMIDLIST itemPidl;
        while (enumIdList->Next(1, &itemPidl, NULL) == S_OK) {
            STRRET strret;
            if (SUCCEEDED(folder->GetDisplayNameOf(itemPidl, SHGDN_FORPARSING, &strret))) {
                WCHAR name[MAX_PATH];
                StrRetToBuf(&strret, itemPidl, name, MAX_PATH);
                wprintf(L"Found: %s\n", name);
            }
            CoTaskMemFree(itemPidl);
        }
        enumIdList->Release();
    }
}


void ProcessFile(const char* filename) {
    printf("Found: %s\n", filename);
}

void IterateFiles(const char* directory) {
    char searchPath[1024];
    struct _finddata_t fileinfo;
    intptr_t handle;

    snprintf(searchPath, sizeof(searchPath), "%s\\*", directory);

    if ((handle = _findfirst(searchPath, &fileinfo)) == -1L) {
        return;
    }

    do {
        if (strcmp(fileinfo.name, ".") == 0 || strcmp(fileinfo.name, "..") == 0)
            continue;

        char fullPath[1024];
        snprintf(fullPath, sizeof(fullPath), "%s\\%s", directory, fileinfo.name);

        if (fileinfo.attrib & _A_SUBDIR) {
            IterateFiles(fullPath);
        }
        else {
            ProcessFile(fullPath);
        }
    } while (_findnext(handle, &fileinfo) == 0);

    _findclose(handle);
}


int _tmain(int argc, ::llcpp::ll_wchar_t** argv) {
    if (argc != 2) {
        _tprintf(_T("Usage: %s <directory>\n"), argv[0]);
        return 1;
    }

    // Initial path to use
    ::llcpp::ll_wchar_t* initial_path = argv[1];
    size_t path_size = ::std::wcslen(initial_path);

    // Buffer of path accumulated
    ::llcpp::ll_wchar_t __path_buffer[MAX_PATH]{};
    StringBuffer path = { { __path_buffer, ::llcpp::meta::traits::array_size<decltype(__path_buffer)> },  __path_buffer };

    // Copy initial buffer into buffer
    ::std::wmemcpy(path.path.begin(), initial_path, path_size);
    path.last += path_size;

    // Generate the database we are going to fill
    HashBuffer db{};
    db.vector = &db.buffer.emplace_back(HashBuffer::VECTOR_SIZE);
    db.last = db.vector->data();
    
    // Start processing files
    iterateFileRecursive(path, db);

    return 0;
}


