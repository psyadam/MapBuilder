//!\file GeoBases.h Base classes for the geo primitives.
//Luke Lenhart, 2007
//See /docs/License.txt for details on how this code may be used.

//This file is only meant to be included by Geo.h.

#pragma once

#include "../Config.h"

#ifdef MPMA_COMPILE_GEO

#include "../base/Vary.h"

namespace
{
    // -- helpers shared between VectorN and MatrixN
    
    template <typename VecType>
    inline void AssignFromArray(VecType &v, const float *arr)
    {
        for (nuint i=0; i<v.ElementCount(); ++i)
            v[i]=arr[i];
    }
    
    template <typename VecType>
    inline void AssignFromValue(VecType &v, float val)
    {
        for (nuint i=0; i<v.ElementCount(); ++i)
            v[i]=val;
    }

    template <typename VecType>
    inline VecType Scale(const VecType &v, const float &s)
    {
        VecType r;
        for (nuint i=0; i<v.ElementCount(); ++i)
            r[i]=v[i]*s;
        return r;
    }

    template <typename VecType>
    inline VecType Scale(const VecType &v1, const VecType &v2)
    {
        VecType r;
        for (nuint i=0; i<v1.ElementCount(); ++i)
            r[i]=v1[i]*v2[i];
        return r;
    }

    template <typename VecType>
    inline VecType Add(const VecType &v1, const VecType &v2)
    {
        VecType r;
        for (nuint i=0; i<v1.ElementCount(); ++i)
            r[i]=v1[i]+v2[i];
        return r;
    }

    template <typename VecType>
    inline VecType Sub(const VecType &v1, const VecType &v2)
    {
        VecType r;
        for (nuint i=0; i<v1.ElementCount(); ++i)
            r[i]=v1[i]-v2[i];
        return r;
    }

    template <typename VecType>
    inline VecType Neg(const VecType &v)
    {
        VecType r;
        for (nuint i=0; i<v.ElementCount(); ++i)
            r[i]=-v[i];
        return r;
    }

    template <typename VecType>
    inline VecType Div(const VecType &v1, const VecType &v2)
    {
        VecType r;
        for (nuint i=0; i<v1.ElementCount(); ++i)
            r[i]=v1[i]/v2[i];
        return r;
    }

    template <typename VecType>
    inline void ScaleSelf(VecType &v, const float &s)
    {
        for (nuint i=0; i<v.ElementCount(); ++i)
            v[i]*=s;
    }

    template <typename VecType>
    inline void ScaleSelf(VecType &v1, const VecType &v2)
    {
        for (nuint i=0; i<v1.ElementCount(); ++i)
            v1[i]*=v2[i];
    }

    template <typename VecType>
    inline void AddSelf(VecType &v1, const VecType &v2)
    {
        for (nuint i=0; i<v1.ElementCount(); ++i)
            v1[i]+=v2[i];
    }

    template <typename VecType>
    inline void SubSelf(VecType &v1, const VecType &v2)
    {
        for (nuint i=0; i<v1.ElementCount(); ++i)
            v1[i]-=v2[i];
    }

    template <typename VecType>
    inline void NegSelf(VecType &v)
    {
        for (nuint i=0; i<v.ElementCount(); ++i)
            v[i]=-v[i];
    }

    template <typename VecType>
    inline void DivSelf(VecType &v1, const VecType &v2)
    {
        for (nuint i=0; i<v1.ElementCount(); ++i)
            v1[i]/=v2[i];
    }
}

namespace GEO
{
    template <nuint elemCount> struct MatrixN; //predef

    
    // -- VectorN
    
    //!Base type of all vectors
    template <nuint elemCount>
    struct VectorN
    {
        typedef MatrixN<elemCount> MatrixType; //!<associated matrix type
        typedef MatrixN<elemCount+1> GreaterMatrixType; //!<associated matrix type with one extra dimension
        
        float elements[elemCount]; //!<Elements of the vector.
        inline nuint ElementCount() const { return elemCount; } //!<The number of elements in the vector.
        
        //ctors
        inline VectorN() {} //!<ctor
        inline VectorN(const float *array) { AssignFromArray(*this,array); } //!<ctor
        inline VectorN(float c) { AssignFromValue(*this,c); } //!<ctor
        
        //access operators
        inline float& operator[](nuint index) { return elements[index]; } //!<access an element by index with []
        inline const float& operator[](nuint index) const { return elements[index]; } //!<access an element by index with []
        
        //binary non-altering operators
        inline VectorN operator*(const VectorN &o) const { return Scale(*this,o); } //!<per-element scale
        inline VectorN operator*(const float &o) const { return Scale<VectorN>(*this,o); } //!<scale by a value
        friend inline VectorN operator*(const float &o, const VectorN &me) { return Scale<VectorN>(me,o); } //!<scale by a value
        inline VectorN operator/(const VectorN &o) const { return Div(*this,o); } //!<per-element divide
        inline VectorN operator/(const float &o) const { return Scale<VectorN>(*this,1/o); } //!<divide elements by a value
        inline VectorN operator+(const VectorN &o) const { return Add(*this,o); } //!<add
        inline VectorN operator+(const float &o) const { return Add(*this,VectorN(o)); } //!<add
        inline VectorN operator-(const VectorN &o) const { return Sub(*this,o); } //!<subtract
        inline VectorN operator-(const float &o) const { return Sub(*this,VectorN(o)); } //!<subtract
        
        //binary self modifying operators
        inline void operator*=(const VectorN &o) { ScaleSelf(*this,o); } //!<per-element scale
        inline void operator*=(const float &o) { ScaleSelf<VectorN>(*this,o); } //!<scale by a value
        inline void operator/=(const VectorN &o) { DivSelf(*this,o); } //!<per-element divide
        inline void operator/=(const float &o) { ScaleSelf<VectorN>(*this,1/o); } //!<divide elements by a value
        inline void operator+=(const VectorN &o) { AddSelf(*this,o); } //!<add
        inline void operator+=(const float &o) { AddSelf(*this,VectorN(o)); } //!<add
        inline void operator-=(const VectorN &o) { SubSelf(*this,o); } //!<subtract
        inline void operator-=(const float &o) { SubSelf(*this,VectorN(o)); } //!<subtract
        
        //unary operators
        inline VectorN operator-() const { return Neg(*this); } //!<negation
        
        //comparison
        inline friend bool operator==(const VectorN &v1, const VectorN &v2) { return NearEqual(v1,v2); } //!<comparison
        inline friend bool operator==(const VectorN &v, const float &c) { return NearEqual(v,c); } //!<comparison
        
        inline friend bool operator!=(const VectorN &v1, const VectorN &v2) { return !NearEqual(v1,v2); } //!<comparison
        inline friend bool operator!=(const VectorN &v, const float &c) { return !NearEqual(v,c); } //!<comparison
        
        //other functions
        inline float LengthSquared() const { return VecDot(*this,*this); } //!<the vector length, squared
        inline float Length() const { return std::sqrt(LengthSquared()); } //!<the vector length
        inline VectorN Normal() const { return VecNormal(*this); } //!<get a normalized version of the vector
        inline void Normalize() { VecNormalize(*this); } //!<normalize this vector
        inline float Dot(const VectorN &o) const { return VecDot(*this,o); } //!<dot product
        inline float AngleBetween(const VectorN &o) const { return VecAngleBetween(*this,o); } //!<angle between vectors
        inline void Transform(const MatrixType &m) const { *this=TransformVector(m,*this); } //!<transform (multiply) this vector with a matrix
        
        //human readable conversion
        operator const std::string() const; //!<human readable string representation
    };
    
    template <nuint elemCount>
    VectorN<elemCount>::operator const std::string() const
    {
        std::string s="(";

        for (nuint i=0; i<ElementCount(); ++i)
        {
            s+=MPMA::Vary(elements[i]);
            if (i!=ElementCount()-1) s+=", ";
        }

        s+=")";
        return s;
    }
    

    // -- MatrixN
    
    //!Base type of all square matrices (row-based)
    template <nuint rowCount>
    struct MatrixN
    {
        typedef VectorN<rowCount> VectorType; //!<associated vector type
        typedef VectorN<rowCount-1> LesserVectorType; //!<associated vector type with one less dimension
        
        union
        {
            float elements[rowCount*rowCount]; //!<The individual elements of the matrix.
            float rowcol[rowCount][rowCount]; //!<The elements of the matrix arranged my row and column.
        };
        inline nuint RowColCount() const { return rowCount; } //!<The number of rows or columns in the matrix.
        inline nuint ElementCount() const { return rowCount*rowCount; } //!<The number of elements in the matrix.
        
        //ctors
        inline MatrixN() {} //!<ctor
        inline MatrixN(const float *array) { AssignFromArray(*this,array); } //!<ctor
        inline MatrixN(float c) { AssignFromValue(*this,c); } //!<ctor
        
        //proxy access class, to allow for both 1d and 2d array notation to work
        template <typename T, typename TRet>
        class Proxy
        {
        public:
            inline Proxy(T &mat, nsint ind): mat(mat), ind(ind) {}
            inline operator TRet&() { return mat.elements[ind]; }
            inline TRet& operator[](nuint offset) { return mat.rowcol[ind][offset]; }
            void operator=(const float val) {mat.elements[ind]=val;}
        
        private:
            T &mat;
            nsint ind;
            void operator=(const Proxy<T, TRet> &notAllowed); //disable
        };
        
        //access operators
        inline Proxy<MatrixN, float> operator[](nuint ind) { return Proxy<MatrixN, float>(*this,ind); } //!<access using [] or [][]
        inline Proxy<const MatrixN, const float> operator[](nuint ind) const { return Proxy<const MatrixN, const float>(*this,ind); } //!<access using [] or [][]
        
        inline float m(int row, int col) const { return rowcol[row][col]; } //!<alias to rowcol
        inline float& m(int row, int col) { return rowcol[row][col]; } //!<alias to rowcol
        
        //equality operators
        inline friend bool operator==(const MatrixN &m1, const MatrixN &m2) { return NearEqual(m1,m2); } //!<comparison
        inline friend bool operator!=(const MatrixN &m1, const MatrixN &m2) { return !NearEqual(m1,m2); } //!<comparison
        
        //access a row(direct) or column(copied out) in the matrix as a vector
        inline VectorType& Row(nuint r) { return *(VectorType*)rowcol[r]; } //!<access a row as a vector
        inline const VectorType& Row(nuint r) const { return *(const VectorType*)rowcol[r]; } //!<access a row as a vector
        inline const VectorType GetColumn(nuint col) const; //!<get a copy of a column as a vector
        inline void SetColumn(nuint col, const VectorType &vec); //!sets a column into the matrix from a vector
        inline void SetColumn(nuint col, const LesserVectorType &vec); //!sets a column into the matrix from a vector except the last row
        
        //binary non-altering operators
        inline MatrixN operator*(const float &o) const { return Scale<MatrixN>(*this,o); } //!<scale by a value
        inline MatrixN operator*(const int &o) const { return Scale<MatrixN>(*this,(float)o); } //!<scale by a value
        friend inline MatrixN operator*(const float &o, const MatrixN &me) { return Scale(me,o); } //!<scale by a value
        inline MatrixN operator/(const float &o) const { return Scale<MatrixN>(*this,1/o); } //!<per-element divide
        inline MatrixN operator/(const int &o) const { return Scale<MatrixN>(*this,1.0f/o); } //!<divide elements by a value
        inline MatrixN operator+(const MatrixN &o) const { return Add(*this,o); } //!<per-element add
        inline MatrixN operator+(const float &o) const { return Add(*this,MatrixN(o)); } //!<add
        inline MatrixN operator-(const MatrixN &o) const { return Sub(*this,o); } //!<per-element subtract
        inline MatrixN operator-(const float &o) const { return Sub(*this,MatrixN(o)); } //!<subtract
        
        inline MatrixN operator-() const { return Neg(*this); } //!<negation
        
        //binary self-modifying operators
        inline void operator*=(const float &o) { ScaleSelf<MatrixN>(*this,o); } //!<scale by a value
        inline void operator/=(const float &o) { ScaleSelf<MatrixN>(*this,1/o); } //!<divide elements by a value
        inline void operator+=(const MatrixN &o) { AddSelf(*this,o); } //!<per-element add
        inline void operator+=(const float &o) { AddSelf(*this,MatrixN(o)); } //!<add
        inline void operator-=(const MatrixN &o) { SubSelf(*this,o); } //!<per-element subtract
        inline void operator-=(const float &o) { SubSelf(*this,MatrixN(o)); } //!<subtract
        
        //other operators
        inline MatrixN operator*(const MatrixN &o) const { return MatMul(*this,o); } //!<matrix multiplication
        inline void operator*=(const MatrixN &o) { *this=MatMul(*this,o); } //!<matrix multiplication
        inline VectorType operator*(const VectorType &v) const { return TransformVector(*this,v); } //!<multiply a matrix with a vector
        
        //other operations
        inline MatrixN Transpose() const; //!<get the transpose of the matrix
        inline void SetTranspose(); //!<transpose the matrix in place
        inline void MakeIdentity(); //!<turn the matrix into an identity matrix
        inline LesserVectorType GetTranslation() const { return LesserVectorType(GetColumn(RowColCount()-1).elements); } //!<retrieve the translation portion of the matrix
        inline void SetTranslation(const LesserVectorType &v) { SetColumn(RowColCount()-1,v); } //!<set the translation portion of the matrix

        //human readable conversion
        operator const std::string() const; //!<human readable string representation
    };
    
    template <nuint rowCount>
    inline const typename MatrixN<rowCount>::VectorType MatrixN<rowCount>::GetColumn(nuint col) const
    {
        VectorType vec;
        for (nuint i=0; i<RowColCount(); ++i)
            vec[i]=rowcol[i][col];
        return vec;
    }
    
    template <nuint rowCount>
    inline void MatrixN<rowCount>::SetColumn(nuint col, const VectorType &vec)
    {
        for (nuint i=0; i<RowColCount(); ++i)
            rowcol[i][col]=vec[i];
    }
    
    template <nuint rowCount>
    inline void MatrixN<rowCount>::SetColumn(nuint col, const LesserVectorType &vec)
    {
        for (nuint i=0; i<vec.ElementCount(); ++i)
            rowcol[i][col]=vec[i];
    }
    
    template <nuint rowCount>
    MatrixN<rowCount> MatrixN<rowCount>::Transpose() const
    {
        return MatTranspose(*this);
    }
    
    template <nuint rowCount>
    void MatrixN<rowCount>::SetTranspose()
    {
        //swap the rows and cols, leaving the diagonal alone
        for (nuint r=1; r<RowColCount(); ++r)
        {
            for (nuint c=0; c<r; ++c)
            {
                float tmp=rowcol[r][c];
                rowcol[r][c]=rowcol[c][r];
                rowcol[c][r]=tmp;
            }
        }
    }
    
    template <nuint rowCount>
    inline void MatrixN<rowCount>::MakeIdentity()
    {
        for (nuint x=0; x<RowColCount(); ++x)
            for (nuint y=0; y<RowColCount(); ++y)
                rowcol[x][y]=0;

        for (nuint i=0; i<RowColCount(); ++i)
            rowcol[i][i]=1;
    }
    
    template <nuint rowCount>
    MatrixN<rowCount>::operator const std::string() const
    {
        std::string s="[";
        for (nuint i=0; i<RowColCount(); ++i)
        {
            s+=Row(i);
            if (i!=RowColCount()-1) s+=", ";
        }

        s+="]";
        return s;
    }
    
} //namespace GEO

#endif //#ifdef MPMA_COMPILE_GEO
