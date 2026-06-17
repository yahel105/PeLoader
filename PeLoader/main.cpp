#include <Windows.h>
#include <iostream>
#include <filesystem>
#include <fstream>
#include "PeLoader.h"
#include "PeLoaderError.h"
#include "Logger.h"
#include "Utility.h"


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
    
    Logger& logger = Logger::instance();
    logger.setLogToFile();

    try {
       PeLoader PE(std::move(fileBytes));
    }
    catch (PeLoaderError& error)
    {
        logger.log(error.m_logLevel, error.what());
        return 1;
    }
}