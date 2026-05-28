#pragma once
#include <windows.h>
#include <string>
#include <vector>

bool Lab3_EditTextDialog(HWND owner, const char *windowTitle, std::string &textInOut);

bool Lab3_ParseInts(const std::string &text, std::vector<int> &out);
