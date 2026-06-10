#pragma once
#include "PeParser.h"
class LoadedImage
{
public:
	LoadedImage(const PeParser& peParser);
private:
	const PeParser& m_peParser;
	PVOID m_peBase;

	void mapSections();
	boolean resolveImports();
	boolean registerExeptionHandlers();
	void reloc();
	boolean sectionsProtect();
	void execute();
};

