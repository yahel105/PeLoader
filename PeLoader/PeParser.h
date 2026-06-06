#pragma once
#include <Windows.h>
#include <span>

class PeParser
{
public:
	PeParser(std::span<const char> peFile);
	PVOID getBaseAddress() const;
	PIMAGE_DOS_HEADER getDosHeader() const;
	PIMAGE_NT_HEADERS getNtHeader() const;
	PIMAGE_SECTION_HEADER getSectionHeader() const;
	PIMAGE_DATA_DIRECTORY getDataDir() const;
private:
	PVOID m_pBase;
	size_t m_fileSize;
};

