#include <Windows.h>
#include "Pe.h"
#include "Utility.h"

Pe::Pe(const PeParser& peParser) :
	m_peParser{peParser},
	m_peBase{VirtualAlloc(reinterpret_cast<LPVOID>(peParser.getNtHeader()->OptionalHeader.ImageBase) ,peParser.getNtHeader()->OptionalHeader.SizeOfImage, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE)}
{
	if (!m_peBase)
	{
		m_peBase = VirtualAlloc(nullptr, m_peParser.getNtHeader()->OptionalHeader.SizeOfImage, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
	}
	if (!m_peBase)
	{
		throw std::bad_alloc();
	}
	mapSections();

}
void Pe::mapSections()
{
	size_t numOfSections = m_peParser.getNtHeader()->FileHeader.NumberOfSections;
	PIMAGE_SECTION_HEADER pSectionHeader = m_peParser.getSectionHeader();

	for ( size_t i = 0; i < numOfSections; i++ )
	{
		size_t sectionSize = static_cast<size_t>( pSectionHeader[i].SizeOfRawData );
		PVOID pInMemorySection = resolve_rva<PVOID>( m_peBase, pSectionHeader[i].VirtualAddress );
		PVOID pPeRawData = resolve_rva<PVOID>( m_peParser.getDosHeader(), pSectionHeader[i].PointerToRawData );

		std::memcpy( pInMemorySection, pPeRawData, sectionSize );
	}
}


void Pe::reloc()
{
	//size_t sizeOfReloc = ;
};
