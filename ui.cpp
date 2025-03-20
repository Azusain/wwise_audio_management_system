#include <windows.h>
#include <commdlg.h>
#include <iostream>
#include <vector>

// TODO: the max buffer size should be considered carefully.
// format of the buffer:
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
    // only contains a single file.
    if (*p == '\0') {
      selected_file_paths.emplace_back(std::string((char*)szFile.data(), first_file_or_dir_size_wchar * sizeof(wchar_t)));
    }
    // multiple files, buffer starts with the directory's path.
    else {
      std::string dir((char*)szFile.data(), first_file_or_dir_size_wchar * sizeof(wchar_t));
      while (*p) {
        size_t len_path = wcslen(p) * sizeof(wchar_t);
        selected_file_paths.emplace_back(dir + "\\" + std::string((char*)p, len_path));
        p += wcslen(p) + 1;
      }
    }
  }
  else {
    std::cerr << "failed to get opened file name\n";
    return false;
  }
  return true;
}