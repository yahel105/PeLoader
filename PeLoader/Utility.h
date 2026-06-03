#pragma once
#include <iostream>
#include <Windows.h>

template <typename T>
T resolve_rva(PVOID base, size_t offset)
{
	static_assert(std::is_pointer<T>::value, "has to be a pointer");
	return reinterpret_cast<T>(reinterpret_cast<PBYTE>(base) + offset);
}
