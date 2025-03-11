#pragma once
#include <cstdint>

struct Texture
{
	uint32_t ID;
	int width, height, nrChannels;

	Texture(const char* path, bool useMipmaps);
	void Bind() const;
	//~Texture();
};