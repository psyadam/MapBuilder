//!\file Vary.h A variable type variable.
//useful for simple things where usability is more important than effeciency
//Luke Lenhart (2005-2011)
//See /docs/License.txt for details on how this code may be used.

//Uninitialized type is a Vary::STRING of length 0.
//Converting a string that doesn't contain a number to a number, is just 0.
//A vectors of char's can be used as an input source, and is treated as a string.
//When converting from real to integral types, number is rounded

#pragma once

#include <string>
#include <vector>
#include "Types.h"

namespace MPMA
{
    //!A variable that can have eithar an integral, real, or string type.  It can convert itself between the different types.
    class Vary
    {
    public:
        //!Type of variable.
        enum Type
        {
            INTEGER, //!<signed integer
            REAL,    //!<floating point type
            STRING   //!<character string
        };

        //ctors and dtor
        Vary(); //!<ctor
        Vary(const Vary &o); //!<ctor
        inline Vary(const sint64 &ival) {*this=ival;} //!<ctor
        inline Vary(const uint64 &ival) {*this=ival;} //!<ctor
        inline Vary(const sint32 &ival) {*this=(sint64)ival;} //!<ctor
        inline Vary(const uint32 &ival) {*this=(uint64)ival;} //!<ctor
        inline Vary(const sint16 &ival) {*this=(sint64)ival;} //!<ctor
        inline Vary(const uint16 &ival) {*this=(uint64)ival;} //!<ctor
        inline Vary(const sint8 &ival) {*this=(sint64)ival;} //!<ctor
        inline Vary(const uint8 &ival) {*this=(uint64)ival;} //!<ctor
        inline Vary(const float &rval) {*this=rval;} //!<ctor
        inline Vary(const std::string &sval) {*this=sval;} //!<ctor
        inline Vary(const char *sval) {*this=sval;} //!<ctor
        inline Vary(const std::vector<char> &sval) {*this=sval;} //!<ctor

#if defined(linux) && defined(__amd64) //need a size_t version also for some reason
        inline Vary(const size_t &ival) {*this=(sint64)ival;} //!<ctor
#endif

        ~Vary();

        //!Retrieves the current type.
        inline const Type GetType() const {return type;}

        //!Resets the value of the variable, but not the type.
        void Clear();

        //assignment
        const Vary& operator=(const Vary &vval); //!<op=
        const Vary& operator=(const sint64 &ival); //!<op=
        inline const Vary& operator=(const uint64 &ival) {*this=(sint64)ival; return *this;} //!<op=
        const Vary& operator=(const float &rval); //!<op=
        const Vary& operator=(const std::string &sval); //!<op=
        const Vary& operator=(const char *sval); //!<op=
        const Vary& operator=(const std::vector<char> &sval); //!<op=

        static Vary FromHexString(const std::string &s); //!<Creates a Vary as an integer from a hex string

        //conversion (non-altering)
        operator const sint64() const; //!<conversion
        inline operator const uint64() const {return (uint64)(sint64)*this;}
        inline operator const sint32() const {return (sint32)(sint64)*this;}
        inline operator const uint32() const {return (uint32)(sint64)*this;}
        inline operator const sint16() const {return (sint16)(sint64)*this;}
        inline operator const uint16() const {return (uint16)(sint64)*this;}
        inline operator const sint8() const {return (sint8)(sint64)*this;}
        inline operator const uint8() const {return (uint8)(sint64)*this;}
        operator const std::string&() const; //!<conversion
        operator const float() const; //!<conversion

#if defined(linux) && defined(__amd64) //need a size_t version also for some reason
        inline operator const size_t() const {return (size_t)(sint64)*this;}
#endif

        inline const sint64 AsInt() const {return (sint64)*this;} //!<conversion
        inline const std::string& AsString() const {return (const std::string&)*this;} //!<conversion
        std::string AsHexString() const; //!<conversion
        inline const float AsFloat() const {return (float)*this;} //!<conversion
        inline const char* c_str() const { return ((const std::string&)*this).c_str(); } //!<Gets a c string that represents this variable.

        //forced conversion of this instance into another type
        inline void MakeInt() {*this=(sint64)*this;} //!<Changes the current type.
        inline void MakeReal() {*this=(float)*this;} //!<Changes the current type.
        inline void MakeString() {*this=(const std::string&)*this;} //!<Changes the current type.

        //!Addition (for numeric) and concatenation (for string)
        const Vary& operator+=(const Vary &var);
        const Vary& operator+=(const std::string &str);
        const Vary& operator+=(const char *str);
        inline friend const std::string& operator+=(std::string &str, const Vary &v) { return str+=(const std::string&)v; }
        //...letting implicit conversions take care of the rest

        //!Addition (for numeric) and concatenation (for string)
        const Vary operator+(const Vary &var) const;
        inline const Vary operator+(const std::string &str) const { return *this+Vary(str); }
        inline const Vary operator+(const char *str) const { return *this+Vary(str); }
        inline friend Vary operator+(const char* str, const Vary &v) { return Vary(str)+v; }
        inline friend Vary operator+(const std::string &str, const Vary &v) { return Vary(str)+v; }
        //...letting implicit conversions take care of the rest

        //!Increment (valid only on INTEGER and REAL types)
        Vary& operator++(); //pre
        Vary operator++(int); //post
        //!Decrement (valid on all types, on strings it removes the last character (if any))
        Vary& operator--(); //pre
        Vary operator--(int); //post

        inline Vary(Vary &&o) //!<move constructor
        {
            *this=o;
        }

        inline const Vary& operator=(Vary &&o) //!<move assignment operator
        {
            type=o.type;
            validBits=o.validBits;
            curInt=o.curInt;
            curReal=o.curReal;
            curStr=(std::string&&)o.curStr;

            return *this;
        }

    protected:
        Type type; //current type of us

        //contents of us (current type has the actual value, the rest are just cache then)
        mutable std::string curStr;
        mutable sint64 curInt;
        mutable float curReal;

        mutable nuint validBits; //which members are up to date
    };


    //!This is the same as Vary, except it is always has the type of "string".
    class VaryString: public Vary
    {
    public:
        //ctors and dtor
        VaryString(); //!<ctor
        VaryString(const Vary &o); //!<ctor
        inline VaryString(const sint64 &ival) {*this=ival;} //!<ctor
        inline VaryString(const uint64 &ival) {*this=ival;} //!<ctor
        inline VaryString(const sint32 &ival) {*this=(sint64)ival;} //!<ctor
        inline VaryString(const uint32 &ival) {*this=(uint64)ival;} //!<ctor
        inline VaryString(const sint16 &ival) {*this=(sint64)ival;} //!<ctor
        inline VaryString(const uint16 &ival) {*this=(uint64)ival;} //!<ctor
        inline VaryString(const sint8 &ival) {*this=(sint64)ival;} //!<ctor
        inline VaryString(const uint8 &ival) {*this=(uint64)ival;} //!<ctor
        inline VaryString(const float &rval) {*this=rval;} //!<ctor
        inline VaryString(const std::string &sval) {*this=sval;} //!<ctor
        inline VaryString(const char *sval) {*this=sval;} //!<ctor
        inline VaryString(const std::vector<char> &sval) {*this=sval;} //!<ctor

#if defined(linux) && defined(__amd64) //need a size_t version also for some reason
        inline VaryString(const size_t &ival) {*this=(sint64)ival;} //!<ctor
#endif

        ~VaryString();

        //assignment
        const VaryString& operator=(const Vary &vval); //!<op=
        const VaryString& operator=(const sint64 &ival); //!<op=
        inline const VaryString& operator=(const uint64 &ival) {*this=(sint64)ival; return *this;} //!<op=
        const VaryString& operator=(const float &rval); //!<op=
        const VaryString& operator=(const std::string &sval); //!<op=
        const VaryString& operator=(const char *sval); //!<op=
        const VaryString& operator=(const std::vector<char> &sval); //!<op=

        //hide stuff that doesn't make sense for this type
    private:
        void MakeInt();
        void MakeReal();

        Vary& operator++();
        Vary operator++(int);
    };

} //namespace MPMA
