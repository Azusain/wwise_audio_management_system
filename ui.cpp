#include <windows.h>
#include <commdlg.h>
#include <iostream>
#include <vector>
#include <string>

// TODO: the max buffer size should be considered carefully.
// Format of the buffer:
// +-------------------------------+-------------+-----------------+
// | directory_path or file_path_1 | file_path_2 | file_path_n ... |
// +-------------------------------+-------------+-----------------+
bool OpenFileDialogMultiSelect(std::vector<std::string>& selected_file_paths, int max_buffer_size) {
  std::vector<wchar_t> szFile(max_buffer_size, 0);
  OPENFILENAME ofn = { sizeof(ofn) };
  ofn.lpstrFile = szFile.data();
  ofn.nMaxFile = max_buffer_size;
  ofn.lpstrFilter = L"All Files\0*.*\0";
  ofn.Flags = OFN_EXPLORER | OFN_ALLOWMULTISELECT | OFN_FILEMUSTEXIST;

  if (GetOpenFileNameW(&ofn)) {
    size_t first_file_or_dir_size_wchar = wcslen(szFile.data());
    wchar_t* p = szFile.data() + first_file_or_dir_size_wchar + 1;
    // Only contains a single file.
    if (*p == '\0') {
      // Convert wchar_t path to UTF-8 string
      int len = WideCharToMultiByte(CP_UTF8, 0, szFile.data(), first_file_or_dir_size_wchar, nullptr, 0, nullptr, nullptr);
      std::string file_path(len, 0);
      WideCharToMultiByte(CP_UTF8, 0, szFile.data(), first_file_or_dir_size_wchar, &file_path[0], len, nullptr, nullptr);
      selected_file_paths.emplace_back(file_path);
    }
    // Multiple files, buffer starts with the directory's path.
    else {
      // Convert wchar_t directory path to UTF-8 string
      int len = WideCharToMultiByte(CP_UTF8, 0, szFile.data(), first_file_or_dir_size_wchar, nullptr, 0, nullptr, nullptr);
      std::string dir(len, 0);
      WideCharToMultiByte(CP_UTF8, 0, szFile.data(), first_file_or_dir_size_wchar, &dir[0], len, nullptr, nullptr);
      while (*p) {
        size_t len_path = wcslen(p);
        int path_len = WideCharToMultiByte(CP_UTF8, 0, p, len_path, nullptr, 0, nullptr, nullptr);
        std::string file_path(path_len, 0);
        WideCharToMultiByte(CP_UTF8, 0, p, len_path, &file_path[0], path_len, nullptr, nullptr);
        selected_file_paths.emplace_back(dir + "\\" + file_path);
        // Move to next file path
        p += len_path + 1; 
      }
    }
  }
  else {
    std::cerr << "failed to get opened file name\n";
    return false;
  }
  return true;
}