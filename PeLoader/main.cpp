#include <Windows.h>
#include <iostream>
#include <filesystem>
#include <fstream>
#include <span>
#include "PeParser.h"
#include "LoadedImage.h"
#include "PeLoader.h"
int main()
{
    std::filesystem::path pePath{ "C:/HelloWorldCRT.exe" };
    std::vector<char> fileBytes;

    {
        std::ifstream file(pePath, std::ios::binary);
        if (!file.is_open()) {
            std::cerr << "Failed to open PE file.\n";
            return 1;
        }

        fileBytes.assign(std::istreambuf_iterator<char>(file), std::istreambuf_iterator<char>());
    }

    if (fileBytes.empty()) {
        std::cerr << "PE file is empty.\n";
        return 1;
    }


    PeLoader PE(std::move(fileBytes));
}