//!\file Types.h \brief Definition of basic data types, so there is no ambiguity between different platforms and compilers.
//!\details
//!The following types are defined:       \n
//! uint8  - 8 bit unsigned integer       \n
//! sint8  - 8 bit signed integer         \n
//! uint16 - 16 bit unsigned integer      \n
//! sint16 - 16 bit signed integer        \n
//! uint32 - 32 bit unsigned integer      \n
//! sint32 - 32 bit signed integer        \n
//! uint64 - 64 bit unsigned integer      \n
//! sint64 - 64 bit signed integer        \n
//! nuint   - unsigned integer of the size native to the current platform   \n
//! nsint   - signed integer of the size native to the current platform     \n
//!
//! POINTER_SIZE - number of bytes needed to store a pointer
//!
//!The following data types are assumed for all platforms: \n
//! char   - 8 bit signed int (sint8)           \n
//! float  - 32 bit floating point number       \n
//! double - 64 bit floating point number       \n
//!
//!The following storage specifiers are defined: \n
//! THREAD_LOCAL  - variable is allocated from thread-local storage

//Luke Lenhart, 2007-2010
//See /docs/License.txt for details on how this code may be used.

// -- platform implementations --

#if defined(_WIN64) // msvc - 64 bit
    typedef unsigned __int64  nuint;
    typedef signed __int64    nsint;
    typedef unsigned char     uint8;
    typedef signed char       sint8;
    typedef unsigned short    uint16;
    typedef signed short      sint16;
    typedef unsigned int      uint32;
    typedef signed int        sint32;
    typedef unsigned __int64  uint64;
    typedef signed __int64    sint64;
    #define POINTER_SIZE      sizeof(void*)
    #define THREAD_LOCAL      __declspec(thread)
#elif defined(_WIN32) // msvc - 32 bit
    typedef unsigned int      nuint;
    typedef signed int        nsint;
    typedef unsigned char     uint8;
    typedef signed char       sint8;
    typedef unsigned short    uint16;
    typedef signed short      sint16;
    typedef unsigned int      uint32;
    typedef signed int        sint32;
    typedef unsigned __int64  uint64;
    typedef signed __int64    sint64;
    #define POINTER_SIZE      sizeof(void*)
    #define THREAD_LOCAL      __declspec(thread)
#elif defined(linux) && defined(__i386__)  // g++ - 32 bit
    typedef unsigned int      nuint;
    typedef signed int        nsint;
    typedef unsigned char     uint8;
    typedef signed char       sint8;
    typedef unsigned short    uint16;
    typedef signed short      sint16;
    typedef unsigned int      uint32;
    typedef signed int        sint32;
    typedef unsigned long long uint64;
    typedef signed long long  sint64;
    #define POINTER_SIZE      sizeof(void*)
    #define THREAD_LOCAL      __thread
#elif defined(linux) && defined(__amd64__)  // g++ - 64 bit
    typedef unsigned long long nuint;
    typedef signed long long   nsint;
    typedef unsigned char     uint8;
    typedef signed char       sint8;
    typedef unsigned short    uint16;
    typedef signed short      sint16;
    typedef unsigned int      uint32;
    typedef signed int        sint32;
    typedef unsigned long long uint64;
    typedef signed long long  sint64;
    #define POINTER_SIZE      sizeof(void*)
    #define THREAD_LOCAL      __thread
#else
    #error Unknown platform in Types.h
#endif

//Compile-time verification of sizes
static_assert(sizeof(uint8)==1, "uint8 is defined incorrectly for the current platform");
static_assert(sizeof(sint8)==1, "sint8 is defined incorrectly for the current platform");
static_assert(sizeof(uint16)==2, "uint16 is defined incorrectly for the current platform");
static_assert(sizeof(sint16)==2, "sint16 is defined incorrectly for the current platform");
static_assert(sizeof(uint32)==4, "uint32 is defined incorrectly for the current platform");
static_assert(sizeof(sint32)==4, "sint32 is defined incorrectly for the current platform");
static_assert(sizeof(uint64)==8, "uint64 is defined incorrectly for the current platform");
static_assert(sizeof(sint64)==8, "sint64 is defined incorrectly for the current platform");

static_assert(sizeof(nsint)==sizeof(nuint), "nsint and nuint are different sizes");
