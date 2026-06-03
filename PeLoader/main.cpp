#include <iostream>
#include <Windows.h>
#include <filesystem>
#include <fstream>
#include "PeParser.h"
#include "Pe.h"
int main()
{
	std::filesystem::path pePath{ "../x64/Debug/HelloWorld.exe" };
	std::ifstream file(pePath , std::ios::binary);
	std::vector<char> fileBytes((std::istreambuf_iterator<char>(file)),std::istreambuf_iterator<char>());



	PeParser peParser(fileBytes);
	Pe pe(peParser);
}