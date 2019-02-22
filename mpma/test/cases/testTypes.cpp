//tests the sizes of the basic data types
//Luke Lenhart, 2007
//See /docs/License.txt for details on how this code may be used.

#include "../UnitTest.h"
#include <string>
#include <iostream>
#include "base/Types.h"

#ifdef DECLARE_TESTS_CODE
class TypesBasic: public UnitTest
{
public:
    #define CheckType(Type,size) CheckTypeCall<Type>(size,""#Type)

    bool Run()
    {
        bool passed=true;

        //pointer size is a special case
        std::cout<<"pointers have size "<<POINTER_SIZE;
        if (sizeof(nuint)!=POINTER_SIZE)
        {
            std::cout<<"... BAD!  They should be equal to the native platform size of "<<sizeof(nuint);
            passed=false;
        }
        std::cout<<std::endl;

        //the "assumed" types
        passed=CheckType(char,1) && passed;
        passed=CheckType(float,4) && passed;
        passed=CheckType(double,8) && passed;

        //2 platform specific types
        passed=CheckType(nuint,-1) && passed;
        passed=CheckType(nsint,-1) && passed;

        //all the normal fixed sized types
        passed=CheckType(uint8,1) && passed;
        passed=CheckType(sint8,1) && passed;
        passed=CheckType(uint16,2) && passed;
        passed=CheckType(sint16,2) && passed;
        passed=CheckType(uint32,4) && passed;
        passed=CheckType(sint32,4) && passed;
        passed=CheckType(uint64,8) && passed;
        passed=CheckType(sint64,8) && passed;

        //
        return passed;
    }

private:
    template <typename T> bool CheckTypeCall(nsint expectedLen, const std::string &name) //-1 = don't check
    {
        std::cout<<name<<" has size "<<sizeof(T);
        if (expectedLen!=-1 && sizeof(T)!=expectedLen)
        {
            std::cout<<"... BAD!  expected "<<expectedLen<<std::endl;
            return false;
        }
        std::cout<<std::endl;
        return true;
    }
};
#endif
DECLARE_TEST_CASE(TypesBasic)
