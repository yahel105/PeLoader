#pragma once
#include "PeParser.h"
class Pe
{
public:
	Pe(const PeParser& peParser);
private:
	const PeParser& m_peParser;
	PVOID m_peBase;

	void mapSections();
	void reloc();
};

