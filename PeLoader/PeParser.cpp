#include <Windows.h>
#include <span>
#include "PeParser.h"
#include "Utility.h"

PeParser::PeParser(std::span<const char> peFile) :
    m_pBase(const_cast<char*>(peFile.data())), 
    m_fileSize{peFile.size()}{}

PVOID PeParser::getBaseAddress() const
{
    return m_pBase;
}

PIMAGE_DOS_HEADER PeParser::getDosHeader() const 
{

    return resolve_rva<PIMAGE_DOS_HEADER>(m_pBase,0);
}

PIMAGE_NT_HEADERS PeParser::getNtHeader() const
{
    return resolve_rva<PIMAGE_NT_HEADERS>(m_pBase, getDosHeader()->e_lfanew);
}

PIMAGE_SECTION_HEADER PeParser::getSectionHeader() const
{
    return IMAGE_FIRST_SECTION(getNtHeader());
}

PIMAGE_DATA_DIRECTORY PeParser::getDataDir() const
{
    return getNtHeader()->OptionalHeader.DataDirectory;
}
