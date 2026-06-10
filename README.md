# PeLoader

A manual PE loader for Windows x64. Reads a PE executable from disk, maps it into memory, and runs it — replicating the core steps the Windows loader performs, without using `CreateProcess` or the OS loader.

## What it does

Given a path to a 64-bit PE executable, PeLoader:

1. Reads the raw file bytes into memory
2. Parses the PE headers
3. Allocates memory and maps each section to its correct virtual address
4. Registers exception handler tables (`RtlAddFunctionTable`) so stack unwinding works
5. Resolves imports — loads each required DLL and patches the IAT with the real function addresses
6. Applies base relocations if the image couldn't be loaded at its preferred base address
7. Sets correct page protections on each section (read, write, execute) based on section characteristics
8. Jumps to the entry point and executes the loaded image

## Project structure

| File | Responsibility |
|------|----------------|
| `PeLoader.h/.cpp` | Top-level owner — holds the file buffer, parser, and loaded image together and controls their lifetime |
| `PeParser.h/.cpp` | Parses the raw PE file. Provides access to NT headers, section headers, and data directories |
| `LoadedImage.h/.cpp` | Does the actual loading: section mapping, imports, relocations, protections, and execution |
| `Utility.h` | `resolve_rva<T>()` helper and RAII wrappers for `HANDLE` and `HMODULE` |
| `main.cpp` | Reads the target PE from disk and hands it to `PeLoader` |

## How it works

### Memory allocation
`VirtualAlloc` is first attempted at the PE's preferred `ImageBase`. If that address is already taken, a fallback allocation at any available address is used and relocations are applied to fix up absolute addresses.

### Section mapping
Each section is copied from the raw file at `PointerToRawData` to its virtual address in the allocated region. Sections with no raw data (e.g. `.textbss`) are left zero-initialized by `VirtualAlloc`.

### Import resolution
The import directory table is walked. For each imported DLL, `LoadLibraryA` loads it and `GetProcAddress` resolves each function — by name or ordinal — into the IAT. The DLL handles are kept alive in `m_importedModules` for the lifetime of the loaded image so the mapped DLL memory remains valid.

### Base relocations
If the load address differs from the preferred `ImageBase`, the `.reloc` section is processed. Each relocation entry adjusts an absolute address by the delta between the actual and preferred base. Supported types: `DIR64`, `HIGHLOW`, `HIGH`, `LOW`.

### Page protections
After loading, each section's memory is protected with `VirtualProtect` according to its section characteristics (`IMAGE_SCN_MEM_READ`, `IMAGE_SCN_MEM_WRITE`, `IMAGE_SCN_MEM_EXECUTE`).

### Execution
The entry point RVA is resolved and called directly.

## Tested with
- No-CRT executables (`/ENTRY /NODEFAULTLIB`)
- CRT-linked executables (`std::cout`, MSVC debug runtime)
