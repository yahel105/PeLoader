#pragma once
#include <iostream>
#include <Windows.h>
#include <variant>

template <typename T, typename PBase>
T resolve_rva(PBase base, size_t offset)
{
	static_assert(std::is_pointer<T>::value, "has to be a pointer");
	static_assert(std::is_pointer<PBase>::value, "has to be a pointer");
	return reinterpret_cast<T>(reinterpret_cast<PBYTE>(base) + offset);
}

struct HandleDeleter {
	using pointer = HANDLE;
	void operator()(HANDLE h) const { CloseHandle(h); }
};
using HHandle = std::unique_ptr<void, HandleDeleter>;


struct HModuleDeleter {
	using pointer = HMODULE;
	void operator()(HMODULE h) const { FreeLibrary(h); }
};
using ModuleHandle = std::unique_ptr<void,HModuleDeleter>;

