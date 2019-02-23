//File name converter and directory management functions.
//Luke Lenhart (2009)
//See /docs/License.txt for details on how this code may be used.

#include "File.h"
#include "MiscStuff.h"

namespace MPMA
{
    //ctor
    Filename::Filename()
    {
        *this=FileUtils::GetWorkingDirectory();
        //Filename curDir=;
        //nameParts=curDir.nameParts;
        //isAbsolutePath=curDir.isAbsolutePath;
    }

    /*//ctor
    Filename::Filename(const std::string &name)
    {
        isAbsolutePath=false;

        std::vector<std::string> parts;
        MISC::ExplodeString(name, parts, PATH_DELIMINATORS);
        for (std::vector<std::string>::iterator i=parts.begin(); i!=parts.end(); ++i)
        {
            if (i->length()>0)
                nameParts.push_back(*i);
        }

#if defined(_WIN32) || defined(_WIN64) // windows
        if (name.length()>=2 && name[1]==':')
            topIsDrive=true;
#endif
    }*/

    //Gets the path containing this file or directory.
    std::string Filename::GetPath() const
    {
        Filename temp(*this);
        temp.StepOut();
        return temp.GetName();
    }

    //Gets the extension of a filename
    std::string Filename::GetNameExtension() const
    {
        if (nameParts.empty())
            return "";

        const std::string &name=nameParts.back();
        nsint i;
        for (i=name.length()-1; i>=0; --i)
        {
            if (name[i]=='.')
                break;
        }

        if (i>=0)
            return &name[i+1];
        else return "";
    }

    //Changes the directory to one tier above the current directory.
    void Filename::StepOut()
    {
        if (nameParts.size()==1 && nameParts[0]=="~") //stepping out of home
            StepOutOfHome();
        else if (!nameParts.empty()) //normal case
            nameParts.pop_back();
        else if (!isAbsolutePath) //we need to step "up" from the current blank relative path
        {
            *this=FileUtils::GetWorkingDirectory(); //returns an absolute path
            StepOut();
        }
    }

    //Steps into a subdirectory of the current directory.  It does not have to exist.  This may also be used to append a filename to the current path.
    void Filename::StepInto(const std::string &name)
    {
        if (name=="." || name.empty())
            return;
        else if (name=="..")
        {
            StepOut();
            return;
        }

        Filename other(name);
        for (std::vector<std::string>::iterator i=other.nameParts.begin(); i!=other.nameParts.end(); ++i)
            nameParts.push_back(*i);
    }
}
