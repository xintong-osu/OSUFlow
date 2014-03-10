#include "shaderprogram.h"

using namespace std;

ShaderProgram::ShaderProgram()
{
}



// loadFile - loads text file into char*
// allocates memory - so need to delete after use
// size of file returned in fSize
char* ShaderProgram::loadFile(const char *fname,GLint &fSize)
{
    ifstream::pos_type size;
    char * memblock;
    string text;

    // file read based on example in cplusplus.com tutorial
    ifstream file (fname, ios::in|ios::binary|ios::ate);
    if (file.is_open())
    {
        size = file.tellg();
        fSize = (GLuint) size;
        memblock = new char [size];
        file.seekg (0, ios::beg);
        file.read (memblock, size);
        file.close();
        cout << "file " << fname << " loaded" << endl;
        text.assign(memblock);
    }
    else
    {
        cout << "Unable to open file " << fname << endl;
//            exit(1);
    }
    return memblock;
}

// printShaderInfoLog
// From OpenGL Shading Language 3rd Edition, p215-216
// Display (hopefully) useful error messages if shader fails to compile
void ShaderProgram::printShaderInfoLog(GLint shader)
{
    int infoLogLen = 0;
    int charsWritten = 0;
    GLchar *infoLog;

    glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &infoLogLen);

    // should additionally check for OpenGL errors here

    if (infoLogLen > 0)
    {
        infoLog = new GLchar[infoLogLen];
        // error check for fail to allocate memory omitted
        glGetShaderInfoLog(shader,infoLogLen, &charsWritten, infoLog);
        cout << "InfoLog:" << endl << infoLog << endl;
        delete [] infoLog;
    }

    // should additionally check for OpenGL errors here
}

void ShaderProgram::printProgramInfoLog(GLint program)			//A printProgramInfo routine that prints out any error messages on linking.
{
    int infoLogLen = 0;
    int charsWritten = 0;
    GLchar *infoLog;

    glGetProgramiv(program, GL_INFO_LOG_LENGTH, &infoLogLen);

    // should additionally check for OpenGL errors here

    if (infoLogLen > 0)
    {
        infoLog = new GLchar[infoLogLen];
        // error check for fail to allocate memory omitted
        glGetProgramInfoLog(program,infoLogLen, &charsWritten, infoLog);
        cout << "InfoLog:" << endl << infoLog << endl;
        delete [] infoLog;
    }

    // should additionally check for OpenGL errors here
}


unsigned int ShaderProgram::createShader(const char* vertexShader, const char* fragmentShader, const char* geometryShader)
{
    GLuint f, v;

    char *vs,*fs;

    v = glCreateShader(GL_VERTEX_SHADER);
    f = glCreateShader(GL_FRAGMENT_SHADER);

    // load shaders & get length of each
    GLint vlen;
    GLint flen;

    vs = loadFile(vertexShader,vlen);
    fs = loadFile(fragmentShader,flen);

    const char * vv = vs;
    const char * ff = fs;

    glShaderSource(v, 1, &vv,&vlen);
    glShaderSource(f, 1, &ff,&flen);

    GLint compiled;

    glCompileShader(v);
    glGetShaderiv(v, GL_COMPILE_STATUS, &compiled);
    if (!compiled)
    {
        cout << "Vertex shader not compiled." << endl;
        printShaderInfoLog(v);
    }

    glCompileShader(f);
    glGetShaderiv(f, GL_COMPILE_STATUS, &compiled);
    if (!compiled)
    {
        cout << "Fragment shader not compiled." << endl;
        printShaderInfoLog(f);
    }

    programID = glCreateProgram();

    glBindAttribLocation(programID,0, "in_Position");

    glAttachShader(programID,v);
    glAttachShader(programID,f);

    if(geometryShader!=NULL)
    {
        GLuint g;
        char *gs;

        //by Xin Tong: replace GL_GEOMETRY_SHADER_EXT with GL_GEOMETRY_SHADER, if using GLEW
        g = glCreateShader(GL_GEOMETRY_SHADER_EXT);

        GLint glen;
        gs = loadFile(geometryShader,glen);

        const char * gg = gs;
        glShaderSource(g, 1, &gg,&glen);

        glCompileShader(g);
        glGetShaderiv(g, GL_COMPILE_STATUS, &compiled);
        if (!compiled)
        {
            cout << "Geometry shader not compiled." << endl;
            printShaderInfoLog(g);
        }
        glAttachShader(programID,g);
    }

    glLinkProgram(programID);

    GLint linked;
    glGetProgramiv(programID, GL_LINK_STATUS, &linked);
    if (!linked)
    {
        cout << "Geometry shader not compiled." << endl;
        printProgramInfoLog(programID);
    }

    glUseProgram(0);

    delete [] vs; // dont forget to free allocated memory
    delete [] fs; // we allocated this in the loadFile function...

    return programID;
}
