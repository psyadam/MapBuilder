//todo... header...

#include "Shader.h"

#ifdef MPMA_COMPILE_GFX

#include "../base/MiscStuff.h"
#include "../base/DebugRouter.h"
#include "../base/Debug.h"

namespace GFX
{
    ShaderCode::~ShaderCode()
    {
        if (IsOnlyReference())
        {
            Free();
        }
    }

    bool ShaderCode::Create(GLenum shaderType)
    {
        Free();
        Data().object=glCreateShader(shaderType);
        return Data().object!=0;
    }

    void ShaderCode::Free()
    {
        if (Data().object)
        {
            glDeleteShader(Data().object);
            Data().object=0;
        }
    }

    bool ShaderCode::Compile(GLenum shaderType, const std::string &code)
    {
        std::vector<std::string> codes;
        codes.emplace_back(code);
        return Compile(shaderType, codes);
    }

    bool ShaderCode::Compile(GLenum shaderType, const std::vector<std::string> &codes)
    {
        Create(shaderType);
        return CompileInternal(codes);
    }

    bool ShaderCode::CompileFile(GLenum shaderType, const MPMA::Filename &file)
    {
        std::vector<MPMA::Filename> files;
        files.emplace_back(file);
        return CompileFiles(shaderType, files);
    }

    bool ShaderCode::CompileFiles(GLenum shaderType, const std::vector<MPMA::Filename> &files)
    {
        std::vector<std::string> codes;
        for (std::vector<MPMA::Filename>::const_iterator i=files.begin(); i!=files.end(); ++i)
        {
            std::string code=MISC::ReadFile(*i);
            if (code.empty())
            {
                MPMA::ErrorReport()<<"Failed to read shader code from file: "<<i->c_str()<<"\n";
                return false;
            }

            codes.emplace_back(std::move(code));
        }

        Data().origFilenames=files;
        return Compile(shaderType, codes);
    }

    bool ShaderCode::CompileInternal(const std::vector<std::string> &codes)
    {
        //reformat the code list into C arrays, then store the code
        std::vector<const char*> codeStrings;
        std::vector<GLint> codeLengths;
        for (std::vector<std::string>::const_iterator i=codes.begin(); i!=codes.end(); ++i)
        {
            codeStrings.emplace_back(i->c_str());
            codeLengths.emplace_back((GLint)i->length());
        }

        glShaderSourceARB(*this, (GLsizei)codes.size(), &codeStrings[0], &codeLengths[0]);

        //compile and store the messages from it
        glCompileShaderARB(*this);

        GLint compileStatus=0;
        glGetShaderiv(*this, GL_COMPILE_STATUS, &compileStatus);

        GLint infoLogLength=0;
        glGetShaderiv(*this, GL_INFO_LOG_LENGTH, &infoLogLength);
        std::vector<char> tempInfoLog;
        tempInfoLog.resize(infoLogLength+1);
        glGetShaderInfoLog(*this, infoLogLength, &infoLogLength, &tempInfoLog[0]);
        tempInfoLog.resize(infoLogLength);
        tempInfoLog.push_back(0);
        Data().compileMessages=&tempInfoLog[0];

        std::string name;
        if (!Data().origFilenames.empty())
        {
            name="(from file";
            if (Data().origFilenames.size()==1)
            {
                name+=": ";
                name+=Data().origFilenames[0];
            }
            else
            {
                name+="s: ";
                for (std::vector<MPMA::Filename>::const_iterator i=Data().origFilenames.begin(); i!=Data().origFilenames.end(); ++i)
                {
                    if (i!=Data().origFilenames.begin())
                        name+=", ";

                    name+=i->c_str();
                }
            }

            name+=")";
        }

        if (!compileStatus)
        {
            MPMA::ErrorReport()<<"Failed to compile shader code "<<name<<":\n"<<Data().compileMessages<<"\nstack:\n"<<MPMA::GetCallStack()<<"\n\n";
            return false;
        }

#ifdef GFX_REPORT_ALL_SHADER_MESSAGES
        if (!Data().compileMessages.empty())
        {
            MPMA::ErrorReport()<<"Successfully compiled shader code "<<name<<" but there was output:\n"<<Data().compileMessages<<"\nstack:\n"<<MPMA::GetCallStack()<<"\n\n";
        }
#endif

        return true;
    }

    // --

    ShaderProgram::~ShaderProgram()
    {
        if (IsOnlyReference())
        {
            Free();
        }
    }

    bool ShaderProgram::Create()
    {
        Free();
        Data().object=glCreateProgram();
        return Data().object!=0;
    }

    void ShaderProgram::Free()
    {
        if (Data().object)
        {
            glDeleteProgram(Data().object);
            Data().object=0;
        }
    }

    void ShaderProgram::AttachShader(const ShaderCode &code)
    {
        glAttachShader(*this, code);
    }

    void ShaderProgram::DetatchShader(const ShaderCode &code)
    {
        glDetachShader(*this, code);
    }

    bool ShaderProgram::Link()
    {
        //link and store the messages from it
        glLinkProgramARB(*this);

        GLint linkStatus=0;
        glGetProgramiv(*this, GL_LINK_STATUS, &linkStatus);

        GLint infoLogLength=0;
        glGetProgramiv(*this, GL_INFO_LOG_LENGTH, &infoLogLength);
        std::vector<char> tempInfoLog;
        tempInfoLog.resize(infoLogLength+1);
        glGetProgramInfoLog(*this, infoLogLength, &infoLogLength, &tempInfoLog[0]);
        tempInfoLog.resize(infoLogLength);
        tempInfoLog.push_back(0);
        Data().linkMessages=&tempInfoLog[0];

        if (!linkStatus)
        {
            MPMA::ErrorReport()<<"Failed to link shader program:\n"<<Data().linkMessages<<"\nstack:\n"<<MPMA::GetCallStack()<<"\n\n";
            return false;
        }

        return true;
    }

    bool ShaderProgram::CreateAndLink(const std::vector<ShaderCode> &codes)
    {
        return CreateAndLink(&codes[0], codes.size());
    }

    bool ShaderProgram::CreateAndLink(const ShaderCode *codes, nuint count)
    {
        Create();
        for (nuint i=0; i<count; ++i)
        {
            AttachShader(codes[i]);
        }
        return Link();
    }
}

#endif //#ifdef MPMA_COMPILE_GFX
