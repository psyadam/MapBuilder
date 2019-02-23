//!\file GeoIntersect.h A few simple geometry intersection functions.
//Luke Lenhart (2005-2011)
//See /docs/License.txt for details on how this code may be used.

#pragma once

#include "../Config.h"

#ifdef MPMA_COMPILE_GEO

#include "GeoObjects.h"

namespace GEO
{
    //!Intersection detection.
    namespace INTERSECT
    {
        // -- 3D

        //!line and plane intersection
        bool LinePlane(const Line &inLine, const Plane &inPlane, Vector3 &outPoint);
        //!line and plane intersection
        bool LinePlane(const Line &inLine, const Plane &inPlane);

        //!line and sphere
        bool LineSphere(const Line &inLine, const Sphere &inSphere, Vector3 &outPoint1, Vector3 &outPoint2);
        //!line and sphere
        bool LineSphere(const Line &inLine, const Sphere &inSphere);

        //!ray and sphere
        bool RaySphere(const Line &inLine, const Sphere &inSphere);

        //!line and y-axis-aligned ellipsoid
        bool LineYAlignedEllipsoid(const Line &inLine, const YAlignedEllipsoid &inEllipse, Vector3 &outPoint1, Vector3 &outPoint2);

        //!ray and y-axis-aligned ellipsoid
        bool RayYAlignedEllipsoid(const Line &inLine, const YAlignedEllipsoid &inEllipse);

        //!line and axis aligned 3d rectangle
        bool LineAARectoid(const Line &inLine, const AARectoid &rect, Vector3 &outPoint1, Vector3 &outNormal1, Vector3 &outPoint2, Vector3 &outNormal2);

        //!line and axis aligned unbounded cylinder
        bool LineXAlignedUnboundedCylinder(const Line &inLine, const XAlignedUnboundedCylinder &inCyl, Vector3 &outPoint1, Vector3 &outPoint2);
        //!line and axis aligned unbounded cylinder
        Vector3 NormalFromIntersection(const Vector3 &isect, const XAlignedUnboundedCylinder &inCyl);

        //!ray and x-axis-aligned unbounded cylinder
        bool RayXAlignedUnboundedCylinder(const Line &inLine, const XAlignedUnboundedCylinder &inCyl);

        // -- 2D

        //!circle and circle
        bool CircleCircle(const Circle &inCircle1, const Circle &inCircle2);

        //!line segment and circle
        bool Line2DSegmentCircle(const Line2D &inLine, const Circle &inCircle, GEO::Vector2 &outPos);
    }

    //!Collision resolution.
    namespace RESOLVE
    {
        // -- 2D

        //!resolves a collision between two round objects of equal mass (circle or sphere)
        bool CircleEqualMass(const GEO::Vector2 &pos1, const GEO::Vector2 &vel1, const GEO::Vector2 &pos2, const GEO::Vector2 &vel2, GEO::Vector2 &outVel1, GEO::Vector2 &outVel2);

        //!resolves a collision between two round objects of equal mass (circle or sphere)
        bool CircleDifferentMass(const GEO::Vector2 &pos1, const GEO::Vector2 &vel1, float mass1, const GEO::Vector2 &pos2, const GEO::Vector2 &vel2, float mass2, GEO::Vector2 &outVel1, GEO::Vector2 &outVel2);
    }

}; // namespace GEO

#endif //#ifdef MPMA_COMPILE_GEO
