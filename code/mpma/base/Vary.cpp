//variable type variable
//useful for simple things where usability is more important than effeciency
//Luke Lenhart (2005)
//See /docs/License.txt for details on how this code may be used.

//
#include "Vary.h"
#include "DebugRouter.h"
#include "Debug.h"
#include <stdlib.h>

//msvc is missing strtoll, but has an equivilent
#if defined(_MSC_VER)
    #define strtoll _strtoi64
#endif

namespace
{
    //valid bits values
    const nuint VALB_STRING=(1<<0);
    const nuint VALB_REAL=(1<<1);
    const nuint VALB_INT=(1<<2);
}

namespace MPMA
{

// ======= Vary

// -- ctor dtor --

Vary::Vary()
{
    type=STRING;
    Clear();
}

Vary::Vary(const Vary &o)
{
    type=STRING;
    Clear();
    *this=o;
}

Vary::~Vary()
{
}

//clears contents, but not type
void Vary::Clear()
{
    curStr.clear();
    curInt=0;
    curReal=0;
    validBits=VALB_STRING|VALB_REAL|VALB_INT;
}


// -- assignment --

const Vary& Vary::operator=(const Vary &vval)
{
    type=vval.type;
    
    if (type==STRING)
    {
        curStr=vval.curStr;
        validBits=VALB_STRING;
    }
    else if (type==INTEGER)
    {
        curInt=vval.curInt;
        validBits=VALB_INT;
    }
    else
    {
        curReal=vval.curReal;
        validBits=VALB_REAL;
    }
        
    return *this;
}

const Vary& Vary::operator=(const sint64 &ival)
{
    type=INTEGER;
    validBits=VALB_INT;
    curInt=ival;
    return *this;
}

const Vary& Vary::operator=(const float &rval)
{
    type=REAL;
    validBits=VALB_REAL;
    curReal=rval;
    return *this;
}

const Vary& Vary::operator=(const std::string &sval)
{
    type=STRING;
    validBits=VALB_STRING;
    curStr=sval;
    return *this;
}

const Vary& Vary::operator=(const char *sval)
{
    type=STRING;
    validBits=VALB_STRING;
    curStr=sval;
    return *this;
}

const Vary& Vary::operator=(const std::vector<char> &sval)
{
    type=STRING;
    validBits=VALB_STRING;
    curStr.assign(&sval[0],sval.size());
    return *this;
}

Vary Vary::FromHexString(const std::string &s)
{
    int offset=0;
    if (s.length()>=2 && s[0]=='0' && (s[1]=='x' || s[1]=='X'))
        offset=2;

    return Vary(strtoll(s.c_str()+offset, 0, 16));
}


// -- conversion (non-altering) --

Vary::operator const sint64() const
{
    if (validBits&VALB_INT)
        return curInt;
    else if (type==STRING)
        curInt=strtoll(curStr.c_str(), 0, 0);
    else
        curInt=(sint64)(curReal+0.5);

    validBits|=VALB_INT;
    return curInt;
}

Vary::operator const std::string&() const
{
    if (validBits&VALB_STRING)
        return curStr;
    else if (type==INTEGER)
    {
        char tmp[64];
        sprintf(tmp,"%lli",curInt);
        curStr=tmp;
    }
    else
    {
        char tmp[64];
        sprintf(tmp,"%f",curReal);
        curStr=tmp;
    }

    validBits|=VALB_STRING;
    return curStr;
}

Vary::operator const float() const
{
    if (validBits&VALB_REAL)
        return curReal;
    else if (type==STRING)
    {
        curReal=(float)atof(curStr.c_str());
    }
    else if (type==INTEGER)
        curReal=(float)curInt;

    validBits|=VALB_REAL;
    return curReal;
}


std::string Vary::AsHexString() const
{
    if (!(validBits&VALB_INT))
        AsInt();

    char tmp[64];
    sprintf(tmp,"%llX",curInt);
    return tmp;
}

// -- addition (for numeric) and concatenation (for string) operators --

const Vary& Vary::operator+=(const Vary &var)
{
    if (type==STRING)
    {
        curStr+=(std::string)var;
        validBits=VALB_STRING;
    }
    else if (type==INTEGER)
    {
        curInt+=(sint64)var;
        validBits=VALB_INT;
    }
    else
    {
        curReal+=(float)var;
        validBits=VALB_REAL;
    }

    return *this;
}

const Vary& Vary::operator+=(const std::string &str)
{
    if (type==STRING) //easy concatenate
    {
        curStr+=str;
        validBits=VALB_STRING;
    }
    else //else... need conversion... just use slower implicit copy since I'm lazy
        *this+=(Vary)str;

    return *this;
}

const Vary& Vary::operator+=(const char *str)
{
    if (type==STRING) //easy concatenate
    {
        curStr+=str;
        validBits=VALB_STRING;
    }
    else //else... need conversion... just use slower implicit copy since I'm lazy
        *this+=(Vary)str;

    return *this;
}

const Vary Vary::operator+(const Vary &var) const
{
    Vary v=*this;
    v+=var;
    return v;
}


// -- increment and decrement --

Vary& Vary::operator++() //pre
{
    if (type==INTEGER)
    {
        ++curInt;
        validBits=VALB_INT;
    }
    else if (type==REAL)
    {
        ++curReal;
        validBits=VALB_REAL;
    }
    else //undefined operation for string
    {
        ErrorReport()<<"Warning: undefined operation on a Vary as type string: increment.\n";
        ErrorReport()<<"  -call stack:\n"<<MPMA::GetCallStack()<<"\n";
    }

    return *this;
}

Vary Vary::operator++(int) //post
{
    Vary v=*this;
    ++*this;
    return v;
}

Vary& Vary::operator--() //pre
{
    if (type==INTEGER)
    {
        --curInt;
        validBits=VALB_INT;
    }
    else if (type==REAL)
    {
        --curReal;
        validBits=VALB_REAL;
    }
    else //removes last char on string
    {
        if (curStr.size()>0) curStr.resize(curStr.size()-1);
    }

    return *this;
}

Vary Vary::operator--(int) //post
{
    Vary v=*this;
    --*this;
    return v;
}


// ======= VaryString

// -- ctor dtor --

VaryString::VaryString()
{
}

VaryString::VaryString(const Vary &o): Vary(o)
{
    MakeString();
}

VaryString::~VaryString()
{
}


// -- assignment --

const VaryString& VaryString::operator=(const Vary &vval)
{
    Vary::operator=(vval);
    MakeString();        
    return *this;
}

const VaryString& VaryString::operator=(const sint64 &ival)
{
    Vary::operator=(ival);
    MakeString();        
    return *this;
}

const VaryString& VaryString::operator=(const float &rval)
{
    Vary::operator=(rval);
    MakeString();        
    return *this;
}

const VaryString& VaryString::operator=(const std::string &sval)
{
    Vary::operator=(sval);
    MakeString();        
    return *this;
}

const VaryString& VaryString::operator=(const char *sval)
{
    Vary::operator=(sval);
    MakeString();        
    return *this;
}

const VaryString& VaryString::operator=(const std::vector<char> &sval)
{
    Vary::operator=(sval);
    MakeString();        
    return *this;
}

} //namespace MPMA
