//!\file ReferenceCount.h Reference counting utilities.
//Luke Lenhart (2008)
//See /docs/License.txt for details on how this code may be used.

#ifndef REFERENCECOUNT_H_INCLUDED
#define REFERENCECOUNT_H_INCLUDED

#include "Types.h"

namespace MPMA
{
    //!Deriving from this allows multiple instances of a class to share data, which will be automatically freed when the last instance is freed.
    template <typename DataType>
    class ReferenceCountedData
    {
    public:
        //!Creates a new instance of the data.
        ReferenceCountedData();

        //!Removes a reference to the data.
        virtual ~ReferenceCountedData();

        //!Creates a new reference to existing data.
        ReferenceCountedData(const ReferenceCountedData &other);
        //!Removes the old reference and adds a new reference to existing data.
        ReferenceCountedData& operator=(const ReferenceCountedData &other);

        //!Returns whether one objects references the same data as another.
        inline bool SharesDataWith(const ReferenceCountedData<DataType> &o)
            { return referencedData==o.referencedData; }

    protected:
        //!Returns the data that is being reference counted.
        inline DataType& Data()
            { return referencedData->data; }
        //!Returns the data that is being reference counted.
        inline const DataType& Data() const
            { return referencedData->data; }

        //!Returns the number of references left (intended for debugging purposes).
        inline nuint ReferenceCount() const
            { return referencedData->count; }

        //!Returns true if this is the only reference to the shared data.  This can be useful in the derived class's constructor/destructor to do any setup or cleanup.
        inline bool IsOnlyReference() const
            { return ReferenceCount()==1; }

        //!Remove the old reference from this object, and creates a new instance of the data which this objet then references.
        void CreateNewData();

    private:
        struct ReferencedData
        {
			volatile nuint count;
            DataType data;
        };

        ReferencedData *referencedData;

        //releases a reference and frees the data if needed
        void ReleaseReference();
    };
}

//include template and inline implementations
#ifndef REFERENCECOUNT_INCLUDE_INLINE
    #define REFERENCECOUNT_INCLUDE_INLINE
    #include "ReferenceCount.cpp"
#endif

#endif //REFERENCECOUNT_H_INCLUDED
