#pragma once
#include <iostream>
#include <Windows.h>

template <typename T, typename PBase>
T resolve_rva(PBase base, size_t offset)
{
	static_assert(std::is_pointer<T>::value, "has to be a pointer");
	static_assert(std::is_pointer<PBase>::value, "has to be a pointer");
	return reinterpret_cast<T>(reinterpret_cast<PBYTE>(base) + offset);
}
