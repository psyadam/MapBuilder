//!\file GeoInterpolators.h some common interpolation stuff
//Luke Lenhart, 2004-2007
//See /docs/License.txt for details on how this code may be used.

#pragma once

#include "../Config.h"

#ifdef MPMA_COMPILE_GEO

#include "Geo.h"
#include <vector>

namespace GEO
{
    //!interpolates between two value, with w as the interpolator between 0 and 1
    template <typename T> inline T Lerp(const T &e0, const T &e1, float w)
    {
        return (1.0f-w)*e0 + w*e1;
    }

    //!bilinearly interpolates between four value (with x and y from 0 to 1)
    template <typename T> inline T Bilerp(const T &e00, const T &e10, const T &e01, const T &e11, float x, float y)
    {
        return Lerp( Lerp(e00,e10,x), Lerp(e01,e11,x), y);
    }

    //!adjust current in the direction of target by at most change
    template <typename T> inline T AdjustTowardsTarget(const T &current, const T &target, const T &change)
    {
        T newVal=current;
        if (std::fabs(target-current)<change)
        {
            newVal=target;
        }
        else
        {
            if (target<current)
                newVal-=change;
            else
                newVal+=change;
        }
        return newVal;
    }

}; //namespace GEO

#endif //#ifdef MPMA_COMPILE_GEO
