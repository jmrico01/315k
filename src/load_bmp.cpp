#include "load_bmp.h"

#include <stdio.h>

#include "main.h"

GLuint OpenGLLoadBMP(const char* filePath)
{
    unsigned char   header[54];
    unsigned int    dataPos;
    unsigned int    width, height;
    unsigned int    imageSize;
    unsigned char*  data;

    char fullPath[PATH_MAX_LENGTH];
    CatStrings(GetAppPath(), filePath, fullPath, PATH_MAX_LENGTH);
    FILE* file = fopen(filePath, "rb");
    if (!file) {
        printf("Failed to open image at: %s\n", filePath);
        return 0;
    }

    if (fread(header, 1, 54, file) != 54) {
        printf("Incorrect BMP file: %s\n", filePath);
        fclose(file);
        return 0;
    }

    if (header[0] != 'B' || header[1] != 'M') {
        printf("Incorrect BMP file: %s\n", filePath);
        fclose(file);
        return 0;
    }

    dataPos     = *(unsigned int*)(&(header[0x0A]));
    imageSize   = *(unsigned int*)(&(header[0x22]));
    width       = *(unsigned int*)(&(header[0x12]));
    height      = *(unsigned int*)(&(header[0x16]));

    if (imageSize == 0) {
        imageSize = width * height * 3;
    }
    if (dataPos == 0) {
        dataPos = 54;
    }

    data = (unsigned char*)malloc(imageSize);
    fread(data, 1, imageSize, file);
    fclose(file);

    GLuint textureID;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height,
        0, GL_BGR, GL_UNSIGNED_BYTE, data);
    
    free(data);

    return textureID;
}