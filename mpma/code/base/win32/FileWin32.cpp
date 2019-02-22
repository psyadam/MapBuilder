//file name converter and file management functions - windows implementation
//Luke Lenhart (2007-2009)
//See /docs/License.txt for details on how this code may be used.

#include "../File.h"
#include "../DebugRouter.h"
#include "../MiscStuff.h"
#include "evil_windows.h"
#include <shlobj.h>

namespace
{
    std::vector<std::string> FindFilesOfType(const std::string &path, bool getDirs)
    {
        std::string findStr=path.empty()?"*.*":path+"\\*.*";

        //start the find process
        WIN32_FIND_DATA file;
        HANDLE cur=FindFirstFile(findStr.c_str(),&file);
        if (cur==INVALID_HANDLE_VALUE || cur==0)
            return std::vector<std::string>();

        //process the files
        std::vector<std::string> list;
        do
        {
            bool isDir=(file.dwFileAttributes&FILE_ATTRIBUTE_DIRECTORY)!=0;
            if (getDirs==isDir && std::string(file.cFileName)!="." && std::string(file.cFileName)!="..")
            {
                list.push_back(file.cFileName);
            }
        } while (0!=FindNextFile(cur, &file));

        FindClose(cur);

        return list;
    }
}

namespace MPMA
{
    //ctor
    Filename::Filename(const std::string &name)
    {
        isAbsolutePath=false;
        if (name.length()>=2 && name[1]==':')
            isAbsolutePath=true;

        std::vector<std::string> parts;
        MISC::ExplodeString(name, parts, "\\:/");

        if (!parts.empty() && parts.front()=="~") //home directory
        {
            parts.erase(parts.begin());

            char homePath[MAX_PATH];
            if (!FAILED(SHGetFolderPath(0, CSIDL_PERSONAL, 0, 0, homePath)))
            {
                Filename home(homePath);
                parts.insert(parts.begin(), home.nameParts.begin(), home.nameParts.end());
                isAbsolutePath=true;
            }
        }

        if (!parts.empty())
        {
            if (parts.size()==1 && parts[0]!=".." && parts[0]!=".") //single thing to do, just set it
                nameParts.push_back(parts.front());
            else //possibly multipart
            {
                for (std::vector<std::string>::iterator i=parts.begin(); i!=parts.end(); ++i)
                    StepInto(*i);
            }
        }
    }

    //!Gets the name of this file or directory, as is usable on the current operatin system.
    std::string Filename::GetName(bool includePath, bool includeExtension) const
    {
        if (nameParts.empty())
        {
            if (isAbsolutePath) return "*";
            else return "";
        }

        //add path
        std::string curName;
        if (includePath)
        {
            int startInd=0;
            if (isAbsolutePath)
            {
                startInd=1;
                curName+=nameParts[0]+":";
            }

            for (int i=startInd; i<(int)nameParts.size()-1; ++i) //copy all but the last
            {
                if (i!=startInd || isAbsolutePath)
                    curName+="\\";
                curName+=nameParts[i];
            }
        }

        //add name
        if (!curName.empty())
            curName+="\\";

        if (!(isAbsolutePath && nameParts.size()==1))
            curName+=nameParts.back();

        //remove extension
        std::string extension=GetNameExtension();
        if (!includeExtension && !extension.empty())
            curName.resize(curName.size()-extension.size()-1);

        return curName;
    }

    //Returns whether the current name represents the top of the file system
    bool Filename::IsRoot() const
    {
        return isAbsolutePath && nameParts.size()<=1;
    }

    //start at home, then step out
    void Filename::StepOutOfHome()
    {
        *this=Filename("~");
        StepOut();
    }

    //retrieves a list of all files in this directory
    std::vector<std::string> Filename::GetFiles() const
    {
        return FindFilesOfType(GetName(), false);
    }
    
    //retrieves a list of all directories in this directory (no . or ..)
    std::vector<std::string> Filename::GetSubDirectories() const
    {
        //TODO: get drives list rather than listing all letters
        if (isAbsolutePath && nameParts.empty())
        {
            std::vector<std::string> drives;
            for (char letter='A'; letter<='Z'; ++letter)
            {
                char tmp[2]={0};
                tmp[0]=letter;
                drives.push_back(tmp);
            }
            return drives;
        }

        return FindFilesOfType(GetName(), true);
    }

    //Retrieves the working directory of the app.
    Filename FileUtils::GetWorkingDirectory()
    {
        char wdir[1024];
        int ret=GetFullPathName("x", 1024, wdir, 0);
        if (!ret || ret>1024)
        {
            MPMA::ErrorReport()<<"GetFullPathName failed.\n";
            return "C:\\";
        }

        wdir[strlen(wdir)-2]=0;
        return Filename(wdir);
    }

    //Sets the working directory for the app to the path represented by this object.
    void FileUtils::SetWorkingDirectory(const Filename &path)
    {
        if (!SetCurrentDirectory(path.c_str()))
            MPMA::ErrorReport()<<"SetCurrentDirectory failed.\n";
    }

    //create a directory if it does not already exist
    bool FileUtils::CreateDirectory(const Filename &path)
    {
        //make parents of that dir if we need to
        Filename od(path);
        if (od.GetName().length()!=0)
        {
            od.StepOut();
            if (od.GetName().length()!=0 && !od.IsRoot())
            {
                if (!CreateDirectory(od))
                    return false;
            }
        }
        
        //do the final make
        if (!CreateDirectoryA(path.c_str(), 0))
        {
            int err=GetLastError();
            if (err!=ERROR_ALREADY_EXISTS)
            {
                ErrorReport()<<"Create directory failed on \""<<path.GetName()<<"\".\n";
                return false;
            }
        }
        
        return true;
    }
}
