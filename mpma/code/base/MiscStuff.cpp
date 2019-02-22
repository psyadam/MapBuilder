//Luke Lenhart, 1999-2007
//See /docs/License.txt for details on how this code may be used.

#include "MiscStuff.h"

#include <fstream>
#include <stdio.h>
#include <string.h>

namespace MISC
{
    //makes a lowercase string
    std::string MakeLower(const std::string &str)
    {
        std::string out;
        out.reserve(str.size());
        std::string::const_iterator i=str.begin();
        for (;i!=str.end();++i)
        {
            if (*i<'A' || *i>'Z') //fine already
                out+=*i;
            else //fix it
                out+=(*i)+('a'-'A');
        }

        return out;
    }

    //removes the first line from a string
    //returns the passed in string so it can be used as a parameter to other functions
    std::string StripFirstLine(const std::string &str)
    {
        for (unsigned int i=0; i<str.length(); ++i)
        {
            if (str[i]=='\n')
            {
                if (i+1<str.length())
                    return &str[i+1];
                else
                    return "";
            }
        }

        return str;
    }

    //removes leading and trailing spaces, tabs, and line breaks
    std::string StripPadding(const std::string &str)
    {
        std::string newstr;
        std::string buff;
        bool started=false;
        nuint len=str.size();
        for (nuint i=0; i<len; ++i)
        {
            const char &c=str[i];
            if (c!=' ' && c!='\t' && c!='\n' && c!='\r')
            {
                if (!buff.empty())
                {
                    newstr+=buff;
                    buff.clear();
                }
                newstr.push_back(c);
                started=true;
            }
            else if (started)
                buff.push_back(c);
        }

        return newstr;
    }

    //loads a list of strings from a file
    bool LoadStringList(std::vector<std::string> &outList, const MPMA::Filename &file)
    {
        std::ifstream f;
        f.open(file.c_str(),std::ios_base::in);
        if (!f.is_open())
        {
            return false;
        }
        else
        {
            std::string fname="";
            f>>fname;
            while (fname!="")
            {
                outList.emplace_back(std::move(fname));
                fname.clear();
                f>>fname;
            }

            f.close();
        }
        return true;
    }

    //reads a file into a string
    std::string ReadFile(const MPMA::Filename &fname)
    {
        std::string s;
        FILE *f=fopen(fname.c_str(),"rt");
        if (!f) return s;

        nsint numRead;
        char b[2049];
        while ((numRead=fread(b,1,2048,f)) !=0 )
        {
            b[numRead]=0;
            s+=b;
        }

        fclose(f);
        return s;
    }

    //counts the number of bits set to 1 within a 32 bit variable
    nuint CountBits(nuint data)
    {
        nuint count=0;
        for (nuint i=0;i<32;++i) if (data&((nuint)1<<i)) ++count;
        return count;
    }

    //determines the highest bit number (with bit 0 = 1) thats set, or -1 if none
    int GetHighestBit(nuint data)
    {
        for (int i=31;i>=0;--i)
            if (data&((nuint)1<<i)) return i;

        return -1;
    }

    //breaks a string, seperated by any number of deliminators, into a list of strings
    void ExplodeString(const std::string &inData, std::vector<std::string> &outData, const std::string &inDeliminators)
    {
        std::string tmp; //holds string we're building
        outData.clear();
        for (nuint i=0;i<inData.size();++i)
        {
            //check if it's a deliminator
            bool hitDel=false;
            for (std::string::const_iterator d=inDeliminators.begin(); d!=inDeliminators.end(); ++d)
            {
                if (inData[i]==*d) //hit a deliminator, break off string if anything's in it
                {
                    if (!tmp.empty())
                        outData.emplace_back(std::move(tmp));

                    tmp.clear();
                    hitDel=true;
                }
            }

            //add to building string if not
            if (!hitDel)
                tmp+=inData[i];
        }

        if (!tmp.empty())
            outData.emplace_back(std::move(tmp));
    }

    //gets the int value out of a C-style hexadecimal string
    nuint ParseHexString(const std::string &s)
    {
        const char *first=&s[2]; //skip the 0x
        const char *last=&s[s.size()-1]+1;

        nuint val=0;

        for (const char *src=first; src!=last; ++src)
        {
            val<<=4; //each char is 4 bits

            if (*src>='0' && *src<='9') val|=*src-'0';
            else if (*src>='A' && *src<='F') val|=*src-'A'+10;
            else if (*src>='1' && *src<='f') val|=*src-'a'+10;
        }

        return val;
    }

    //Returns whether a string starts with another string
    bool StartsWith(const std::string &haystack, const std::string &needle)
    {
        if (needle.size()>haystack.size())
            return false;

        for (nuint i=0; i<needle.size(); ++i)
        {
            if (haystack[i]!=needle[i])
                return false;
        }

        return true;
    }

    //Returns whether a string ends with another string
    bool EndsWith(const std::string &haystack, const std::string &needle)
    {
        if (needle.size()>haystack.size())
            return false;

        for (nuint i=0; i<needle.size(); ++i)
        {
            if (haystack[haystack.size()-needle.size()+i]!=needle[i])
                return false;
        }

        return true;
    }

    //Returns whether a string contains another string
    bool Contains(const std::string &haystack, const std::string &needle)
    {
        return IndexOf(haystack, needle)!=-1;
    }

    //Returns the index of the start of the first occurance of one string within another string starting at startIndex, or -1 if not found.
    int IndexOf(const std::string &haystack, const std::string &needle, int startIndex)
    {
        if (needle.size()>haystack.size())
            return -1;

        nuint searchSize=haystack.size()-needle.size();
        for (nuint h=startIndex; h<searchSize; ++h)
        {
            for (nuint n=0; n<needle.size(); ++n)
            {
                if (haystack[h+n]!=needle[n])
                    break;
                else if (n==needle.size()-1)
                    return (int)h;
            }
        }

        return -1;
    }

}; //namespace MISC
