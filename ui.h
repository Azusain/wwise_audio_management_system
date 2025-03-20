#pragma once
#include <string>
#include <vector>

bool OpenFileDialogMultiSelect(std::vector<std::string>& selected_file_paths, int max_buffer_size = 32768);
