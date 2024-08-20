#include <iostream>
#include <unordered_map>

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <stdio.h>
#include <wchar.h>

std::wstring get_extension(const std::wstring& path) {
    std::wstring ext = {};
    static constexpr int ext_max_size = 10;
    ext.resize(ext_max_size);
    int i = (int)(path.size() - 1);
    while (i >= 0 && i >= ((int)path.size()) - ext_max_size && path[i] != L'.') {
        i--;
    }
    if (path[i] == L'.') {
        ext = path.substr(i, path.size() - i);
    }
    else {
        ext = L".unk";
    }
    return ext;
}

std::pair<std::wstring, std::wstring> get_output_location(const std::wstring& fullpath, const FILETIME& sometime) {
    SYSTEMTIME system_time;
    FileTimeToSystemTime(&sometime, &system_time);
    wchar_t dirname[32] = {};
    wchar_t filename[96] = {};
    const std::size_t namehash = std::hash<std::wstring>{}(fullpath);
    _snwprintf_s(dirname, 32, L"%02d-%02d-%02d", (int)system_time.wYear, (int)system_time.wMonth, (int)system_time.wDay);
    _snwprintf_s(filename, 96, L"%02d-%02d-%02d-%02d-%02d-%02d-%03d-%zu\0", (int)system_time.wYear, (int)system_time.wMonth, (int)system_time.wDay,
        (int)system_time.wHour, (int)system_time.wMinute, (int)system_time.wSecond,
        (int)system_time.wMilliseconds, namehash);
    return std::pair<std::wstring, std::wstring>(std::wstring(dirname), std::wstring(filename));
}

bool is_dir_exists(const std::wstring& path) {
    DWORD filetype = GetFileAttributesW(path.c_str());
    if (filetype == INVALID_FILE_ATTRIBUTES) {
        return false;
    }

    if (filetype & FILE_ATTRIBUTE_DIRECTORY) {
        return true;
    }

    return false;
}

struct SORTINFO {
    std::wstring input_dir_wstr;
    std::wstring output_dir_wstr;
    bool rename;
};

void sort_files(const SORTINFO& sortinfo) {
    std::wstring dir_path = {};
    dir_path = sortinfo.input_dir_wstr + L"\\*";
    WIN32_FIND_DATAW find_data;
    HANDLE hfind = FindFirstFileW(dir_path.c_str(), &find_data);
    if (hfind == INVALID_HANDLE_VALUE) {
        printf("FindFirstFile failed (error %lu)\n", GetLastError());
        return;
    }

    do {
        if (wcscmp(find_data.cFileName, L".") != 0 &&
            wcscmp(find_data.cFileName, L"..") != 0) {
            std::wstring old_file_relname(find_data.cFileName);
            std::wstring old_full_path = sortinfo.input_dir_wstr + L"\\" + old_file_relname;

            if (find_data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
                SORTINFO newsortinfo = {};
                newsortinfo.input_dir_wstr = old_full_path;
                newsortinfo.output_dir_wstr = sortinfo.output_dir_wstr;
                newsortinfo.rename = sortinfo.rename;
                sort_files(newsortinfo);
            }
            else {
                HANDLE hfile = CreateFileW(old_full_path.c_str(), GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
                FILETIME creation_time = {};
                FILETIME last_access_time = {};
                FILETIME last_write_time = {};
                if (GetFileTime(hfile, &creation_time, &last_access_time, &last_write_time)) {
                    std::pair<std::wstring, std::wstring> output = get_output_location(old_full_path, last_write_time);
                    std::wstring new_dir = sortinfo.output_dir_wstr + L"\\" + output.first;
                    std::wstring new_filepath = new_dir + L"\\";
                    if (sortinfo.rename) {
                        new_filepath += output.second + get_extension(old_full_path);
                    }
                    else {
                        new_filepath += old_file_relname;
                    }
                    
                    wprintf(L"\"%s\" would be copied to \"%s\"\n", old_full_path.c_str(), new_filepath.c_str());
                    if (!is_dir_exists(new_dir)) {
                        CreateDirectoryW(new_dir.c_str(), NULL);
                    }
                    CopyFileW(old_full_path.c_str(), new_filepath.c_str(), FALSE);
                } 
                else {
                    wprintf(L"failed to receive time info for file \"%s\"", old_full_path.c_str());
                }
            }
        }
    } while (FindNextFileW(hfind, &find_data) != 0);

    FindClose(hfind);
}

int wmain(int argc, wchar_t** argv)
{
    if (argc >= 3) {
        SORTINFO sortinfo = {};
        sortinfo.input_dir_wstr = std::wstring(argv[1]);
        sortinfo.output_dir_wstr = std::wstring(argv[2]);
        if (argc >= 4) {
            std::wstring rename_arg(argv[3]);
            static const std::wstring rename_str(L"rename");
            sortinfo.rename = (rename_arg == rename_str);
        }
        else {
            sortinfo.rename = false;
        }
        sort_files(sortinfo);
    }
    else {
        printf("usage: sort_by_date.exe <input_dir> <output_dir> <rename>\n");
        printf("// it copies files, input and output must be different directories\n");
        printf("// last argument can be \"rename\" if needed to make names of copies from the template YYYY-MM-DD-hh-mm-mss-namehash\n");
    }
}
