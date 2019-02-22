//tests that filename converter and file management functions work
//Luke Lenhart, 2007
//See /docs/License.txt for details on how this code may be used.

#include "../UnitTest.h"
#include <string>
#include <iostream>
#include <algorithm>
#include "base/Info.h"
#include "base/File.h"

#ifdef DECLARE_TESTS_CODE
class FilenameConversion: public UnitTest
{
public:
    bool Run()
    {
        bool passed=true;

        //
        std::string nameSingle=MPMA::Filename("name.txt");
        if (nameSingle!="name.txt")
        {
            std::cout<<"nameSingle is wrong."<<std::endl;
            passed=false;
        }

        std::string nameDouble=MPMA::Filename("start/name.txt");
        if (!(nameDouble=="start/name.txt" || nameDouble=="start\\name.txt"))
        {
            std::cout<<"nameDouble is wrong."<<std::endl;
            passed=false;
        }

        std::string nameDoubleSpaces=MPMA::Filename("start of name/end of name.txt");
        if (!(nameDoubleSpaces=="start of name/end of name.txt" || nameDoubleSpaces=="start of name\\end of name.txt"))
        {
            std::cout<<"nameDoubleSpaces is wrong."<<std::endl;
            passed=false;
        }

        //
        std::string name1=MPMA::Filename("subdirectory/test/name.txt");
        std::string name2=MPMA::Filename("subdirectory\\test\\name.txt");
        std::string name3=MPMA::Filename("subdirectory/test\\name.txt");

        if (name1!=name2 || name2!=name3)
        {
            std::cout<<"slash conversion is incorrect."<<std::endl;
            passed=false;
        }

        //
        std::string homePathBase="~";
        std::string homePathExpanded=MPMA::Filename(homePathBase);
        std::cout<<homePathBase<<" expanded to "<<homePathExpanded<<std::endl;
        std::string pathSeperator="/";
#ifdef _WIN32 //windows only
        pathSeperator="\\";
        if (homePathBase==homePathExpanded || homePathExpanded.size()<4)
        {
            std::cout<<"home directory did not expand correctly"<<std::endl;
            passed=false;
        }

        //
        std::string winStylePath=MPMA::Filename("C:\\stuff\\here");
        if (winStylePath!="C:\\stuff\\here")
        {
            std::cout<<"Windows style path evaluates wrong."<<std::endl;
            passed=false;
        }

        winStylePath=MPMA::Filename("C:");
        if (winStylePath!="C:\\")
        {
            std::cout<<"Windows style short path evaluates wrong."<<std::endl;
            passed=false;
        }
#else //linux only
        std::string linStylePath=MPMA::Filename("/stuff/here");
        if (linStylePath!="/stuff/here")
        {
            std::cout<<"Linux style path evaluates wrong."<<std::endl;
            passed=false;
        }

        linStylePath=MPMA::Filename("/stuff");
        if (linStylePath!="/stuff")
        {
            std::cout<<"Linux style short path evaluates wrong."<<std::endl;
            passed=false;
        }

        linStylePath=MPMA::Filename("/");
        if (linStylePath!="/")
        {
            std::cout<<"Linux style root path evaluates wrong."<<std::endl;
            passed=false;
        }
#endif

        std::string extraPath="something";
        std::string homePlusExtra=MPMA::Filename(homePathBase+pathSeperator+extraPath);
        std::string homePlusExtra2=homePathExpanded+pathSeperator+extraPath;

        if (homePlusExtra!=homePlusExtra2)
        {
            std::cout<<"home with subpath did not expand correctly."<<std::endl;
            passed=false;
        }

        //
        {
            MPMA::Filename fn("woot/test.txt");
            std::string fnPath=fn.GetPath();
            if (fnPath!="woot")
            {
                std::cout<<"GetPath() returned "<<fnPath<<" unexpectedly."<<std::endl;
                passed=false;
            }
            std::string fnExt=fn.GetNameExtension();
            if (fnExt!="txt")
            {
                std::cout<<"GetNameExtension returned "<<fnExt<<" unexpectedly."<<std::endl;
                passed=false;
            }
            std::string fnFile=fn.GetName(false);
            if (fnFile!="test.txt")
            {
                std::cout<<"GetName(false) returned "<<fnFile<<" unexpectedly."<<std::endl;
                passed=false;
            }
            std::string fnFileNoExt=fn.GetName(false, false);
            if (fnFileNoExt!="test")
            {
                std::cout<<"GetName(false, false) returned "<<fnFileNoExt<<" unexpectedly."<<std::endl;
                passed=false;
            }
        }

        //
        std::string nameWithDoubleDot=MPMA::Filename("one/crud/../two/three.txt");
        if (!(nameWithDoubleDot=="one/two/three.txt" || nameWithDoubleDot=="one\\two\\three.txt"))
        {
            std::cout<<"center .. case is wrong."<<std::endl;
            passed=false;
        }

        return passed;
    }

};
#endif
DECLARE_TEST_CASE(FilenameConversion)


#ifdef DECLARE_TESTS_CODE
class DirectoryCreate: public UnitTest
{
public:
    bool Run()
    {
        bool passed=true;
        passed=MPMA::FileUtils::CreateDirectory("temp/deep/directory");
        return passed;
    }

};
#endif
DECLARE_TEST_CASE(DirectoryCreate)


#ifdef DECLARE_TESTS_CODE
class DirectoryBrowsing: public UnitTest
{
public:
    bool Run()
    {
        bool passed=true;

        {
            MPMA::Filename d1;
            std::vector<std::string> files=d1.GetFiles();
            if (files.empty())
            {
                std::cout<<"No files listed in (default) dir "<<(const std::string&)d1<<std::endl;
                passed=false;
            }

            MPMA::Filename d2(d1);
            files=d2.GetFiles();
            if (files.empty())
            {
                std::cout<<"No files in dir "<<(const std::string&)d2<<std::endl;
                passed=false;
            }
        }

        {
            MPMA::Filename d("~");
            d.StepOut();
            std::vector<std::string> dirs=d.GetSubDirectories();
            if (dirs.empty())
            {
                std::cout<<"No dirs in step above home dir: "<<(const std::string&)d<<std::endl;
                passed=false;
            }

            for (std::vector<std::string>::iterator i=dirs.begin(); i!=dirs.end(); ++i)
            {
                if (*i=="." || *i=="..")
                {
                    std::cout<<"Subdirectory list contained an unexpected entry: "<<*i<<std::endl;
                    passed=false;
                }
            }
        }

        {
            //make 2 test dirs
            MPMA::Filename d("temp/testdir1");
            MPMA::FileUtils::CreateDirectory(d);
            d.StepOut();
            d.StepInto("testdir2");
            MPMA::FileUtils::CreateDirectory(d);
            d.StepOut();

            //verify they're in dir list
            d=MPMA::Filename();
            std::vector<std::string> dirs=d.GetSubDirectories();
            if (std::find(dirs.begin(), dirs.end(), "temp")==dirs.end())
            {
                std::cout<<"Could not find directory \"temp\" inside of current directory."<<std::endl;
                passed=false;
            }
            else
            {
                d.StepInto("temp");
                std::vector<std::string> dirs=d.GetSubDirectories();
                if (std::find(dirs.begin(), dirs.end(), "testdir1")==dirs.end() || std::find(dirs.begin(), dirs.end(), "testdir2")==dirs.end())
                {
                    std::cout<<"Could not find directory testdir1 and testdir2 inside of temp directory."<<std::endl;
                    passed=false;
                }
            }
        }

        {
            //get path and set it to itself
            MPMA::Filename curPath=MPMA::FileUtils::GetWorkingDirectory();
            std::cout<<"Working directory is: "<<(const std::string&)curPath<<std::endl;
            MPMA::FileUtils::SetWorkingDirectory(curPath);
        }

        {
            //step up from current
            MPMA::Filename stepUp1("..");
            MPMA::Filename stepUp2=MPMA::FileUtils::GetWorkingDirectory();
            stepUp2.StepOut();
            std::string upName1=stepUp1;
            std::string upName2=stepUp2;
            if (upName1!=upName2)
            {
                std::cout<<"Stepping up from current gave the wrong result.";
                passed=false;
            }
        }

        return passed;
    }

};
#endif
DECLARE_TEST_CASE(DirectoryBrowsing)
