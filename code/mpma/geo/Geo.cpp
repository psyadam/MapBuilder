//Vector and matrix primitives and operations.
//Luke Lenhart, 2007
//See /docs/License.txt for details on how this code may be used.

#include "../Config.h"

#ifdef MPMA_COMPILE_GEO

#ifndef GEO_INCLUDE_INLINE // ------------ normal compiled section -----------

#include "Geo.h"

namespace GEO
{
    //global constants
    const float PI2=6.2831853071f;
    const float PI=3.14159265f;

    const float FLOAT_TOLERANCE=0.00001f; //float accuracy that we care about

    //global statics
    Matrix4 Matrix4::globalGLConversionMatrix;

    // --

    //finds the determinant of a matrix
    float MatDeterminant(const Matrix4 &m)
    {
        return
            m[0][0]*MatDeterminant(Matrix3(m[1][1],m[1][2],m[1][3], m[2][1],m[2][2],m[2][3], m[3][1],m[3][2],m[3][3])) -
            m[0][1]*MatDeterminant(Matrix3(m[1][0],m[1][2],m[1][3], m[2][0],m[2][2],m[2][3], m[3][0],m[3][2],m[3][3])) +
            m[0][2]*MatDeterminant(Matrix3(m[1][0],m[1][1],m[1][3], m[2][0],m[2][1],m[2][3], m[3][0],m[3][1],m[3][3])) -
            m[0][3]*MatDeterminant(Matrix3(m[1][0],m[1][1],m[1][2], m[2][0],m[2][1],m[2][2], m[3][0],m[3][1],m[3][2]));
    }

    float MatDeterminant(const Matrix3 &m)
    {
        return
            m[0][0]*MatDeterminant(Matrix2(m[1][1],m[1][2],m[2][1],m[2][2])) -
            m[0][1]*MatDeterminant(Matrix2(m[1][0],m[1][2],m[2][0],m[2][2])) +
            m[0][2]*MatDeterminant(Matrix2(m[1][0],m[1][1],m[2][0],m[2][1]));
    }

    //constructs a perspective projection transform
    Matrix4 MatProjectionFoV(float fov, float aspectYdivX, float nearZ, float farZ)
    {
        float yScale=1.0f/std::tan(fov/2.0f);
        Matrix4 ret(aspectYdivX*yScale, 0,      0,                         0,
                    0,                  yScale, 0,                         0,
                    0,                  0,      (farZ+nearZ)/(nearZ-farZ), (2*farZ*nearZ)/(nearZ-farZ),
                    0,                  0,      -1,                        0);
        return ret;
    }

    //constructs a parallel projection transform
    Matrix4 MatProjectionOrtho(float left, float right, float bottom, float top, float nearZ, float farZ)
    {
        Matrix4 ret(2/(right-left), 0,              0,               -(right+left)/(right-left),
                    0,              2/(top-bottom), 0,               -(top+bottom)/(top-bottom),
                    0,              0,              -2/(farZ-nearZ), -(farZ+nearZ)/(farZ-nearZ),
                    0,              0,              0,               1);
        return ret;
    }

    //constructs a look-at-based left-handed view transform
    Matrix4 MatViewLookAt(const Vector3 &pos, const Vector3 &tar, const Vector3 &vagueUp, bool flipVert, bool flipHorz)
    {
        //calc the 3 axises
        Vector3 front=tar-pos;
        front*=-1.0f;
        VecNormalize(front);
        Vector3 right=VecCross(vagueUp,front);
        VecNormalize(right);
        if (flipHorz) right*=-1.0f;
        Vector3 up=VecCross(front,right);
        if (flipVert) up*=-1.0f;

        //build matrix
        Matrix4 ret;
        ret.Row(0)=Vector4(right, -VecDot(right,pos));
        ret.Row(1)=Vector4(up,    -VecDot(up,   pos));
        ret.Row(2)=Vector4(front, -VecDot(front,pos));
        ret.Row(3)=Vector4(0, 0, 0, 1);

        return ret;
    }

}; //namespace GEO


// ----------------- end normal compile section ----------------
#else // -------------- start template and inline section -----------------


#ifndef GEO_CPP_TEMPLATES_INCLUDED
#define GEO_CPP_TEMPLATES_INCLUDED

#include "../base/Vary.h"

#if defined(_WIN32) || defined(_WIN64)  //windows needs float.h for _finite
    #include <float.h>
#endif

#ifdef _DEBUG //for reporting errors in debug builds
    #include "../base/Debug.h"
    #include "../base/DebugRouter.h"
#endif

namespace GEO
{
    // -- generic utilities

    //compare two entities or all elements in one
    inline bool NearEqual(float c0, float c1, float tolerance)
    {
        if (fabs(c1-c0)>tolerance)
            return false;
        else
            return true;
    }
    template <typename VecType>
    inline bool NearEqual(const VecType &v1, const VecType &v2, float tolerance)
    {
        for (nuint i=0; i<v1.ElementCount(); ++i)
        {
            if (fabs(v1[i]-v2[i])>tolerance)
                return false;
        }
        return true;
    }

    template <typename VecType>
    inline bool NearEqual(const VecType &v, float c, float tolerance)
    {
        for (nuint i=0; i<v.ElementCount(); ++i)
        {
            if (fabs(v[i]-c)>tolerance)
                return false;
        }
        return true;
    }

    //return true if float is valid
    inline bool IsFinite(float c)
    {
        #if defined(_WIN32) || defined(_WIN64) //windows calls it _finite.. grr.
            if (!_finite(c)) return false;
        #else
        if (!finite(c)) return false;
        #endif
        return true;
    }
    
    //returns true if all elements are valid floats
    template <typename VecType>
    inline bool IsFinite(VecType &v)
    {
        for (nuint i=0;i<v.ElementCount();++i)
        {
            if (!IsFinite(v[i]))
                return false;
        }
        return true;
    }

    //component-wise clamps val between min and max and returns the result
    template <typename VecType>
    inline VecType Clamp(const VecType &val, const VecType &min, const VecType &max)
    {
        VecType clampedVal;
        for (nuint i=0; i<val.ElementCount(); ++i)
        {
            clampedVal[i]=Clamp(val[i], min, max);
            
        }
        return clampedVal;
    }

    //clamps val between min and max and returns the result
    inline float Clamp(const float &val, const float &min, const float &max)
    {
        float clampedVal;
        if (val<min)
            clampedVal=min;
        else if (val>max)
            clampedVal=max;
        else
            clampedVal=val;
        return clampedVal;
    }

    // -- vector constructions
    
    inline Vector4::Vector4(const Vector3 &o, float w)
    {
        x()=o.x();
        y()=o.y();
        z()=o.z();
        this->w()=w;
    }

    inline Vector3::Vector3(const Vector4 &o)
    {
        x()=o.x();
        y()=o.y();
        z()=o.z();
    }

    inline Vector3::Vector3(const Vector2 &o, float z)
    {
        x()=o.x();
        y()=o.y();
        this->z()=z;
    }

    inline Vector2::Vector2(const Vector3 &o)
    {
        x()=o.x();
        y()=o.y();
    }


    // -- matrix constructions
    
    inline Matrix4::Matrix4(float e0,float e1,float e2,float e3,float e4,float e5,float e6,float e7,float e8,float e9,float e10,float e11,float e12,float e13,float e14,float e15)
    {
        elements[0]=e0;   elements[1]=e1;   elements[2]=e2;   elements[3]=e3;
        elements[4]=e4;   elements[5]=e5;   elements[6]=e6;   elements[7]=e7;
        elements[8]=e8;   elements[9]=e9;   elements[10]=e10; elements[11]=e11;
        elements[12]=e12; elements[13]=e13; elements[14]=e14; elements[15]=e15;
    }

    inline Matrix3::Matrix3(float e0,float e1,float e2,float e3,float e4,float e5,float e6,float e7,float e8)
    {
        elements[0]=e0; elements[1]=e1; elements[2]=e2;
        elements[3]=e3; elements[4]=e4; elements[5]=e5;
        elements[6]=e6; elements[7]=e7; elements[8]=e8;
    }
    
    inline Matrix2::Matrix2(float e0,float e1,float e2,float e3)
    {
        elements[0]=e0; elements[1]=e1;
        elements[2]=e2; elements[3]=e3;
    }

    inline Matrix4::Matrix4(const Matrix3 &o)
    {
        //copy 3x3 portion
        for (nuint r=0; r<o.RowColCount(); ++r)
        {
            for (nuint c=0; c<o.RowColCount(); ++c)
            {
                rowcol[r][c]=o.rowcol[r][c];
            }
        }
        //fill rest with 0, except bottom-right which gets a 1
        for (nuint i=0; i<RowColCount()-1; ++i)
        {
            rowcol[i][RowColCount()-1]=0;
            rowcol[RowColCount()-1][i]=0;
        }
        rowcol[RowColCount()-1][RowColCount()-1]=1;
    }

    inline Matrix3::Matrix3(const Matrix4 &o)
    {
        for (nuint r=0; r<RowColCount(); ++r)
        {
            for (nuint c=0; c<RowColCount(); ++c)
            {
                rowcol[r][c]=o.rowcol[r][c];
            }
        }        
    }

    inline Matrix3::Matrix3(const Matrix2 &o)
    {
        //copy 2x2 portion
        for (nuint r=0; r<o.RowColCount(); ++r)
        {
            for (nuint c=0; c<o.RowColCount(); ++c)
            {
                rowcol[r][c]=o.rowcol[r][c];
            }
        }
        //fill rest with 0, except bottom-right which gets a 1
        for (nuint i=0; i<RowColCount()-1; ++i)
        {
            rowcol[i][RowColCount()-1]=0;
            rowcol[RowColCount()-1][i]=0;
        }
        rowcol[RowColCount()-1][RowColCount()-1]=1;
    }

    inline Matrix2::Matrix2(const Matrix3 &o)
    {
        for (nuint r=0; r<RowColCount(); ++r)
        {
            for (nuint c=0; c<RowColCount(); ++c)
            {
                rowcol[r][c]=o.rowcol[r][c];
            }
        }
    }

    
    // -- Vector functions --
    
    //dot product
    template <typename VecType1, typename VecType2>
    inline float VecDot(const VecType1 &v1, const VecType2 &v2)
    {
        float c=v1[0]*v2[0];
        for (nuint i=1;i<v1.ElementCount();++i)
            c+=v1[i]*v2[i];
        return c;
    }
    
    //cross product
    inline Vector3 VecCross(const Vector3 &v1, const Vector3 &v2)
    {
        Vector3 r;
        r.x()=v1.y()*v2.z() - v1.z()*v2.y();
        r.y()=v1.z()*v2.x() - v1.x()*v2.z();
        r.z()=v1.x()*v2.y() - v1.y()*v2.x();
        return r;
    }

    //vector normalization
    template <typename VecType>
    inline VecType VecNormal(const VecType &v)
    {
        float len=v.Length();
#ifdef _DEBUG
        if (fabs(len)<FLOAT_TOLERANCE)
            MPMA::ErrorReport()<<"Divide by 0 while normalizing a vector.  Call stack:\n"<<MPMA::GetCallStack()<<"\n";
#endif
        return v/len;
    }

    template <typename VecType>
    inline void VecNormalize(VecType &v)
    {
        float len=v.Length();
#ifdef _DEBUG
        if (fabs(len)<FLOAT_TOLERANCE)
            MPMA::ErrorReport()<<"Divide by 0 while normalizing a vector.  Call stack:\n"<<MPMA::GetCallStack()<<"\n";
#endif
        v/=len;
    }

    //angle between
    template <typename VecType1, typename VecType2>
    inline float VecAngleBetween(const VecType1 &v0, const VecType2 &v1)
    {
        float mulledLens=v0.Length()*v1.Length();
#ifdef _DEBUG
        if (fabs(mulledLens)<FLOAT_TOLERANCE)
            MPMA::ErrorReport()<<"Divide by 0 while taking the angle between vectors.  Call stack:\n"<<MPMA::GetCallStack()<<"\n";
#endif
        float val=VecDot(v0, v1);
        val/=mulledLens;

        if(val<-1.0f)
            val=-1.0f;
        else if(val>1.0f)
            val=1.0f;

        return std::acos(val);
    }

    //distance
    template <typename VecType1, typename VecType2>
    inline float VecDistance(const VecType1 &v0, const VecType2 &v1)
    {
        VecType1 diff=v1-v0;
        return sqrt(VecDot(diff,diff));
    }
    
    //length
    template <typename VecType>
    inline float VecLength(const VecType &v)
    {
        return sqrt(VecDot(v, v));
    }

    //length squared
    template <typename VecType>
    inline float VecLengthSquared(const VecType &v)
    {
        return VecDot(v, v);
    }
    
    //reflect a vector off a normal
    template <typename VecType1, typename VecType2>
    inline VecType1 VecReflect(const VecType1 &vec, const VecType2 &norm)
    {
        return vec - 2.0f*VecDot(vec,norm)*norm;
    }
    
    
    // -- Matrix functions --
    
    //multiply 2 matrices
    template <typename MatType>
    MatType MatMul(const MatType &m1, const MatType &m2)
    {
        MatType ret=0.0f;

        for (nuint dr=0; dr<m1.RowColCount(); dr++)
        {
            for (nuint dc=0; dc<m1.RowColCount(); dc++)
            {
                for (nuint s=0; s<m1.RowColCount(); ++s)
                {
                    ret[dr][dc]+=m1[dr][s]*m2[s][dc];
                }
            }
        }

        return ret;
    }

    //finds the determinant of a matrix
    inline float MatDeterminant(const Matrix2 &mat)
    {
        return mat[0][0]*mat[1][1] - mat[1][0]*mat[0][1];
    }
    
    //finds the transpose of a matrix
    template <typename MatType>
    MatType MatTranspose(const MatType &mat)
    {
        MatType ret;

        //copy swapped rows and cols
        for (nuint r=1; r<mat.RowColCount(); ++r)
        {
            for (nuint c=0; c<r; ++c)
            {
                ret[r][c]=mat[c][r];
                ret[c][r]=mat[r][c];
            }
        }

        //copy diagonal in
        for (nuint i=0; i<mat.RowColCount(); ++i)
            ret[i][i]=mat[i][i];

        return ret;
    }
    
    //finds the inverse of a matrix using the row reduction algorithm
    template <typename MatType>
    MatType MatInverse(const MatType &mat)
    {
#ifdef _DEBUG
        if (std::fabs(MatDeterminant(mat))<FLOAT_TOLERANCE)
        {
            MPMA::ErrorReport()<<"Taking the inverse of a matrix whose determinant is very near 0.  It may not have an inverse.  Call stack:\n"<<MPMA::GetCallStack()<<"\n";
        }
#endif

        MatType org=mat; //working matrix from the original, which will become the identity
        MatType inv; //identity matrix, which will become the inverse
        inv.MakeIdentity();
        
        //row reduce into an upper triangular matrix with 1's on the diagonal
        for (nuint c=0; c<mat.RowColCount(); ++c)
        {
            //get a non-0 value into our diagonal spot if we don't have one
            nuint dvfrow=c+1;
            while (std::fabs(org.Row(c)[c])<FLOAT_TOLERANCE && dvfrow<mat.RowColCount())
            {
                org.Row(c)+=org.Row(dvfrow);
                inv.Row(c)+=inv.Row(dvfrow);
                ++dvfrow;
            }

            //scale this row so that the spot (row,col) is 1
            inv.Row(c)/=org.Row(c)[c];
            org.Row(c)/=org.Row(c)[c];

            //now zero out this column in the rest of the rows
            for (nuint rw=c+1; rw<mat.RowColCount(); ++rw)
            {
                if (std::fabs(org.Row(rw)[c])>FLOAT_TOLERANCE)
                {
                    inv.Row(rw)-=inv.Row(c)*org.Row(rw)[c];
                    org.Row(rw)-=org.Row(c)*org.Row(rw)[c];
                }
            }
        }

        //row reduce the upper triangle into zeros
        for (nuint rc=0; rc<mat.RowColCount()-1; ++rc)
        {
            for (nuint sr=rc+1; sr<mat.RowColCount(); ++sr)
            {
                inv.Row(rc)-=inv.Row(sr)*org.Row(rc)[sr];
                org.Row(rc)-=org.Row(sr)*org.Row(rc)[sr];
            }
        }

        return inv;
    }

    //builds a rotation around a specific axis
    inline Matrix3 MatRotateX(float angle)
    {
        float s=sin(angle), c=cos(angle);
        return Matrix3(1, 0,  0,
                       0, c,  s,
                       0, -s, c);
    }
    inline Matrix3 MatRotateY(float angle)
    {
        float s=sin(angle), c=cos(angle);
        return Matrix3(c, 0, -s,
                       0, 1, 0,
                       s, 0, c);
    }
    inline Matrix3 MatRotateZ(float angle)
    {
        float s=sin(angle), c=cos(angle);
        return Matrix3(c,  s, 0,
                       -s, c, 0,
                       0,  0, 1);
    }

    //builds a rotation around an arbitrary axis
    inline Matrix3 MatRotateAxis(const Vector3 &axis, float angle)
    {
#ifdef _DEBUG
        if (fabs(axis[0])<FLOAT_TOLERANCE && fabs(axis[1])<FLOAT_TOLERANCE && fabs(axis[2])<FLOAT_TOLERANCE)
        {
            MPMA::ErrorReport()<<"Attempt to use Rotate around a non-axis.  Call stack:\n"<<MPMA::GetCallStack()<<"\n";
        }
#endif

        float c=cos(angle), s=sin(angle);
        float t=1.0f-c;
        GEO::Vector3 naxis=axis.Normal();
        float &x=naxis.x(), &y=naxis.y(), &z=naxis.z();

        return Matrix3(t*x*x+c,   t*x*y-s*z, t*x*z+s*y,
                       t*x*y+s*z, t*y*y+c,   t*y*z-s*x,
                       t*x*z-s*y, t*y*z+s*x, t*z*z+c);
    }
    
    //builds a translation matrix
    template <typename VecType>
    inline typename VecType::GreaterMatrixType MatTranslate(const VecType &offset)
    {
        typename VecType::GreaterMatrixType mat;
        mat.MakeIdentity();
        mat.SetColumn(mat.RowColCount()-1, offset);
        return mat;
    }

    template <typename VecType>
    inline typename VecType::GreaterMatrixType MatScale(const VecType &scale)
    {
        typename VecType::GreaterMatrixType mat;
        mat.MakeIdentity();
        for (nuint d=0; d<scale.ElementCount(); ++d)
            mat[d][d]=scale[d];
        return mat;
    }
    
    //constructs a parallel projection transform (from -1 to 1 on the x/y axis)
    inline Matrix4 MatProjectionOrtho(float nearZ, float farZ)
    {
        return MatProjectionOrtho(-1, 1, -1, 1, nearZ, farZ);
    }

    //Retrieves a pointer to a Matrix4 that can be directly fed to OpenGL (which expects column-based), using the global static Matrix4.
    inline const float* Matrix4::ToGL() const
    {
        globalGLConversionMatrix=(*this).Transpose();
        return globalGLConversionMatrix.elements;
    }
    
    // -- Mixed functions
    
    //multiply a matrix by a vector
    template <typename VecType>
    inline VecType TransformVector(const typename VecType::MatrixType &m, const VecType &v)
    {
        VecType ret=0.0f;

        for (nuint r=0; r<m.RowColCount(); ++r)
        {
            for (nuint e=0; e<v.ElementCount(); ++e)
            {
                ret[r]+=m[r][e]*v[e];
            }
        }

        return ret;
    }

} //namespace GEO


#endif // GEO_CPP_TEMPLATES_INCLUDED

#endif // ------------- end template and inline section -----------

#endif //#ifdef MPMA_COMPILE_GEO
