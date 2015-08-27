#pragma once
#include <string>

#include <GL/glew.h>
#include "Magick++.h"
#include <SOIL.h>

#define FOURCC_DXT1 0x31545844 // Equivalent to "DXT1" in ASCII
#define FOURCC_DXT3 0x33545844 // Equivalent to "DXT3" in ASCII
#define FOURCC_DXT5 0x35545844 // Equivalent to "DXT5" in ASCII

class Texture
{
public:
	Texture();
	Texture(GLenum TextureTarget, const std::string& FileName);
	GLuint loadTexture(const char* path);
	void UseDefaultTexture(GLuint textureID);
	GLuint loadDDS(const char * imagepath);
	bool Load();

	void Bind(GLenum TextureUnit);

private:
	std::string m_fileName;
	GLenum m_textureTarget;
	GLuint m_textureObj;
	Magick::Image m_image;
	Magick::Blob m_blob;
};

