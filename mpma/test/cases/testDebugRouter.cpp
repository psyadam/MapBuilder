//tests that the geo library functions work
//Luke Lenhart, 2007
//See /docs/License.txt for details on how this code may be used.

#include "../UnitTest.h"
#include "Setup.h"
#include "base/DebugRouter.h"
#include "base/MiscStuff.h"
#include <iostream>
#include <vector>
using namespace MPMA;

#ifdef DECLARE_TESTS_CODE
class FailerOutput: public RouterOutput
{
private:
    bool *pass;
    
public:
    FailerOutput(bool *passBool)
    {
        pass=passBool;
    }
    
    void Output(const uint8 *incoming, nuint incomingLen)
    {
        *pass=false;
        std::cout<<" -Unexpected output of "<<incomingLen<<" bytes.- "<<std::endl;
    }
};

class TestDebugRouter: public UnitTest
{
    class OutputBuffer: public RouterOutput
    {
    public:
        std::vector<uint8> data;
        MutexLock lock;
        
        void Output(const uint8 *incoming, nuint incomingLen)
        {
            TakeMutexLock mlock(lock);
            data.insert(data.end(), incoming, incoming+incomingLen);
        }
    };

public:
    bool Run()
    {
        bool pass=true;
        FailerOutput failOnError(&pass);
        ErrorReport().AddOutputMethod(&failOnError);
        
        float sizemod[4]={0.5f, 1.0f, 3.0f, 499.9f};
        for (int i=0; i<(int)(sizeof(sizemod)/sizeof(float)); ++i)
        {
            int bytes=(int)(sizemod[i]*DEBUGROUTER_BUFFER_SIZE);

            //alloc
            uint8 *tmp=new2_array(uint8,bytes,uint8);
            AutoDeleteArray<uint8> del(tmp);
            for (int b=0; b<bytes; ++b)
                tmp[b]=b%255;
            memcpy(tmp,"hello",6);
            
            std::cout<<"single output ("<<bytes<<" bytes)... "; std::cout.flush();
            {
                //setup and output
                OutputBuffer target;
                RouterInput rep;
                rep.AddOutputMethod(&target);
                if (sizemod[i]!=1.0f)
                {
                    rep<<"hello";
                    rep.Output(tmp+5, bytes-5);
                }
                else
                    rep.Output(tmp, bytes);
                
                //check
                Sleep(150); //give thread a chance to work
                TakeMutexLock mlock(target.lock);
                
                if (!CheckBuffer("", tmp, bytes, &target.data[0], (int)target.data.size())) return false;
                
                std::cout<<"ok"<<std::endl;
            }

            std::cout<<"single chain ("<<bytes<<" bytes)... "; std::cout.flush();
            {
                //setup and output
                OutputBuffer target;
                RouterInput rep1;
                RouterInput rep2;
                rep1.AddOutputMethod(&rep2);
                rep2.AddOutputMethod(&target);
                if (sizemod[i]!=1.0f)
                {
                    rep1<<"hello";
                    rep1.Output(tmp+5, bytes-5);
                }
                else
                    rep1.Output(tmp, bytes);
                
                //check
                Sleep(150); //give thread a chance to work
                TakeMutexLock mlock(target.lock);
                
                if (!CheckBuffer("", tmp, bytes, &target.data[0], (int)target.data.size())) return false;
                
                std::cout<<"ok"<<std::endl;
            }
            
            std::cout<<"complex chained output ("<<bytes<<" bytes)... "; std::cout.flush();
            {
                //setup and output
                OutputBuffer tar0, tar1;
                OutputBuffer tarMerge;
                RouterInput repRoot;
                RouterInput repA;
                RouterInput repB;
                RouterInput repB1, repB2;
                
                repRoot.AddOutputMethod(&repA);
                repRoot.AddOutputMethod(&repB);
                
                repB.AddOutputMethod(&repB1);
                repB.AddOutputMethod(&repB2);
                
                repA.AddOutputMethod(&tar0);
                repB1.AddOutputMethod(&tar1);
                repA.AddOutputMethod(&tarMerge);
                repB2.AddOutputMethod(&tarMerge);
                
                if (sizemod[i]!=1.0f)
                {
                    repRoot<<"hello";
                    repRoot.Output(tmp+5, bytes-5);
                }
                else
                    repRoot.Output(tmp, bytes);
                
                //check
                Sleep(250); //give threads a chance to work
                
                {
                    TakeMutexLock mlock0(tar0.lock);
                    if (!CheckBuffer("tar0", tmp, bytes, &tar0.data[0], (int)tar0.data.size())) return false;
                }
                {
                    TakeMutexLock mlock1(tar1.lock);
                    if (!CheckBuffer("tar1", tmp, bytes, &tar1.data[0], (int)tar1.data.size())) return false;
                }
                {
                    TakeMutexLock mlock2(tarMerge.lock);
                
                    if ((int)tarMerge.data.size()!=bytes*2)
                    {
                        std::cout<<"wrong number of bytes ended up merged output target: "<<tarMerge.data.size()<<std::endl;
                        return false;
                    }
                }
            
                std::cout<<"ok"<<std::endl;
            }
            
        }
        
        Sleep(100); //grace time for fail on error
        return pass;
    }
    
private:
    bool CheckBuffer(const std::string &name, uint8 *expect, int expectLen, uint8 *got, int gotLen)
    {
        if (gotLen!=expectLen)
        {
            std::cout<<"wrong number of bytes ended up in output target "<<name<<": "<<gotLen<<std::endl;
            return false;
        }
        
        if (memcmp(expect, got, gotLen)!=0)
        {
            std::cout<<"contents of output "<<name<<" doesn't match what was put in."<<std::endl;
            return false;
        }
        
        return true;
    }
};
#endif
DECLARE_TEST_CASE(TestDebugRouter)

#ifdef DECLARE_TESTS_CODE
class TestRouterInputFileOutput: public UnitTest
{
public:
    bool Run()
    {
        bool pass=true;
        FailerOutput failOnError(&pass);
        ErrorReport().AddOutputMethod(&failOnError);
        
        //clear out a temp file
        FILE *f=fopen(".tmp.txt","wb");
        if (!f)
        {
            std::cout<<"Failed to reset temp file."<<std::endl;
            return false;
        }
        
        fwrite("boo",4,1,f);
        fclose(f);
        
        //now route a reporter there and write something  avd verify (destroy reporter first)
        {
            RouterOutputFile rof(".tmp.txt");
            {
                RouterInput rep;
                rep.AddOutputMethod(&rof);
                rep<<"test1";
            }
        }
        
        std::string fileText=MISC::ReadFile(".tmp.txt");
        if (fileText!="test1")
        {
            std::cout<<"(pass1) Test file was expected to contain 'test1' but it contained: "+fileText<<std::endl;
            return false;
        }

        //now route a reporter there and write something  avd verify (destroy outputter first)
        {
            RouterInput rep;
            {
                RouterOutputFile rof(".tmp.txt");                
                rep.AddOutputMethod(&rof);
                rep<<"test2";
            }
        }
        
        fileText=MISC::ReadFile(".tmp.txt");
        if (fileText!="test2")
        {
            std::cout<<"(pass2) Test file was expected to contain 'test2' but it contained: "+fileText<<std::endl;
            return false;
        }
        
        Sleep(50); //grace time for fail on error
        return pass;
    }
};
#endif
DECLARE_TEST_CASE(TestRouterInputFileOutput)
