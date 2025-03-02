#include "include/texture.hpp"
#include "include/stb_image.h"
#include <glad/glad.h>

#include <iostream>

Texture::Texture(const char* path, bool useMipmaps) 
{
	glGenTextures(1, &ID);
	glBindTexture(GL_TEXTURE_2D, ID);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	uint8_t* data = stbi_load(path, &width, &height, &nrChannels, 0);
	if (data) 
	{
		uint32_t format = (nrChannels == 4) ? GL_RGBA : GL_RGB;
		glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);

		if (useMipmaps) 
		{
			glGenerateMipmap(GL_TEXTURE_2D);
		}
		stbi_image_free(data);
	}
	else 
	{
		std::cerr << "Failed to load texture" << std::endl;
	}
}

void Texture::Bind() const 
{
	glBindTexture(GL_TEXTURE_2D, ID);
}

Texture::~Texture() 
{
	glDeleteTextures(1, &ID);
}