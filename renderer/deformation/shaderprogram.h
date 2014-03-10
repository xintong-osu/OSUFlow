#ifndef SHADERPROGRAM_H
#define SHADERPROGRAM_H

#include <GL\glew.h>
#include <fstream>
#include <iostream>


class ShaderProgram
{
public:
    ShaderProgram();
    unsigned int createShader(const char* vertexShader, const char* fragmentShader, const char* geometryShader=0);
    unsigned int programID;

private:
    // loadFile - loads text file into char*
    // allocates memory - so need to delete after use
    // size of file returned in fSize
    char* loadFile(const char *fname,GLint &fSize);

    // printShaderInfoLog
    // From OpenGL Shading Language 3rd Edition, p215-216
    // Display (hopefully) useful error messages if shader fails to compile
    void printShaderInfoLog(GLint shader);

    void printProgramInfoLog(GLint program);



};

#endif // SHADERPROGRAM_H
