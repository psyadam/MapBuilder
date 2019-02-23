//!\file Shader.h OpenGL Shader wrapper.
//Luke Lenhart, 2010
//See /docs/License.txt for details on how this code may be used.

#pragma once

#include "../Config.h"

#ifdef MPMA_COMPILE_GFX

#include "../base/Types.h"
#include "../base/ReferenceCount.h"
#include "../base/File.h"
#include "../gfxsetup/GL.h"
#include "../gfxsetup/Extensions.h"
#include <string>
#include <vector>

#ifdef MPMA_COMPILE_GEO
    #include "../geo/Geo.h"
#endif

namespace GFX
{
    //!Represents a uniform variable in a shader program.  The program must be bound before any values may be set.
    class UniformVariable
    {
    public:
        //!Retrieves the location of the uniform variable.
        inline operator GLint() const { return location; }

        //!Returns whether this object reprents a valid variable.
        inline bool Exists() const { return location!=-1; }

        inline void SetFloat1(float f0) { glUniform1fARB(*this, f0); } //!<Assigns a value to this variable in the shader.
        inline void SetFloat2(float f0, float f1) { glUniform2fARB(*this, f0, f1); } //!<Assigns a value to this variable in the shader.
        inline void SetFloat3(float f0, float f1, float f2) { glUniform3fARB(*this, f0, f1, f2); } //!<Assigns a value to this variable in the shader.
        inline void SetFloat4(float f0, float f1, float f2, float f3) { glUniform4fARB(*this, f0, f1, f2, f3); } //!<Assigns a value to this variable in the shader.

        inline void SetFloat1(const float *f, GLsizei arrayCount=1) { glUniform1fvARB(*this, arrayCount, f); } //!<Assigns a value to this variable in the shader.
        inline void SetFloat2(const float *f, GLsizei arrayCount=1) { glUniform2fvARB(*this, arrayCount, f); } //!<Assigns a value to this variable in the shader.
        inline void SetFloat3(const float *f, GLsizei arrayCount=1) { glUniform3fvARB(*this, arrayCount, f); } //!<Assigns a value to this variable in the shader.
        inline void SetFloat4(const float *f, GLsizei arrayCount=1) { glUniform4fvARB(*this, arrayCount, f); } //!<Assigns a value to this variable in the shader.

        inline void SetInt1(int i0) { glUniform1iARB(*this, i0); } //!<Assigns a value to this variable in the shader.
        inline void SetInt2(int i0, int i1) { glUniform2iARB(*this, i0, i1); } //!<Assigns a value to this variable in the shader.
        inline void SetInt3(int i0, int i1, int i2) { glUniform3iARB(*this, i0, i1, i2); } //!<Assigns a value to this variable in the shader.
        inline void SetInt4(int i0, int i1, int i2, int i3) { glUniform4iARB(*this, i0, i1, i2, i3); } //!<Assigns a value to this variable in the shader.

        inline void SetInt1(const int *i, GLsizei arrayCount=1) { glUniform1ivARB(*this, arrayCount, i); } //!<Assigns a value to this variable in the shader.
        inline void SetInt2(const int *i, GLsizei arrayCount=1) { glUniform2ivARB(*this, arrayCount, i); } //!<Assigns a value to this variable in the shader.
        inline void SetInt3(const int *i, GLsizei arrayCount=1) { glUniform3ivARB(*this, arrayCount, i); } //!<Assigns a value to this variable in the shader.
        inline void SetInt4(const int *i, GLsizei arrayCount=1) { glUniform4ivARB(*this, arrayCount, i); } //!<Assigns a value to this variable in the shader.

        inline void SetMatrix2x2(const float *m, bool transpose=false, GLsizei arrayCount=1) { glUniformMatrix2fvARB(*this, arrayCount, transpose, m); } //!<Assigns a value to this variable in the shader.
        //inline void SetMatrix2x3(const float *m, bool transpose=false, GLsizei arrayCount=1) { glUniformMatrix2x3fvARB(*this, arrayCount, transpose, m); } //may enable later
        //inline void SetMatrix2x4(const float *m, bool transpose=false, GLsizei arrayCount=1) { glUniformMatrix2x4fvARB(*this, arrayCount, transpose, m); } //may enable later
        //inline void SetMatrix3x2(const float *m, bool transpose=false, GLsizei arrayCount=1) { glUniformMatrix3x2fvARB(*this, arrayCount, transpose, m); } //may enable later
        inline void SetMatrix3x3(const float *m, bool transpose=false, GLsizei arrayCount=1) { glUniformMatrix3fvARB(*this, arrayCount, transpose, m); } //!<Assigns a value to this variable in the shader.
        //inline void SetMatrix3x4(const float *m, bool transpose=false, GLsizei arrayCount=1) { glUniformMatrix3x4fvARB(*this, arrayCount, transpose, m); } //may enable later
        //inline void SetMatrix4x2(const float *m, bool transpose=false, GLsizei arrayCount=1) { glUniformMatrix4x2fvARB(*this, arrayCount, transpose, m); } //may enable later
        //inline void SetMatrix4x3(const float *m, bool transpose=false, GLsizei arrayCount=1) { glUniformMatrix4x3fvARB(*this, arrayCount, transpose, m); } //may enable later
        inline void SetMatrix4x4(const float *m, bool transpose=false, GLsizei arrayCount=1) { glUniformMatrix4fvARB(*this, arrayCount, transpose, m); } //!<Assigns a value to this variable in the shader.

        //!Sets a texture stage number.
        inline void SetTextureStage(int stage) { SetInt1(stage); }
        
#ifdef MPMA_COMPILE_GEO
        inline void SetVector2(const GEO::Vector2 &v) { SetFloat2(&v[0]); } //!<Assigns a value to this variable in the shader.
        inline void SetVector3(const GEO::Vector3 &v) { SetFloat3(&v[0]); } //!<Assigns a value to this variable in the shader.
        inline void SetVector4(const GEO::Vector4 &v) { SetFloat4(&v[0]); } //!<Assigns a value to this variable in the shader.

        inline void SetMatrix2x2(const GEO::Matrix2 &m, bool transpose=false) { SetMatrix2x2(&m[0][0], transpose); } //!<Assigns a value to this variable in the shader.
        inline void SetMatrix3x3(const GEO::Matrix3 &m, bool transpose=false) { SetMatrix3x3(&m[0][0], transpose); } //!<Assigns a value to this variable in the shader.
        inline void SetMatrix4x4(const GEO::Matrix4 &m, bool transpose=false) { SetMatrix4x4(&m[0][0], transpose); } //!<Assigns a value to this variable in the shader.
#endif

    private:
        inline UniformVariable(GLint loc): location(loc) {}

        GLint location;
        friend class ShaderProgram;
    };

    // --

    struct ShaderCodeData
    {
        GLuint object;
        std::vector<MPMA::Filename> origFilenames;
        std::string compileMessages;

        inline ShaderCodeData(): object(0)
        {}
    };

    //!Represents source and compiled shader code pieces.
    class ShaderCode: public MPMA::ReferenceCountedData<ShaderCodeData>
    {
    public:
        virtual ~ShaderCode();

        //!Creates an shader code of a specific type.
        bool Create(GLenum shaderType); //<!GL_VERTEX_SHADER, GL_FRAGMENT_SHADER, or GL_GEOMETRY_SHADER_EXT

        //!Frees the shader code.
        void Free();

        //!Creates and compiles a string reperesents shader code.
        bool Compile(GLenum shaderType, const std::string &code);
        //!Creates and compiles a set string reperesents shader codes.
        bool Compile(GLenum shaderType, const std::vector<std::string> &codes);

        //!Creates and compiles a file containing shader code.
        bool CompileFile(GLenum shaderType, const MPMA::Filename &file);
        //!Creates and compiles a set of files containing shader codes.
        bool CompileFiles(GLenum shaderType, const std::vector<MPMA::Filename> &files);

        //!Retrieves messages generated by the shader compiler.
        inline const std::string& GetCompileMessages() const { return Data().compileMessages; }

        //!Returns the OpenGL shader source object.
        inline operator GLuint() const { return Data().object; }

    private:
        bool CompileInternal(const std::vector<std::string> &codes);
    };

    // --

    struct ShaderProgramData
    {
        GLuint object;
        std::string linkMessages;

        inline ShaderProgramData(): object(0)
        {}
    };

    //!Represents a shader program.
    class ShaderProgram: public MPMA::ReferenceCountedData<ShaderProgramData>
    {
    public:
        virtual ~ShaderProgram();

        //!Creates an empty shader program.
        bool Create();
        //!Frees the shader program.
        void Free();

        //!Attaches a shader source to the program.
        void AttachShader(const ShaderCode &code);
        //!Detaches ashader source from the program.
        void DetatchShader(const ShaderCode &code);
        //!Links all of the attached shader sources into the completed program.
        bool Link();

        //!Attaches and links a set of shader sources into a completed program.
        bool CreateAndLink(const std::vector<ShaderCode> &codes);
        //!Attaches and links a set of shader sources into a completed program.
        bool CreateAndLink(const ShaderCode *codes, nuint count);

        //!Retrieves messages generated by the shader linker.
        inline const std::string& GetLinkMessages() const { return Data().linkMessages; }

        //!Binds the shader program.
        inline void Bind()
        {
            glUseProgram(*this);
        }

        //!Unbinds the shader program.
        inline void Unbind()
        {
            glUseProgram(0);
        }

        //!Retrieves the uniform variable location of a variable inside a program.
        inline UniformVariable FindVariable(const std::string &name)
        {
            return UniformVariable(glGetUniformLocationARB(*this, name.c_str()));
        }

        //!Sets the uniform output which stores the result of a fragment shader
        inline void SetFragmentOutputVariable(const std::string &variableName, GLuint colorNumber=0) { if (glBindFragDataLocation) glBindFragDataLocation(*this, colorNumber, variableName.c_str()); }

        //!Returns the OpenGL shader program object.
        inline operator GLuint() const { return Data().object; }
    };

    //!Helper to automatically bind a shader program then unbind it when it leaves scope.
    class AutoBindShaderProgram
    {
    public:
        //!
        inline AutoBindShaderProgram(ShaderProgram &shaderToBind): prog(shaderToBind) { prog.Bind(); }
        //!
        inline ~AutoBindShaderProgram() { prog.Unbind(); }

    private:
        ShaderProgram &prog;

        //prevent copying
        AutoBindShaderProgram(const AutoBindShaderProgram &o);
        bool operator=(const AutoBindShaderProgram &o);
    };
}

#endif //#ifdef MPMA_COMPILE_GFX
