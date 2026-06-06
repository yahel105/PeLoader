#include <Windows.h>
#include <filesystem>
#include <fstream>
#include <span>
#include "PeParser.h"
#include "LoadedImage.h"

int main()
{
	std::filesystem::path pePath{ "../x64/Debug/HelloWorld.exe" };
	std::ifstream file(pePath , std::ios::binary);
	std::vector<char> fileBytes((std::istreambuf_iterator<char>(file)),std::istreambuf_iterator<char>());

	PeParser peParser(fileBytes);
	LoadedImage pe(peParser);
}