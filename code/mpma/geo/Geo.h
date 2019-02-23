//!\file Geo.h Vector and matrix primitives and operations.
//Luke Lenhart, 2007
//See /docs/License.txt for details on how this code may be used.

//all angles are in radians

#ifndef GEO_H_INCLUDED
#define GEO_H_INCLUDED

#include "../Config.h"

#ifdef MPMA_COMPILE_GEO

#include <cmath>
#include <string>

#include "../base/Types.h"
#include "GeoBases.h"

//!Geometry library
namespace GEO
{
    // -- global constants

    extern const float PI2; //!<2 PI
    extern const float PI; //!<PI
    extern const float FLOAT_TOLERANCE; //!<Acceptable range of closeness to count floats as equal


    // -- vector definitions

    //!A 4d vector or point.
    struct Vector4: public VectorN<4>
    {
        //ctors
        inline Vector4() {} //!<ctor
        inline Vector4(float x, float y, float z, float w) {elements[0]=x; elements[1]=y; elements[2]=z; elements[3]=w;} //!<ctor
        inline Vector4(const float *array): VectorN<4>(array) {} //!<ctor
        inline Vector4(float c): VectorN<4>(c) {} //!<ctor
        inline Vector4(int c): VectorN<4>((float)c) {} //!<ctor
        inline Vector4(const Vector4 &o): VectorN<4>((const float*)&o) {} //!<ctor
        inline Vector4(const VectorN<4> &o): VectorN<4>((const float*)&o) {} //!<ctor
        inline Vector4(const struct Vector3 &o, float w=1); //!<ctor

        //other accessors
        inline float x() const { return elements[0]; } //!<accessor
        inline float& x() { return elements[0]; } //!<accessor
        inline float y() const { return elements[1]; } //!<accessor
        inline float& y() { return elements[1]; } //!<accessor
        inline float z() const { return elements[2]; } //!<accessor
        inline float& z() { return elements[2]; } //!<accessor
        inline float w() const { return elements[3]; } //!<accessor
        inline float& w() { return elements[3]; } //!<accessor
    };


    //!A 3d vector or point.
    struct Vector3: public VectorN<3>
    {
        //ctors
        inline Vector3() {} //!<ctor
        inline Vector3(float x, float y, float z) {elements[0]=x; elements[1]=y; elements[2]=z;} //!<ctor
        inline Vector3(const float *array): VectorN<3>(array) {} //!<ctor
        inline Vector3(float c): VectorN<3>(c) {} //!<ctor
        inline Vector3(int c): VectorN<3>((float)c) {} //!<ctor
        inline Vector3(const Vector3 &o): VectorN<3>((const float*)&o) {} //!<ctor
        inline Vector3(const VectorN<3> &o): VectorN<3>((const float*)&o) {} //!<ctor
        explicit inline Vector3(const Vector4 &o); //!<ctor
        inline Vector3(const struct Vector2 &o, float z=1);

        //other accessors
        inline float x() const { return elements[0]; } //!<accessor
        inline float& x() { return elements[0]; } //!<accessor
        inline float y() const { return elements[1]; } //!<accessor
        inline float& y() { return elements[1]; } //!<accessor
        inline float z() const { return elements[2]; } //!<accessor
        inline float& z() { return elements[2]; } //!<accessor
    };

    //!A 2d vector or point.
    struct Vector2: public VectorN<2>
    {
        //ctors
        inline Vector2() {} //!<ctor
        inline Vector2(float x, float y) {elements[0]=x; elements[1]=y;} //!<ctor
        inline Vector2(const float *array): VectorN<2>(array) {} //!<ctor
        inline Vector2(float c): VectorN<2>(c) {} //!<ctor
        inline Vector2(int c): VectorN<2>((float)c) {} //!<ctor
        inline Vector2(const Vector2 &o): VectorN<2>((const float*)&o) {} //!<ctor
        inline Vector2(const VectorN<2> &o): VectorN<2>((const float*)&o) {} //!<ctor
        explicit inline Vector2(const Vector3 &o); //!<ctor

        //other accessors
        inline float x() const { return elements[0]; } //!<accessor
        inline float& x() { return elements[0]; } //!<accessor
        inline float y() const { return elements[1]; } //!<accessor
        inline float& y() { return elements[1]; } //!<accessor
    };

    //!A 1d value
    struct Vector1: public VectorN<1>
    {
        //ctors
        inline Vector1() {} //!<ctor
        inline Vector1(float x) {elements[0]=x;} //!<ctor
        inline Vector1(const float *array): VectorN<1>(array) {} //!<ctor
        inline Vector1(int c): VectorN<1>((float)c) {} //!<ctor
        inline Vector1(const Vector1 &o): VectorN<1>(o.x()) {} //!<ctor
        inline Vector1(const VectorN<1> &o): VectorN<1>(o.elements[0]) {} //!<ctor
        explicit inline Vector1(const Vector3 &o); //!<ctor

        //other accessors
        inline float x() const { return elements[0]; } //!<accessor
        inline float& x() { return elements[0]; } //!<accessor
    };

    // -- square matrix definitions

    //!A 4x4 matrix.
    struct Matrix4: public MatrixN<4>
    {
        //useful ctors
        inline Matrix4() {} //!<ctor
        inline Matrix4(const float *array): MatrixN<4>(array) {} //!<ctor
        inline Matrix4(float e0,float e1,float e2,float e3,float e4,float e5,float e6,float e7,float e8,float e9,float e10,float e11,float e12,float e13,float e14,float e15); //!<ctor
        inline Matrix4(float c): MatrixN<4>(c) {} //!<ctor
        inline Matrix4(int c): MatrixN<4>((float)c) {} //!<ctor
        inline Matrix4(const Matrix4 &o): MatrixN<4>((const float*)&o) {} //!<ctor
        inline Matrix4(const MatrixN<4> &o): MatrixN<4>((const float*)&o) {} //!<ctor
        inline Matrix4(const struct Matrix3 &o); //!<ctor

        //!Retrieves a pointer to this matrix that can be directly fed to OpenGL (which expects column-based). This function is NOT safe to use from multiple threads, since it uses a globally-shared data store as a backing.
        inline const float* ToGL() const;
    private:
        static Matrix4 globalGLConversionMatrix;
    };

    //!A 3x3 matrix.
    struct Matrix3: public MatrixN<3>
    {
        //useful ctors
        inline Matrix3() {} //!<ctor
        inline Matrix3(const float *array): MatrixN<3>(array) {} //!<ctor
        inline Matrix3(float e0,float e1,float e2,float e3,float e4,float e5,float e6,float e7,float e8); //!<ctor
        inline Matrix3(float c): MatrixN<3>(c) {} //!<ctor
        inline Matrix3(int c): MatrixN<3>((float)c) {} //!<ctor
        inline Matrix3(const Matrix3 &o): MatrixN<3>((const float*)&o) {} //!<ctor
        inline Matrix3(const MatrixN<3> &o): MatrixN<3>((const float*)&o) {} //!<ctor
        explicit inline Matrix3(const struct Matrix4 &o); //!<ctor
        inline Matrix3(const struct Matrix2 &o); //!<ctor
    };

    //!A 2x2 matrix.
    struct Matrix2: public MatrixN<2>
    {
        //useful ctors
        inline Matrix2() {} //!<ctor
        inline Matrix2(const float *array): MatrixN<2>(array) {} //!<ctor
        inline Matrix2(float e0,float e1,float e2,float e3); //!<ctor
        inline Matrix2(float c): MatrixN<2>(c) {} //!<ctor
        inline Matrix2(int c): MatrixN<2>((float)c) {} //!<ctor
        inline Matrix2(const Matrix2 &o): MatrixN<2>((const float*)&o) {} //!<ctor
        inline Matrix2(const MatrixN<2> &o): MatrixN<2>((const float*)&o) {} //!<ctor
        explicit inline Matrix2(const Matrix3 &o); //!<ctor
    };


    // -- quaternion definition

    //TODO


    // -- generic utilities

    inline bool NearEqual(float c0, float c1, float tolerance=FLOAT_TOLERANCE); //!<compare elements for "near equal"
    template <typename VecType>
    inline bool NearEqual(const VecType& v1, const VecType& v2, float tolerance=FLOAT_TOLERANCE); //!<compare elements for "near equal"
    template <typename VecType>
    inline bool NearEqual(const VecType& v, float c, float tolerance=FLOAT_TOLERANCE); //!<compare elements for "near equal"

    inline bool IsFinite(float c); //!<return true if a float is valid
    template <typename VecType>
    inline bool IsFinite(VecType &v); //!<returns true if all elements are valid floats

    template <typename VecType>
    inline VecType Clamp(const VecType &val, const VecType &min, const VecType &max); //!<component-wise clamps val between min and max and returns the result
    template <>
    inline float Clamp(const float &val, const float &min, const float &max); //!<clamps val between min and max and returns the result

    // -- Vector functions

    template <typename VecType1, typename VecType2>
    inline float VecDot(const VecType1 &v1, const VecType2 &v2); //!<dot product

    inline struct Vector3 VecCross(const struct Vector3 &v1, const struct Vector3 &v2); //!<cross product

    template <typename VecType>
    inline VecType VecNormal(const VecType &v); //!<get the normalized version of a vector
    template <typename VecType>
    inline void VecNormalize(VecType &v); //!<normalize a vector

    template <typename VecType1, typename VecType2>
    inline float VecAngleBetween(const VecType1 &l1, const VecType2 &l2); //!<angle between vectors

    template <typename VecType1, typename VecType2>
    inline float VecDistance(const VecType1 &l1, const VecType2 &l2); //!<distance between two points

    template <typename VecType>
    inline float VecLength(const VecType &v); //<!length of a vector
    template <typename VecType>
    inline float VecLengthSquared(const VecType &v); //<!squared length of a vector

    template <typename VecType1, typename VecType2>
    inline VecType1 VecReflect(const VecType1 &vec, const VecType2 &norm); //!<reflect a vector off of a normal


    // -- Matrix functions

    template <typename MatType>
    MatType MatMul(const MatType &m1, const MatType &m2); //!<multiply two matrices

    float MatDeterminant(const Matrix4 &mat); //!<finds the determinant of a matrix
    float MatDeterminant(const Matrix3 &mat); //!<finds the determinant of a matrix
    inline float MatDeterminant(const Matrix2 &mat); //!<finds the determinant of a matrix

    template <typename MatType>
    MatType MatTranspose(const MatType &mat); //!<finds the transpose of a matrix

    template <typename MatType>
    MatType MatInverse(const MatType &mat); //!<finds the inverse of a matrix (undefined behaviour if inverse does not exist)

    inline Matrix3 MatRotateX(float angle); //!<creates a matrix that rotates around the x axis
    inline Matrix3 MatRotateY(float angle); //!<creates a matrix that rotates around the y axis
    inline Matrix3 MatRotateZ(float angle); //!<creates a matrix that rotates around the z axis
    inline Matrix3 MatRotateAxis(const Vector3 &axis, float angle); //!<creates a matrix that rotates around an axis

    template <typename VecType>
    inline typename VecType::GreaterMatrixType MatTranslate(const VecType &offset); //!<builds a translation matrix

    template <typename VecType>
    inline typename VecType::GreaterMatrixType MatScale(const VecType &scale); //!<builds a scaling matrix

    Matrix4 MatProjectionFoV(float fov=PI/2, float aspectYdivX=1.0f, float nearZ=1.0f, float farZ=1000.0f); //!<constructs a perspective projection transform

    Matrix4 MatProjectionOrtho(float left, float right, float bottom, float top, float nearZ=1.0f, float farZ=1000.0f); //!<constructs a parallel projection transform
    inline Matrix4 MatProjectionOrtho(float nearZ=1.0f, float farZ=1000.0f); //!<constructs a parallel projection transform (from -1 to 1 on the x and y axis)

    Matrix4 MatViewLookAt(const Vector3 &pos, const Vector3 &tar, const Vector3 &vagueUp=Vector3(0,0,1), bool flipVert=false, bool flipHorz=false); //!<constructs a look-at-based left-handed view transformation


    // -- Mixed functions

    template <typename VecType>
    inline VecType TransformVector(const typename VecType::MatrixType &m, const VecType &v); //!<multiply a matrix by a vector

}; //namespace GEO

//include template and inline implementations
#ifndef GEO_INCLUDE_INLINE
    #define GEO_INCLUDE_INLINE
    #include "Geo.cpp"
#endif

#endif //#ifdef MPMA_COMPILE_GEO

#endif // GEO_H_INCLUDED