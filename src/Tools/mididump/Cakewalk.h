
/** $VER: Cakewalk.cpp (2025.07.26) P. Stuer - Reads an Instrument Definition file **/

#pragma once

#include <filesystem>

namespace fs = std::filesystem;

void ReadIns(const fs::path & filePath);
