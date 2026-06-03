#pragma once
#include <Windows.h>
#include <iostream>
#include <vector>

class PeParser
{
public:
	PeParser(const std::vector<char>& peFile);
	PVOID getBaseAddress() const;
	PIMAGE_DOS_HEADER getDosHeader() const;
	PIMAGE_NT_HEADERS getNtHeader() const;
	PIMAGE_SECTION_HEADER getSectionHeader() const;
	PIMAGE_DATA_DIRECTORY getDataDir() const;
private:
	PVOID m_pBase;
	size_t m_fileSize;
};

