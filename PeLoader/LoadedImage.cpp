#include <Windows.h>
#include <iostream>
#include "LoadedImage.h"
#include "Utility.h"

LoadedImage::LoadedImage(const PeParser& peParser) :
	m_peParser{ peParser },
	m_peBase{ VirtualAlloc(reinterpret_cast<LPVOID>( peParser.getNtHeader()->OptionalHeader.ImageBase ),peParser.getNtHeader()->OptionalHeader.SizeOfImage, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE)}
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

	if (!registerExeptionHandlers())
	{
		throw std::exception("[!]registerExeptionHandlers() failed!");
	}

	if (!resolveImports())
	{
		throw std::exception("[!]resolveImports() failed!");
	}

	reloc();

	if (!sectionsProtect())
	{
		throw std::exception("[!]sectionsProtect() failed!");
	}
	//////////
	execute();
}
void LoadedImage::mapSections()
{
	size_t numOfSections = m_peParser.getNtHeader()->FileHeader.NumberOfSections;
	PIMAGE_SECTION_HEADER pSectionHeader = m_peParser.getSectionHeader();

	DWORD sectionSize;
	PVOID pInMemorySection;
	PVOID pPeRawData;

	for ( size_t i = 0; i < numOfSections; i++ )
	{
		pInMemorySection = resolve_rva<PVOID>( m_peBase, pSectionHeader[i].VirtualAddress );
		pPeRawData = resolve_rva<PVOID>( m_peParser.getBaseAddress(), pSectionHeader[i].PointerToRawData );
		sectionSize =  pSectionHeader[i].SizeOfRawData ;

		std::memcpy( pInMemorySection, pPeRawData, sectionSize );
	}
}

void LoadedImage::reloc()
{
	typedef struct _BASE_RELOCATION_ENTRY {
		WORD Offset : 12;
		WORD Type : 4;
	} BASE_RELOCATION_ENTRY, *PBASE_RELOCATION_ENTRY; //credit to https://hasherezade.github.io/libpeconv/structpeconv_1_1___b_a_s_e___r_e_l_o_c_a_t_i_o_n___e_n_t_r_y.html

	PIMAGE_DATA_DIRECTORY relocDataDir = &m_peParser.getDataDir()[IMAGE_DIRECTORY_ENTRY_BASERELOC];
	auto pRelocSection = resolve_rva<PIMAGE_BASE_RELOCATION>(m_peBase, relocDataDir->VirtualAddress);
	size_t delta = reinterpret_cast<ULONGLONG>(m_peBase) - m_peParser.getNtHeader()->OptionalHeader.ImageBase;
	std::cout << "[*] Reloc delta: 0x" << std::hex << delta << "\n";
	PBASE_RELOCATION_ENTRY relocEntry{nullptr};

	if (!relocDataDir->VirtualAddress) return;

	while (pRelocSection->VirtualAddress)
	{
		relocEntry = reinterpret_cast<PBASE_RELOCATION_ENTRY>(pRelocSection + 1);
		while (reinterpret_cast<PBYTE>(relocEntry) != (reinterpret_cast<PBYTE>(pRelocSection) + pRelocSection->SizeOfBlock))
		{
			switch (relocEntry->Type){
			case IMAGE_REL_BASED_ABSOLUTE:
				break;

			case IMAGE_REL_BASED_DIR64:
			{
				auto toReloc = resolve_rva<ULONG_PTR*>(m_peBase, static_cast<size_t>(pRelocSection->VirtualAddress) + relocEntry->Offset);
				*toReloc += delta;
			} break;
			case IMAGE_REL_BASED_HIGHLOW:
			{
				auto toReloc = resolve_rva<PDWORD>(m_peBase, static_cast<size_t>(pRelocSection->VirtualAddress) + relocEntry->Offset);
				*toReloc += static_cast<DWORD>(delta);
			}break;

			case IMAGE_REL_BASED_HIGH:
			{
				auto toReloc = resolve_rva<PWORD>(m_peBase, static_cast<size_t>(pRelocSection->VirtualAddress) + relocEntry->Offset);
				*toReloc += HIWORD(delta);
			}break;

			case IMAGE_REL_BASED_LOW:
			{
				auto toReloc = resolve_rva<PWORD>(m_peBase, static_cast<size_t>(pRelocSection->VirtualAddress) + relocEntry->Offset);
				*toReloc += LOWORD(delta);
			}break;

			default:
				std::cout << "[!]Bad Relocation type\n";
				break;
			}
			relocEntry++;
		}
		pRelocSection = reinterpret_cast<PIMAGE_BASE_RELOCATION>(relocEntry);
	}


}
boolean LoadedImage::resolveImports()
{
	PIMAGE_DATA_DIRECTORY importData{ &m_peParser.getDataDir()[IMAGE_DIRECTORY_ENTRY_IMPORT] };
	auto importDirTable{ resolve_rva<PIMAGE_IMPORT_DESCRIPTOR>(m_peBase ,importData->VirtualAddress) }; 
 	std::string_view dllName;
	ULONG_PTR pFuncAddr{};

	while (importDirTable->OriginalFirstThunk)
	{
		PIMAGE_THUNK_DATA importLookupTable{ resolve_rva<PIMAGE_THUNK_DATA>(m_peBase , importDirTable->OriginalFirstThunk) };
		PIMAGE_THUNK_DATA importAddressTable{ resolve_rva<PIMAGE_THUNK_DATA>(m_peBase , importDirTable->FirstThunk) };
		dllName = { resolve_rva<LPSTR>(m_peBase , importDirTable->Name) };
		
		ModuleHandle hDll{ LoadLibraryA(dllName.data()) };
		if (!hDll.get())
		{
			std::cerr << "[!]Couldnt Load DLL using LoadLibraryA\n";
			return false;
		}
		while (importLookupTable->u1.AddressOfData)
		{
			if (IMAGE_SNAP_BY_ORDINAL(importLookupTable->u1.Ordinal))
			{
				pFuncAddr = reinterpret_cast<ULONG_PTR>( GetProcAddress(hDll.get(), MAKEINTRESOURCEA( IMAGE_ORDINAL(importLookupTable->u1.Ordinal ))));
			}
			else {
				auto pImportFuncByName{ resolve_rva<PIMAGE_IMPORT_BY_NAME>(m_peBase,importLookupTable->u1.AddressOfData) };
				pFuncAddr = reinterpret_cast<ULONG_PTR>(GetProcAddress(hDll.get(), pImportFuncByName->Name));
			}
			if (!pFuncAddr)
			{
				std::cerr << "couldnt resolve function";
				return false;
			}
			importAddressTable->u1.Function = pFuncAddr;
			pFuncAddr = NULL;
			++importLookupTable;
			++importAddressTable;

		}

		m_importedModules.push_back(std::move(hDll));
		++importDirTable;
	}
	return true;
}
boolean LoadedImage::registerExeptionHandlers()
{
	auto exceptionDataDir{ m_peParser.getDataDir()[IMAGE_DIRECTORY_ENTRY_EXCEPTION] };
	auto exeptionDirEntry{ resolve_rva<PIMAGE_RUNTIME_FUNCTION_ENTRY>(m_peBase, exceptionDataDir.VirtualAddress) };

	if (!RtlAddFunctionTable(exeptionDirEntry, (exceptionDataDir.Size / sizeof(IMAGE_RUNTIME_FUNCTION_ENTRY)), reinterpret_cast<DWORD64> (m_peBase)))
	{
		return false;
	}
	return true;
};
boolean LoadedImage::sectionsProtect()
{
	size_t numOfSections = m_peParser.getNtHeader()->FileHeader.NumberOfSections;
	PIMAGE_SECTION_HEADER pSectionHeader = m_peParser.getSectionHeader();

	DWORD sectionSize;
	PVOID pInMemorySection;
	DWORD characteristics;

	for (size_t i = 0; i < numOfSections; i++)
	{
		pInMemorySection = resolve_rva<PVOID>(m_peBase, pSectionHeader[i].VirtualAddress);
		sectionSize = pSectionHeader[i].SizeOfRawData;
		characteristics = pSectionHeader[i].Characteristics;
		DWORD protection{ PAGE_NOACCESS };
		DWORD oldProtection{PAGE_NOACCESS};


		if (characteristics & IMAGE_SCN_MEM_EXECUTE)
		{
			if (characteristics & IMAGE_SCN_MEM_READ && characteristics & IMAGE_SCN_MEM_WRITE)
				protection = PAGE_EXECUTE_READWRITE;
			else if (characteristics & IMAGE_SCN_MEM_READ)
				protection = PAGE_EXECUTE_READ;
			else
				protection = PAGE_EXECUTE;
		}
		else if (characteristics & IMAGE_SCN_MEM_WRITE)
		{
			protection = PAGE_READWRITE;
		}
		else if (characteristics & IMAGE_SCN_MEM_READ)
		{
			protection = PAGE_READONLY;
		}

		if (oldProtection && sectionSize > 0)
			if (!VirtualProtect(pInMemorySection, sectionSize, protection, &oldProtection))
			{
				return false;
			}
	}
	return true;
}
void LoadedImage::execute()
{
	PVOID entryPointAddress{ resolve_rva<PVOID>(m_peBase ,m_peParser.getNtHeader()->OptionalHeader.AddressOfEntryPoint) };


	std::cout << "[*] TLSDataDir Size: " << m_peParser.getDataDir()[IMAGE_DIRECTORY_ENTRY_TLS].Size << "\n";
	std::cout << "[*] m_peBase: 0x" << std::hex << m_peBase << "\n";
	std::cout << "[*] AddressOfEntryPoint RVA: 0x" << m_peParser.getNtHeader()->OptionalHeader.AddressOfEntryPoint << "\n";
	std::cout << "[*] Entry point address: 0x" << entryPointAddress << "\n";
	std::cout << "[*] ImageBase: 0x" << m_peParser.getNtHeader()->OptionalHeader.ImageBase << "\n";
	std::cout << "[*] Delta: 0x" << (reinterpret_cast<ULONGLONG>(m_peBase) - m_peParser.getNtHeader()->OptionalHeader.ImageBase) << "\n";


	using entryPoint = DWORD(WINAPI*)();
	entryPoint run = reinterpret_cast<entryPoint>(entryPointAddress);
	run();
};
