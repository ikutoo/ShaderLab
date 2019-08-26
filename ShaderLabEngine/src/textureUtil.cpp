#include "textureUtil.h"
#include <iostream>
#include <SOIL2/stb_image.h>
#include <SOIL2/SOIL2.h>

//**********************************************************************************************
//FUNCTION:
GLuint util::loadCubemap(const std::vector<std::string>& vFaces, bool vGenerateMipMap)
{
	GLuint TextureID;
	glGenTextures(1, &TextureID);

	int Width, Height, NrComponents;
	float *pImageData = nullptr;

	glBindTexture(GL_TEXTURE_CUBE_MAP, TextureID);
	for (GLuint i = 0; i < vFaces.size(); ++i)
	{
		pImageData = stbi_loadf(vFaces[i].c_str(), &Width, &Height, &NrComponents, 0);
		glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB16F, Width, Height, 0, GL_RGB, GL_FLOAT, pImageData);
		stbi_image_free(pImageData);
	}
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

	if (vGenerateMipMap)
	{
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glGenerateMipmap(GL_TEXTURE_CUBE_MAP);
	}
	else
	{
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	}
	glBindTexture(GL_TEXTURE_CUBE_MAP, 0);

	return TextureID;
}

//**********************************************************************************************
//FUNCTION:
GLuint util::setupCubemap(int vWidth, int vHeight, bool vGenerateMipMap)
{
	GLuint Cubemap;
	glGenTextures(1, &Cubemap);
	glBindTexture(GL_TEXTURE_CUBE_MAP, Cubemap);
	for (unsigned int i = 0; i < 6; ++i)
	{
		glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB32F, vWidth, vHeight, 0, GL_RGB, GL_FLOAT, nullptr);
	}
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	if (vGenerateMipMap)
	{
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR_MIPMAP_LINEAR);

		glGenerateMipmap(GL_TEXTURE_CUBE_MAP);
	}
	else
	{
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	}

	glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
	return Cubemap;
}

//**********************************************************************************************
//FUNCTION:
GLuint util::loadTexture(const char *vPath, GLint vFilterMode, GLint vWrapMode, bool vVerticallyFlip, bool vGenerateMipMap)
{
	_ASSERTE(vPath);

	unsigned Flags = 0;
	if (vVerticallyFlip) Flags |= SOIL_FLAG_INVERT_Y;
	if (vGenerateMipMap) Flags |= SOIL_FLAG_MIPMAPS;

	GLuint TextureID = SOIL_load_OGL_texture(vPath, SOIL_LOAD_AUTO, 0, Flags);
	if (TextureID <= 0, "Failed to load texture due to failure of SOIL_load_OGL_texture().", 0);

	glBindTexture(GL_TEXTURE_2D, TextureID);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, vWrapMode);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, vWrapMode);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, vFilterMode);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, vFilterMode);
	glBindTexture(GL_TEXTURE_2D, 0);

	return TextureID;
}

//**********************************************************************************************
//FUNCTION:
GLuint util::setupTexture2D(unsigned int vWidth, unsigned int vHeight, GLint vInternalFormat, GLenum vFormat, GLenum vType, GLint vFilterMode, GLint vWrapMode, bool vGenerateMipMap)
{
	GLuint texture;
	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_2D, texture);
	glTexImage2D(GL_TEXTURE_2D, 0, vInternalFormat, vWidth, vHeight, 0, vFormat, vType, nullptr);

	if (vGenerateMipMap) glGenerateMipmap(GL_TEXTURE_2D);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, vWrapMode);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, vWrapMode);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, vFilterMode);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, vFilterMode);

	glBindTexture(GL_TEXTURE_2D, 0);

	return texture;
}