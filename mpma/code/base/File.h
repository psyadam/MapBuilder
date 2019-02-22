//!\file File.h File name converter and directory management functions.
//Luke Lenhart (2008-2009)
//See /docs/License.txt for details on how this code may be used.

#pragma once
#include <string>
#include <vector>

namespace MPMA
{
    //!\brief Represents a file name or directory name, which can navigated or created.  This can be constructed using a either windows or a linux style path.
    //!Names that begin with ~ are relative to the users home directory.
    //!A filename may be in relative form or as an absolute path.
    class Filename
    {
    public:
        Filename(); //!<ctor - sets to the current app working directory
        Filename(const std::string &name); //!<ctor
        inline Filename(const char *name) { *this=Filename(std::string(name)); } //!<ctor

        //!Gets a string that represents the filename for the current operating system.
        inline operator std::string() const  { return GetName(); }

        //!Gets a c string that represents the filename for the current operating system.
        inline const char* c_str() const { tempCStrCache=GetName(); return tempCStrCache.c_str(); }

        //!Gets the path containing this file or directory.
        std::string GetPath() const;

        //!Gets the name of this file or directory, as is usable on the current operatin system.
        std::string GetName(bool includePath=true, bool includeExtension=true) const;

        //!Gets the extension of a filename
        std::string GetNameExtension() const;

        //!Retrieves a list of files in this directory.
        std::vector<std::string> GetFiles() const;

        //!Retrieves a list of subdirectories in this directory.
        std::vector<std::string> GetSubDirectories() const;

        //!Changes the directory to one tier above the current directory.
        void StepOut();

        //!Steps into a subdirectory of the current directory.  It does not have to exist.  This may also be used to append a filename to the current path.
        void StepInto(const std::string &name);

        //!Returns whether the current name represents the top of the file system
        bool IsRoot() const;

    private:
        std::vector<std::string> nameParts;
        bool isAbsolutePath;

        void StepOutOfHome(); //start at home, then step out

        mutable std::string tempCStrCache;
    };

    //!File system utilities.
    class FileUtils
    {
    public:
        //!Retrieves the working directory of the app and stores it in this object.
        static Filename GetWorkingDirectory();

        //!Sets the working directory for the app to the path represented by this object.
        static void SetWorkingDirectory(const Filename &path);

        //!Create a directory if it does not already exist.  Returns true if the directory existed or was created.
        static bool CreateDirectory(const Filename &path);
    };

} //namespace MPMA
