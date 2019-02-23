//file name converter and file management functions - linux implementation
//Luke Lenhart (2007-2009)
//See /docs/License.txt for details on how this code may be used.

#include "../File.h"
#include "../DebugRouter.h"
#include "../MiscStuff.h"
#include <sys/stat.h>
#include <errno.h>
#include <dirent.h>
#include <string.h>
#include <unistd.h>
#include <pwd.h>

namespace
{
    //retrieves a list of all entries in a directory of a certain type
    std::vector<std::string> GetFilesOfType(const std::string &path, bool isDir)
    {
        std::string openPath=path.empty()?"./":path;

        DIR *dir=opendir(openPath.c_str());
        if (dir==0)
            return std::vector<std::string>();
        else
        {
            std::vector<std::string> list;
            dirent *ent;
            while ((ent=readdir(dir))!=0)
            {
                if ((isDir && ent->d_type==DT_DIR) || (!isDir && (ent->d_type==DT_REG || ent->d_type==DT_BLK || ent->d_type==DT_CHR || ent->d_type==DT_FIFO || ent->d_type==DT_SOCK || ent->d_type==DT_UNKNOWN)))
                {                
                    if (std::string(".")!=ent->d_name && std::string("..")!=ent->d_name)
                        list.push_back(ent->d_name);
                }
            }
            closedir(dir);

            return list;
        }
    }
}

namespace MPMA
{
    //ctor
    Filename::Filename(const std::string &name)
    {
        isAbsolutePath=false;
        if (name.length()>=1 && name[0]=='/')
            isAbsolutePath=true;

        std::vector<std::string> parts;
        MISC::ExplodeString(name, parts, "/\\");

        if (!parts.empty() && parts.front()=="~") //home directory
        {
            parts.erase(parts.begin());

            std::vector<char> scratchMem;
            scratchMem.resize(4096);
            passwd pwdstruct;
            passwd *ppwdstruct=0;
            if (getpwuid_r(getuid(), &pwdstruct, &scratchMem[0], scratchMem.size(), &ppwdstruct)==0)
            {
                if (ppwdstruct!=0 && ppwdstruct->pw_dir!=0)
                {
                    Filename home(ppwdstruct->pw_dir);
                    parts.insert(parts.begin(), home.nameParts.begin(), home.nameParts.end());
                    isAbsolutePath=true;
                }
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

    //Gets the name of this file or directory, as is usable on the current operuint8 typeatin system.
    std::string Filename::GetName(bool includePath, bool includeExtension) const
    {
        if (nameParts.empty())
        {
            if (isAbsolutePath) return "/";
            else return "";
        }

        //add path
        std::string curName;
        if (includePath)
        {
            for (int i=0; i<(int)nameParts.size()-1; ++i) //copy all but the last
            {
                if (i!=0 || isAbsolutePath)
                    curName+="/";
                curName+=nameParts[i];
            }
        }

        //add name
        if (!curName.empty() || (nameParts.size()==1 && isAbsolutePath))
            curName+="/";
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
        return isAbsolutePath && nameParts.empty();
    }

    //retrieves a list of all files in this directory
    std::vector<std::string> Filename::GetFiles() const
    {
        return GetFilesOfType(GetName(), false);
    }

    //retrieves a list of all directories in this directory (no . or ..)
    std::vector<std::string> Filename::GetSubDirectories() const
    {
        return GetFilesOfType(GetName(), true);
    }

    //start at home, then step out
    void Filename::StepOutOfHome()
    {
        *this=Filename("/home");
    }

    //Retrieves the working directory of the app.
    Filename FileUtils::GetWorkingDirectory()
    {
        char wdir[1024];
        char *ret=getcwd(wdir, 1024);
        if (!ret)
        {
            MPMA::ErrorReport()<<"getcwd failed.\n";
            return "/";
        }
        else if (ret[0]==0)
            return "/";

        return Filename(wdir);
    }

    //Sets the working directory for the app to the path represented by this object.
    void FileUtils::SetWorkingDirectory(const Filename &path)
    {
        if (chdir(path.c_str()))
            MPMA::ErrorReport()<<"chdir failed.\n";
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
                if (!CreateDirectory(od.GetName()))
                    return false;
            }
        }

        //do the final make
        int ret=mkdir(path.c_str(), S_IRWXU|S_IRWXG|S_IROTH);
        int err=errno;
        if (!(ret==0 || err==EEXIST))
        {
            ErrorReport()<<"Create directory failed on \""<<path.GetName()<<"\": "<<strerror(errno)<<"\n";
            return false;
        }

        return true;
    }
}
