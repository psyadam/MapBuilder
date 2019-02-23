//!\file GeoObjects.h A few feometric objects derived other primitives.
//Luke Lenhart (2005-2008)
//See /docs/License.txt for details on how this code may be used.

#pragma once

#include "../Config.h"

#ifdef MPMA_COMPILE_GEO

#include "Geo.h"

namespace GEO
{
    // -- 3D

    //!A line or ray.
    struct Line
    {
        Vector3 pos; //!<position of a point on the line
        Vector3 dir; //!<direction of the line

        //
        inline Line() {} //!<
        inline Line(const Vector3 &inPos, const Vector3 &inDir) //!<
            { pos=inPos; dir=inDir; }
    };

    //!A plane.
    struct Plane
    {
        float c0,c1,c2,c3; //!<plane coeffecients

        //
        inline Plane() {} //!<
        //!from a position and a normal
        inline Plane(const Vector3 &inPos, const Vector3 &inNormal)
        {
            Normal()=inNormal;
            Renormalize();
            c3=-VecDot(inPos,Normal());
        }
        //!from the 4 coeffecients
        inline Plane(float inC0,float inC1,float inC2,float inC3): c0(inC0), c1(inC1), c2(inC2), c3(inC3)
            { Renormalize(); }

        //!gets normal of plane
        inline Vector3& Normal() const { return *((Vector3*)this); }

        //!normalizes normal of plane
        inline void Renormalize() { Normal().Normalize(); }
    };

    //!A sphere.
    struct Sphere
    {
        Vector3 pos; //!<
        float radius; //!<

        //
        inline Sphere() {} //!<
        inline Sphere(const Vector3 &p, float r) //!<
        { pos=p; radius=r; }
    };

    //!Axis-aligned 3d rectangle.
    struct AARectoid
    {
        Vector3 center; //!<
        float xradius; //!<
        float yradius; //!<
        float zradius; //!<

        //
        inline AARectoid() {} //!<
        inline AARectoid(Vector3 c, float x, float y, float z) //!<
        { center=c; xradius=x; yradius=y; zradius=z; }
    };

    //!A an unbounded cylendar aligned along the x axis.
    struct XAlignedUnboundedCylinder: Line
    {
        float y; //!<
        float z; //!<
        float radius; //!<

        //
        inline XAlignedUnboundedCylinder() {} //!<
        inline XAlignedUnboundedCylinder(float ypos, float zpos, float rad) //!<
        { y=ypos; z=zpos; radius=rad; }
    };

    //!An ellipsoid whose stretch is aligned along the y axis
    struct YAlignedEllipsoid: Sphere
    {
        float ymultiplier; //!<

        //
        inline YAlignedEllipsoid() {} //!<
        inline YAlignedEllipsoid(const Vector3 &p, float r, float ymod) //!<
        { pos=p; radius=r; ymultiplier=ymod; }
    };

    // -- 2D

    //!A line or ray.
    struct Line2D
    {
        Vector2 pos; //!position of a point on the line
        Vector2 dir; //!direction of the line

        //
        inline Line2D() {} //!<
        inline Line2D(const Vector2 &inPos, const Vector2 &inDir) //!<
            { pos=inPos; dir=inDir; }
    };

    //!a circle
    struct Circle
    {
        Vector2 pos; //!<
        float radius; //!<

        inline Circle() {} //!<
        inline Circle(GEO::Vector2 position, float rad) //!<
        { pos=position; radius=rad; }
    };

    //!an ellipse aligned to the x and y axis
    struct AxisAlignedEllipse
    {
        Vector2 pos; //!<
        float radius; //!<
        float xmod; //!<
        float ymod; //!<

        inline AxisAlignedEllipse() {} //!<
        inline AxisAlignedEllipse(Vector2 position, float rad, float xax, float yax) //!<
        { pos=position; radius=rad; xmod=xax; ymod=yax; }
    };

}; //namespace GEO

#endif //#ifdef MPMA_COMPILE_GEO
