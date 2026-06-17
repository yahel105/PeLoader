#pragma once
#include <vector>
#include "PeParser.h"
#include "Utility.h"


class LoadedImage
{
public:
	LoadedImage(const PeParser& peParser);

	LoadedImage(const LoadedImage&) = delete;
	LoadedImage& operator=(const LoadedImage&) = delete;
	LoadedImage(LoadedImage&&) = delete;
	LoadedImage& operator=(LoadedImage&&) = delete;
	~LoadedImage() = default;

private:
	const PeParser& m_peParser;
	PVOID m_peBase;
	std::vector<ModuleHandle> m_importedModules;

	void mapSections();
	boolean resolveImports();
	boolean registerExeptionHandlers();
	void reloc();
	boolean sectionsProtect();
	void execute();
};

