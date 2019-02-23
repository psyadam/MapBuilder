//!\file MiscStuff.h A bunch of mismatched but useful functions.
//Luke Lenhart, 1999-2007
//See /docs/License.txt for details on how this code may be used.

#pragma once

#include <string>
#include <vector>
#include <stdlib.h> //for rand()
#include "File.h"
#include "Types.h"

//!A bunch of mismatched but useful functions.
namespace MISC
{
    // -- assorted misc functions --

    //!Reads a text file into a string.
    std::string ReadFile(const MPMA::Filename &fname);
    //!Load a list of strings from the lines of a file.
    bool LoadStringList(std::vector<std::string> &outList, const MPMA::Filename &file);
    //!Converts a string to lowercase.
    std::string MakeLower(const std::string &str);
    //!Removes the first line from a string.
    std::string StripFirstLine(const std::string &str);
    //!Removes leading and trailing spaces, tabs, and line breaks.
    std::string StripPadding(const std::string &str);
    //!Counts the number of bits set to 1.
    nuint CountBits(nuint data);
    //!Determines the highest bit number thats set, or -1 if none.
    int GetHighestBit(nuint data);
    //!Breaks a string, seperated by any number of deliminators, into a list of strings.
    void ExplodeString(const std::string &inData, std::vector<std::string> &outData, const std::string &inDeliminators=" \t\n");
    //!Gets the int value out of a C-style hexadecimal string (assumes no whitespace or bad characters).
    nuint ParseHexString(const std::string &s);
    //!Returns whether a string starts with another string.
    bool StartsWith(const std::string &haystack, const std::string &needle);
    //!Returns whether a string ends with another string.
    bool EndsWith(const std::string &haystack, const std::string &needle);
    //!Returns whether a string contains another string.
    bool Contains(const std::string &haystack, const std::string &needle);
    //!Returns the index of the start of the first occurance of one string within another string starting at startIndex, or -1 if not found.
    int IndexOf(const std::string &haystack, const std::string &needle, int startIndex=0);
}

// -- other misc stuff

//leftover habits from QBasic...
inline double rndd() { return ((double)rand()/(double)RAND_MAX); }
inline float rndf() { return ((float)rand()/(float)RAND_MAX); }
