//tests that MemMan is thread-safe
//Luke Lenhart, 2007
//See /docs/License.txt for details on how this code may be used.

#include "../UnitTest.h"
#include <vector>
#include "base/Thread.h"
#include "base/Locks.h"
using namespace MPMA;

#ifdef _DEBUG
#ifdef DECLARE_TESTS_CODE
class MemManAllocFree: public UnitTest
{
    class SillyNestedNew
    {
        int i;
    };

    class VirtTestBaseClass
    {
    public:
        virtual void Rar()
        {
        }
        virtual ~VirtTestBaseClass()
        {
        }

        int x;
    };
    class VirtTestClass: public VirtTestBaseClass
    {
    public:
        virtual void Rar()
        {
            SillyNestedNew *a=new3(SillyNestedNew);
            delete3(a);
        }
        virtual ~VirtTestClass()
        {
            x=0;
        }
    };

public:
    bool Run()
    {
        bool good=true;

        {
            std::cout<<"Testing single int..."<<std::endl;

            mpmaMemoryManager.CheckForPaddingCorruption();
            int *a=new3(int);
            mpmaMemoryManager.CheckForPaddingCorruption();
            *a=7;
            mpmaMemoryManager.CheckForPaddingCorruption();

            if (!mpmaMemoryManager.IsPointerAnObject(a))
            {
                good=false;
                std::cout<<"Object should have been reported as valid."<<std::endl;
            }

            delete3(a);
            mpmaMemoryManager.CheckForPaddingCorruption();

            if (mpmaMemoryManager.IsPointerAnObject(a))
            {
                good=false;
                std::cout<<"Object should NOT have been reported as valid anymore."<<std::endl;
            }
        }

        {
            std::cout<<"Testing int array..."<<std::endl;

            mpmaMemoryManager.CheckForPaddingCorruption();
            int *a=new3_array(int,7);
            mpmaMemoryManager.CheckForPaddingCorruption();
            for (int i=0; i<7; ++i)
                a[i]=7;
            mpmaMemoryManager.CheckForPaddingCorruption();

            if (!mpmaMemoryManager.IsPointerAnObject(a))
            {
                good=false;
                std::cout<<"Array should have been reported as valid."<<std::endl;
            }

            delete3_array(a);
            mpmaMemoryManager.CheckForPaddingCorruption();

            if (mpmaMemoryManager.IsPointerAnObject(a))
            {
                good=false;
                std::cout<<"Array should NOT have been reported as valid anymore."<<std::endl;
            }
        }

        {
            std::cout<<"Testing virtual object..."<<std::endl;

            mpmaMemoryManager.CheckForPaddingCorruption();
            VirtTestClass *a=new3(VirtTestClass);
            mpmaMemoryManager.CheckForPaddingCorruption();
            a->x=7;
            mpmaMemoryManager.CheckForPaddingCorruption();

            if (!mpmaMemoryManager.IsPointerAnObject(a))
            {
                good=false;
                std::cout<<"Object should have been reported as valid."<<std::endl;
            }

            delete3(a);
            mpmaMemoryManager.CheckForPaddingCorruption();

            if (mpmaMemoryManager.IsPointerAnObject(a))
            {
                good=false;
                std::cout<<"Object should NOT have been reported as valid anymore."<<std::endl;
            }
        }

        {
            std::cout<<"Testing array of virtual objects..."<<std::endl;

            mpmaMemoryManager.CheckForPaddingCorruption();
            VirtTestClass *a=new3_array(VirtTestClass,7);
            mpmaMemoryManager.CheckForPaddingCorruption();
            for (int i=0; i<7; ++i)
                a[i].x=7;
            mpmaMemoryManager.CheckForPaddingCorruption();

            if (!mpmaMemoryManager.IsPointerAnObject(a))
            {
                good=false;
                std::cout<<"Array should have been reported as valid."<<std::endl;
            }

            delete3_array(a);
            mpmaMemoryManager.CheckForPaddingCorruption();

            if (mpmaMemoryManager.IsPointerAnObject(a))
            {
                good=false;
                std::cout<<"Array should NOT have been reported as valid anymore."<<std::endl;
            }
        }

        return good;
    }
};
#endif
DECLARE_TEST_CASE(MemManAllocFree)
#endif //_DEBUG
//

#ifdef DECLARE_TESTS_CODE
class MemManSafety: public UnitTest
{
private:
    static bool badness;

public:
    bool Run()
    {
        badness=false;

        //make 10 test threads
        std::vector<Thread*> threads;
        for (int i=0; i<10; ++i)
            threads.push_back(new2(Thread(UberThread, ThreadParam(0)),Thread));
        while (!threads.empty())
        {
            delete2(threads.back());
            threads.pop_back();
        }

        //
        return !badness;
    }

    //thread that does some alloc and frees
    static void UberThread(Thread &me, ThreadParam param)
    {
        //main test
        for (volatile int i=0; i<3000; ++i)
        {
            if (i%5==1 || i%5==4) //single
            {
                //alloc
                volatile int *a=new2(int,int);
                *a=i;

                //twiddle our thumbs
                ++i; ++i; ++i;
                --i; --i; --i;

                //check
                if (*a!=i)
                {
                    badness=true;
                    std::cout<<"My single int changed...!\n";
                    return;
                }

                delete2(a);
            }
            else //array
            {
                int num=((i+111)*33333)%100000;
                //alloc
                volatile int *a=new2_array(int,num,int);

                for (int c=0; c<num; ++c)
                    a[c]=i+c;

                //twiddle our thumbs
                ++i; ++i; ++i;
                --i; --i; --i;

                //check
                for (int c=0; c<num; ++c)
                {
                    if (a[c]!=i+c)
                    {
                        badness=true;
                        std::cout<<"My array int changed...!\n";
                        return;
                    }
                }

                delete2_array(a);
            }
        }
    }
};
bool MemManSafety::badness;
#endif
DECLARE_TEST_CASE(MemManSafety)

#ifdef DECLARE_TESTS_CODE
class MemManIntegrity: public UnitTest
{
private:
    struct TestObjectBase
    {
        virtual void Rar()
        {
        }
    };

    struct TestObject: TestObjectBase
    {
        int a;
        char foo[84];
        int b;

        TestObject(): a(0), b(0)
        {
            Rar();
        }
    };

public:
    bool Run()
    {
        bool good=true;

        std::cout<<"Test object is "<<sizeof(TestObject)<<" bytes."<<std::endl;

        TestObject *single=new2(TestObject,TestObject);
        if (single->a!=0 || single->b!=0)
        {
            std::cout<<"Single allocation corrupted object.  a and b should both be 0 but a="<<single->a<<" and b="<<single->b<<std::endl;
            good=false;
        }
        delete2(single);
        single=0;

        int numToAlloc=2;
        TestObject *array=new2_array(TestObject,numToAlloc,TestObject);
        std::cout<<"First 2 objects start "<<((char*)&array[1]-(char*)&array[0])<<" apart."<<std::endl;
        for (int i=0; i<numToAlloc; ++i)
        {
            if (array[i].a!=0 || array[i].b!=0)
            {
                std::cout<<"Array allocation corrupted object at index "<<i<<".  a and b should both be 0 but a="<<array[i].a<<" and b="<<array[i].b<<std::endl;
                good=false;
            }
        }
        delete2_array(array);
        array=0;

        return good;
    }
};
#endif
DECLARE_TEST_CASE(MemManIntegrity)
