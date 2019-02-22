//!\file ReferenceCount.cpp Reference counting utilities.
//Luke Lenhart (2008)
//See /docs/License.txt for details on how this code may be used.

#ifndef REFERENCECOUNT_INCLUDE_INLINE // ---- normal compiled section ----

#include "ReferenceCount.h"

bool mpmaForceReferenceToReferenceCountCPP=false; //work around a problem using MPMA as a static library

// ----------------- end normal compile section ----------------
#else // -------------- start template and inline section -----------------
#ifndef REFERENCECOUNT_CPP_TEMPLATES_INCLUDED
#define REFERENCECOUNT_CPP_TEMPLATES_INCLUDED

#include "Memory.h"
#include "Locks.h"

namespace MPMA
{
    //Creates a new instance of the data.
    template <typename DataType>
    ReferenceCountedData<DataType>::ReferenceCountedData()
    {
        referencedData=new3(ReferencedData);
        referencedData->count=1;
    }

    //Removes a reference to the data.
    template <typename DataType>
    ReferenceCountedData<DataType>::~ReferenceCountedData()
    {
        ReleaseReference();
    }

    //releases a reference and frees the data if needed
    template <typename DataType>
    void ReferenceCountedData<DataType>::ReleaseReference()
    {
        if (referencedData)
        {
            AtomicIntDec(&referencedData->count);
            if (referencedData->count==0)
            {
                delete3(referencedData);
                referencedData=0;
            }
        }
    }

    //Creates a new reference to existing data.
    template <typename DataType>
    ReferenceCountedData<DataType>::ReferenceCountedData(const ReferenceCountedData &other)
    {
        referencedData=0;
        *this=other;
    }

    //Removes the old reference and adds a new reference to existing data.
    template <typename DataType>
    ReferenceCountedData<DataType>& ReferenceCountedData<DataType>::operator=(const ReferenceCountedData &other)
    {
        if (other.referencedData==referencedData) return *this; //if data pointer is the same, it's a self-assignment

        ReleaseReference();

        AtomicIntInc(&other.referencedData->count);
        referencedData=other.referencedData;

        return *this;
    }

    //Remove the old reference and creates a new instance of the data.
    template <typename DataType>
    void ReferenceCountedData<DataType>::CreateNewData()
    {
        ReleaseReference();

        referencedData=new3(ReferencedData);
        referencedData->count=1;
    }
}

#endif //REFERENCECOUNT_CPP_TEMPLATES_INCLUDED
#endif //REFERENCECOUNT_INCLUDE_INLINE ---- end template and inline section ----