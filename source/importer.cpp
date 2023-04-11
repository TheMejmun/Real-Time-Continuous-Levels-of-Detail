//
// Created by Sam on 2023-04-07.
//

#include <fstream>
#include "importer.h"
#include "printer.h"

std::vector<char> Importer::readFile(const std::string &filename) {
    std::ifstream file(filename, std::ios::ate | std::ios::binary);

    if (!file.is_open()) {
        THROW("Failed to open file " + filename);
    }

    auto fileSize = file.tellg();
    std::vector<char> buffer(fileSize);

    file.seekg(0);
    file.read(buffer.data(), fileSize);

    file.close();
    return buffer;
}