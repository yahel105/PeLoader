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
	bool resolveImports();
	bool registerExceptionHandlers();
	void reloc();
	bool sectionsProtect();
	void execute();
};

bool IsValidRelocTarget(PVOID base, size_t imageSize, void* toReloc, size_t width);
