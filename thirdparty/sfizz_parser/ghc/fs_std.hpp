//
// TAQS.IM JUCEKit
// Created by lucianoiam on 23/03/23.
//

#ifndef FS_STD_HPP
#define FS_STD_HPP

// Fake fs_std.hpp avoids dependency ghc

#include <filesystem>

namespace fs {
    using namespace std::filesystem;
    using ifstream = std::ifstream;
    using ofstream = std::ofstream;
    using fstream = std::fstream;
}

#endif
