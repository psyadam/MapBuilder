//Represents a single test.
//Luke Lenhart, 2007
//See /docs/License.txt for details on how this code may be used.

#pragma once
#include <string>

class UnitTest
{
public:
    inline UnitTest() {}
    inline virtual ~UnitTest() {}

    //implement this:  (return true on test passing, false on test failing)
    virtual bool Run()=0;
};
