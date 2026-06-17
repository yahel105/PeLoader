#pragma once
#include <vector>
#include "LoadedImage.h"
#include "PeParser.h"


class PeLoader
{

public:
	PeLoader(std::vector<char>&& fileBytes) :m_fileBytes{ std::move(fileBytes) }, m_peParser(m_fileBytes), m_loadedImage(m_peParser) {}
	~PeLoader() = default;

	PeLoader(const PeLoader&) = delete;
	PeLoader& operator=(const PeLoader&) = delete;
	PeLoader(PeLoader&&) = delete;
	PeLoader& operator=(PeLoader&&) = delete;
private:
	std::vector<char> m_fileBytes;
	PeParser m_peParser;
	LoadedImage m_loadedImage;
};

