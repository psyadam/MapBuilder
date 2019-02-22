//tests that the geo library functions work
//Luke Lenhart, 2007
//See /docs/License.txt for details on how this code may be used.

#include "../UnitTest.h"
#include <iostream>
#include "geo/Geo.h"
#include "base/Vary.h"
using namespace MPMA;

#ifdef DECLARE_TESTS_CODE
#define SpewUnit(unit) std::cout<<""#unit<<"\n:"<<(const std::string&)Vary(unit)<<"\n";
template <typename UnitType>
class GeoBasics: public UnitTest
{
public:
    bool Run()
    {
        bool good=true;
        
        //byte paction
        if (sizeof(UnitType)!=UnitType().ElementCount()*4)
        {
            std::cout<<"Number of bytes is wrong.\n";
            return false;
        }
        
        //construction and comparison
        {
            UnitType u0;
            UnitType u1(9.0f);
            UnitType u2(9);
            for (nuint i=0; i<u0.ElementCount(); ++i)
                u0[i]=9.0f;
            UnitType u3(u0);
            if ((!(u0==u1)) || (u2!=u1) || u3!=u2 || !(u1==9.0f))
            {
                std::cout<<"Construct and compare failed.\n";
                good=false;
            }
            
            if (!GEO::IsFinite(u3))
            {
                std::cout<<"Not finite.\n";
                good=false;
            }
        }
        
        //add
        UnitType unitThree(3.0f);
        {
            UnitType u0(10.5f);
            UnitType u1(13.5f);
            UnitType u2(u0+unitThree);
            UnitType u3(u0+3.0f);
            UnitType u4(u0+3);
            UnitType u5=u0; u5+=unitThree;
            UnitType u6=u0; u6+=3.0f;
            UnitType u7=u0; u7+=3;
            if (u1!=u2 || u2!=u3 || u3!=u4 || u4!=u5 || u5!=u6 || u6!=u7)
            {
                std::cout<<"Add failed.\n";
                SpewUnit(u0); SpewUnit(u1); SpewUnit(u2); SpewUnit(u3);
                SpewUnit(u4); SpewUnit(u5); SpewUnit(u6); SpewUnit(u7);
                good=false;
            }
        }
        
        //sub
        {
            UnitType u0(10.5f);
            UnitType u1(7.5f);
            UnitType u2(u0-unitThree);
            UnitType u3(u0-3.0f);
            UnitType u4(u0-3);
            UnitType u5=u0; u5-=unitThree;
            UnitType u6=u0; u6-=3.0f;
            UnitType u7=u0; u7-=3;
            if (u1!=u2 || u2!=u3 || u3!=u4 || u4!=u5 || u5!=u6 || u6!=u7)
            {
                std::cout<<"Subtract failed.\n";
                SpewUnit(u0); SpewUnit(u1); SpewUnit(u2); SpewUnit(u3);
                SpewUnit(u4); SpewUnit(u5); SpewUnit(u6); SpewUnit(u7);
                good=false;
            }
        }
        
        //scale
        {
            UnitType u0(9.9f);
            UnitType u1(29.7f);
            UnitType u2(u0*3.0f);
            UnitType u3(u0*3);
            UnitType u4=u0; u4*=3.0f;
            UnitType u5=u0; u5*=3;
            if (u1!=u2 || u2!=u3 || u3!=u4 || u4!=u5)
            {
                std::cout<<"Scale(mul) failed.\n";
                SpewUnit(u0); SpewUnit(u1); SpewUnit(u2);
                SpewUnit(u3); SpewUnit(u4); SpewUnit(u5);
                good=false;
            }
        }
        
        //descale
        {
            UnitType u0(9.9f);
            UnitType u1(3.3f);
            UnitType u2(u0/3.0f);
            UnitType u3(u0/3);
            UnitType u4=u0; u4/=3.0f;
            UnitType u5=u0; u5/=3;
            if (u1!=u2 || u2!=u3 || u3!=u4 || u4!=u5)
            {
                std::cout<<"Descale(div) failed.\n";
                SpewUnit(u0); SpewUnit(u1); SpewUnit(u2);
                SpewUnit(u3); SpewUnit(u4); SpewUnit(u5);
                good=false;
            }
        }
        
        //negate
        {
            UnitType u0(9.9f);
            UnitType u1(-9.9f);
            UnitType u2=-u0;
            UnitType u3=u0;
            u3=-u3;
            if (u1!=u2 || u2!=u3)
            {
                std::cout<<"Negate failed.\n";
                SpewUnit(u0); SpewUnit(u1); SpewUnit(u2); SpewUnit(u3);
                good=false;
            }
        }
        
        //finite
        {
            UnitType u0=1;
            UnitType u1=u0/0.0f;
            UnitType u2=-u0/0.0f;
            if (!GEO::IsFinite(u0) || GEO::IsFinite(u1) || GEO::IsFinite(u2))
            {
                std::cout<<"IsFinite failed.\n";
                good=false;
            }
        }
        
        return good;
    }
};
#endif
DECLARE_TEST_CASE(GeoBasics<GEO::Vector2>);
DECLARE_TEST_CASE(GeoBasics<GEO::Vector3>);
DECLARE_TEST_CASE(GeoBasics<GEO::Vector4>);
DECLARE_TEST_CASE(GeoBasics<GEO::Matrix2>);
DECLARE_TEST_CASE(GeoBasics<GEO::Matrix3>);
DECLARE_TEST_CASE(GeoBasics<GEO::Matrix4>);

#ifdef DECLARE_TESTS_CODE
class GeoMatrixOps: public UnitTest
{
public:
    bool Run()
    {
        bool good=true;
        
        {
            std::cout<<"Matrix Multiply..."; std::cout.flush();
            GEO::Matrix3 m1(1,0,2,
                            0,1,0,
                            0,0,1);
            GEO::Matrix3 m2(3,0,1,
                            0,1,0,
                            1,0,0);
            GEO::Matrix3 m3=m1*m2;
            GEO::Matrix3 mexpect(5,0,1,
                                 0,1,0,
                                 1,0,0);
            if (m3!=mexpect)
            {
                good=false;
                std::cout<<"fail"<<std::endl;
                std::cout<<"expect\n"<<(const std::string&)mexpect<<"\n";
                std::cout<<"got\n"<<(const std::string&)m3<<std::endl;
            }
            else
                std::cout<<"ok"<<std::endl;
        }

        {
            std::cout<<"MatRotateAxis..."; std::cout.flush();
            GEO::Matrix3 mrot=GEO::MatRotateAxis(GEO::Vector3(0,0,1),0);
            GEO::Matrix3 mexpect(1,0,0,
                                 0,1,0,
                                 0,0,1);
            if (mrot!=mexpect)
            {
                good=false;
                std::cout<<"fail"<<std::endl;
            }
            else
                std::cout<<"ok"<<std::endl;
        }
        
        {
            //GEO::Matrix4 mtrans=
            GEO::MatTranslate(GEO::Vector3(1,2,3));
            //TODO:
        }
        
        {
            std::cout<<"MatInverse..."; std::cout.flush();
            GEO::Matrix3 morig(1,2,4,
                               4,6,1,
                               1,2,3);
            GEO::Matrix3 minv=GEO::MatInverse(morig);
            GEO::Matrix3 mexpect(8,    1,    -11,
                                 -5.5f,-0.5f,7.5f,
                                 1,    0,    -1);
            if (minv!=mexpect)
            {
                good=false;
                std::cout<<"fail"<<std::endl;
                std::cout<<"expect\n"<<(const std::string&)mexpect<<"\n";
                std::cout<<"got\n"<<(const std::string&)minv<<std::endl;
            }
            else
                std::cout<<"ok"<<std::endl;
        }
        
        return good;
    }
};
#endif
DECLARE_TEST_CASE(GeoMatrixOps);

//This isn't a test case.  This is just a use of every memeber of every construct to force template expansion to catch compile errors.
#ifdef DECLARE_TESTS_CODE
class ForceGeoTemplateExpansion
{
    ForceGeoTemplateExpansion()
    {
        GEO::Vector4 v4;
        GEO::Vector3 v3;
        GEO::Matrix4 m4;
        GEO::Matrix3 m3;
        
        GEO::NearEqual(v4,v4);
        GEO::NearEqual(m4,1.0f);
        GEO::IsFinite(v4);
        GEO::VecDot(v4,v4);
        GEO::VecNormal(v4);
        GEO::VecNormalize(v4);
        GEO::VecAngleBetween(v4,v4);
        GEO::VecReflect(v4,v4);
        GEO::MatMul(m4,m4);
        GEO::MatTranspose(m4);
        GEO::MatInverse(m4);
        GEO::MatTranslate(v3);
        GEO::TransformVector(m4,v4);
    }
};
#endif
