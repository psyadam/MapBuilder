//simple geometry intersection functions. This is not yet officially part of MPMA, due to the fact that what's here is just random scattered stuff I found useful.
//Luke Lenhart (2005-2011)
//See /docs/License.txt for details on how this code may be used.

#include "GeoIntersect.h"

#ifdef MPMA_COMPILE_GEO

#include <cmath>

//
namespace GEO
{
    namespace INTERSECT
    {
        //line and plane intersection
        bool LinePlane(const Line &inLine, const Plane &inPlane, Vector3 &outPoint)
        {
            float dotPL=VecDot(inPlane.Normal(), inLine.dir);

            //check if they intersect
            if (std::fabs(dotPL) > FLOAT_TOLERANCE)
            {
                float param=-(VecDot(inPlane.Normal(),inLine.pos)+inPlane.c3)/dotPL;

                //scale direction of line from its base point to get intersection
                outPoint=inLine.pos + inLine.dir*param;
                return true;
            }

            return false;
        }

        bool LinePlane(const Line &inLine, const Plane &inPlane)
        {
            float dotPL=VecDot(inPlane.Normal(),inLine.dir);
            return std::fabs(dotPL) > FLOAT_TOLERANCE;
        }

        //line and sphere
        bool LineSphere(const Line &inLine, const Sphere &inSphere, Vector3 &outPoint1, Vector3 &outPoint2)
        {
            //see if it intersected
            Vector3 dirLineToSphere=inSphere.pos-inLine.pos;
            float sMag=VecDot(dirLineToSphere, inLine.dir);
            float dirLineToSphereMag=VecDot(dirLineToSphere, dirLineToSphere);
            float rPow2=inSphere.radius*inSphere.radius;

            float m=dirLineToSphereMag-sMag*sMag;

            if (m>rPow2)
                return false;

            //it did intersect, so figure out where
            float q=sqrt(rPow2-m);
            float t1=sMag-q;
            float t2=sMag+q;

            outPoint1=inLine.pos+inLine.dir*t1;
            outPoint2=inLine.pos+inLine.dir*t2;
            return true;
        }

        bool LineSphere(const Line &inLine, const Sphere &inSphere)
        {
            //see if it intersected
            Vector3 dirLineToSphere=inSphere.pos-inLine.pos;
            float sMag=VecDot(dirLineToSphere, inLine.dir);
            float dirLineToSphereMag=VecDot(dirLineToSphere, dirLineToSphere);
            float rPow2=inSphere.radius*inSphere.radius;

            float m=dirLineToSphereMag-sMag*sMag;

            if (m>rPow2)
                return false;

            return true;
        }

        bool RaySphere(const Line &inLine, const Sphere &inSphere)
        {
            //see if a line intersects
            Vector3 dirLineToSphere=inSphere.pos-inLine.pos;
            float sMag=VecDot(dirLineToSphere, inLine.dir);
            float dirLineToSphereMag=VecDot(dirLineToSphere, dirLineToSphere);
            float rPow2=inSphere.radius*inSphere.radius;

            float m=dirLineToSphereMag-sMag*sMag;

            if (m>rPow2)
                return false;

            //it does intersect, figure out where
            float q=sqrt(rPow2-m);
            float t1=sMag-q;
            float t2=sMag+q;

            //only in the forward direction
            if (t1>0 || t2>0)
                return true;

            return false;
        }

        //line and axis-aligned ellipsoid
        bool LineYAlignedEllipsoid(const Line &inLine, const YAlignedEllipsoid &inEllipse, Vector3 &outPoint1, Vector3 &outPoint2)
        {
            //shift line, so that we can solve for the ellipsoid being at the origin
            Vector3 adjustedLinePos=inLine.pos-inEllipse.pos;

            //quadratic parts
            float ymodsqr=inEllipse.ymultiplier*inEllipse.ymultiplier;

            float a=ymodsqr*(inLine.dir.x()*inLine.dir.x() + inLine.dir.z()*inLine.dir.z()) + inLine.dir.y()*inLine.dir.y();
            float b=2*(ymodsqr*(adjustedLinePos.x()*inLine.dir.x() + adjustedLinePos.z()*inLine.dir.z()) + adjustedLinePos.y()*inLine.dir.y());
            float c=ymodsqr*(adjustedLinePos.x()*adjustedLinePos.x() + adjustedLinePos.z()*adjustedLinePos.z() - inEllipse.radius*inEllipse.radius) + adjustedLinePos.y()*adjustedLinePos.y();
            float disc=b*b-4*a*c;

            if (disc<=0)
                return false;

            //now calc intersect points
            float sqrDisc=sqrt(disc);
            float t1=(-b-sqrDisc)/(2*a);
            float t2=(-b+sqrDisc)/(2*a);
            outPoint1=inLine.pos+t1*inLine.dir;
            outPoint2=inLine.pos+t2*inLine.dir;

            return true;
        }

        bool RayYAlignedEllipsoid(const Line &inLine, const YAlignedEllipsoid &inEllipse)
        {
            //shift line, so that we can solve for the ellipsoid being at the origin
            Vector3 adjustedLinePos=inLine.pos-inEllipse.pos;

            //quadratic parts
            float ymodsqr=inEllipse.ymultiplier*inEllipse.ymultiplier;

            float a=ymodsqr*(inLine.dir.x()*inLine.dir.x() + inLine.dir.z()*inLine.dir.z()) + inLine.dir.y()*inLine.dir.y();
            float b=2*(ymodsqr*(adjustedLinePos.x()*inLine.dir.x() + adjustedLinePos.z()*inLine.dir.z()) + adjustedLinePos.y()*inLine.dir.y());
            float c=ymodsqr*(adjustedLinePos.x()*adjustedLinePos.x() + adjustedLinePos.z()*adjustedLinePos.z() - inEllipse.radius*inEllipse.radius) + adjustedLinePos.y()*adjustedLinePos.y();
            float disc=b*b-4*a*c;

            if (disc<=0)
                return false;

            //only in the forward direction
            float sqrDisc=sqrt(disc);
            float t1=(-b-sqrDisc)/(2*a);
            float t2=(-b+sqrDisc)/(2*a);
            if (t1>0 || t2>0)
                return true;

            return false;
        }

        //line and axis aligned 3d rectangle
        bool LineAARectoid(const Line &inLine, const AARectoid &rect, Vector3 &outPoint1, Vector3 &outNormal1, Vector3 &outPoint2, Vector3 &outNormal2)
        {
            //set up array to write output to
            nuint isectNum=0;
            Vector3 *outPoints[2]={&outPoint1, &outPoint2};
            Vector3 *outNormals[2]={&outNormal1, &outNormal2};

            //precalc rect stuff
            float minx=rect.center.x()-rect.xradius;
            float maxx=rect.center.x()+rect.xradius;
            float miny=rect.center.y()-rect.yradius;
            float maxy=rect.center.y()+rect.yradius;
            float minz=rect.center.z()-rect.zradius;
            float maxz=rect.center.z()+rect.zradius;

            //check each plane
            Vector3 isect;
            Plane plane(rect.center-Vector3(0, 0, rect.zradius), Vector3(0, 0, -1));
            if (LinePlane(inLine, plane, isect))
            {
                if (isect.x()>minx && isect.x()<maxx && isect.y()>miny && isect.y()<maxy)
                {
                    *outPoints[isectNum]=isect;
                    *outNormals[isectNum]=plane.Normal();
                    ++isectNum;
                }
            }

            plane=Plane(rect.center+Vector3(0, 0, rect.zradius), Vector3(0, 0, 1));
            if (LinePlane(inLine, plane, isect))
            {
                if (isect.x()>minx && isect.x()<maxx && isect.y()>miny && isect.y()<maxy)
                {
                    *outPoints[isectNum]=isect;
                    *outNormals[isectNum]=plane.Normal();
                    ++isectNum;
                }
            }

            plane=Plane(rect.center-Vector3(0, rect.yradius, 0), Vector3(0, -1, 0));
            if (LinePlane(inLine, plane, isect))
            {
                if (isect.x()>minx && isect.x()<maxx && isect.z()>minz && isect.z()<maxz)
                {
                    *outPoints[isectNum]=isect;
                    *outNormals[isectNum]=plane.Normal();
                    ++isectNum;
                }
            }

            plane=Plane(rect.center+Vector3(0, rect.yradius, 0), Vector3(0, 1, 0));
            if (LinePlane(inLine, plane, isect))
            {
                if (isect.x()>minx && isect.x()<maxx && isect.z()>minz && isect.z()<maxz)
                {
                    *outPoints[isectNum]=isect;
                    *outNormals[isectNum]=plane.Normal();
                    ++isectNum;
                }
            }

            plane=Plane(rect.center-Vector3(rect.xradius, 0, 0), Vector3(-1, 0, 0));
            if (LinePlane(inLine, plane, isect))
            {
                if (isect.y()>miny && isect.y()<maxy && isect.z()>minz && isect.z()<maxz)
                {
                    *outPoints[isectNum]=isect;
                    *outNormals[isectNum]=plane.Normal();
                    ++isectNum;
                }
            }

            plane=Plane(rect.center+Vector3(rect.xradius, 0, 0), Vector3(1, 0, 0));
            if (LinePlane(inLine, plane, isect))
            {
                if (isect.y()>miny && isect.y()<maxy && isect.z()>minz && isect.z()<maxz)
                {
                    *outPoints[isectNum]=isect;
                    *outNormals[isectNum]=plane.Normal();
                    ++isectNum;
                }
            }

            if (isectNum==1)
            {
                outPoint2=outPoint1;
                outNormal2=outNormal1;
            }

#ifdef _DEBUG
            if (isectNum>2)
                throw "Bad number of intersects in LineAARectoid";
#endif

            return isectNum!=0;
        }

        //line and axis aligned unbounded cylinder
        bool LineXAlignedUnboundedCylinder(const Line &inLine, const XAlignedUnboundedCylinder &inCyl, Vector3 &outPoint1, Vector3 &outPoint2)
        {
            //shift the line so we can intersect with a cylinder at the origin
            GEO::Vector3 adjustedLinePos=inLine.pos-GEO::Vector3(0, inCyl.y, inCyl.z);

            //make the quadratic parts to see if it intersects
            float a=inLine.dir.y()*inLine.dir.y() + inLine.dir.z()*inLine.dir.z();
            float b=2*(adjustedLinePos.y()*inLine.dir.y() + adjustedLinePos.z()*inLine.dir.z());
            float c=adjustedLinePos.y()*adjustedLinePos.y() + adjustedLinePos.z()*adjustedLinePos.z() - inCyl.radius*inCyl.radius;
            float disc=b*b-4*a*c;

            if (disc<=0)
                return false;

            //now calc intersect points
            float sqrDisc=sqrt(disc);
            float t1=(-b-sqrDisc)/(2*a);
            float t2=(-b+sqrDisc)/(2*a);
            outPoint1=inLine.pos+t1*inLine.dir;
            outPoint2=inLine.pos+t2*inLine.dir;

            return true;
        }

        Vector3 NormalFromIntersection(const Vector3 &isect, const XAlignedUnboundedCylinder &inCyl)
        {
            return Vector3(isect-Vector3(isect.x(), inCyl.y, inCyl.z)).Normal();
        }

        bool RayXAlignedUnboundedCylinder(const Line &inLine, const XAlignedUnboundedCylinder &inCyl)
        {
            //shift the line so we can intersect with a cylinder at the origin
            GEO::Vector3 adjustedLinePos=inLine.pos-GEO::Vector3(0, inCyl.y, inCyl.z);

            //make the quadratic parts to see if it intersects
            float a=inLine.dir.y()*inLine.dir.y() + inLine.dir.z()*inLine.dir.z();
            float b=2*(adjustedLinePos.y()*inLine.dir.y() + adjustedLinePos.z()*inLine.dir.z());
            float c=adjustedLinePos.y()*adjustedLinePos.y() + adjustedLinePos.z()*adjustedLinePos.z() - inCyl.radius*inCyl.radius;
            float disc=b*b-4*a*c;

            if (disc<=0)
                return false;

            //only if in the forward direction
            float sqrDisc=sqrt(disc);
            float t1=(-b-sqrDisc)/(2*a);
            float t2=(-b+sqrDisc)/(2*a);
            if (t1>0 || t2>0)
                return true;

            return false;
        }

        //circle and circle
        bool CircleCircle(const Circle &inCircle1, const Circle &inCircle2)
        {
            return (inCircle1.pos-inCircle2.pos).Length() <= inCircle1.radius+inCircle2.radius;
        }

        //line segment and circle
        bool Line2DSegmentCircle(const Line2D &inLine, const Circle &inCircle, GEO::Vector2 &outPos)
        {
            //project the center of the circle onto the line to find the closest point on the line to the circle
            Vector2 lineStartToCircle=inCircle.pos-inLine.pos;
            float lineDirLength=inLine.dir.Length();
            if (lineDirLength<0.0001f) //it's basically a point, treat it as such
            {
                outPos=inLine.pos;
                return lineStartToCircle.Length()<inCircle.radius;
            }

            Vector2 lineDirUnit=inLine.dir/lineDirLength;
            float scalarAlongLine=GEO::VecDot(lineStartToCircle, lineDirUnit);
            
            Vector2 lineClosestPoint;
            if (scalarAlongLine<0.0f)
                lineClosestPoint=inLine.pos;
            else if (scalarAlongLine>lineDirLength)
                lineClosestPoint=inLine.pos+inLine.dir;
            else
                lineClosestPoint=inLine.pos+lineDirUnit*scalarAlongLine;

            //if the point is within the circle they intersect
            outPos=lineClosestPoint;
            float distFromClosestToCircle=(lineClosestPoint-inCircle.pos).Length();
            return distFromClosestToCircle<inCircle.radius;
        }

    } //namespace INTERSECT

    namespace RESOLVE
    {
        //resolves a collision between two round objects of equal mass (circle or sphere)
        bool CircleEqualMass(const GEO::Vector2 &pos1, const GEO::Vector2 &vel1, const GEO::Vector2 &pos2, const GEO::Vector2 &vel2, GEO::Vector2 &outVel1, GEO::Vector2 &outVel2)
        {
            GEO::Vector2 baseAxis=(pos2-pos1);
            float baseAxistLength=baseAxis.Length();
            if (baseAxistLength>0.0001f)
            {
                baseAxis/=baseAxistLength;

                GEO::Vector2 p1x=baseAxis*(GEO::VecDot(baseAxis, vel1));
                GEO::Vector2 p1y=vel1-p1x;
                GEO::Vector2 p2x=-baseAxis*(GEO::VecDot(-baseAxis, vel2));
                GEO::Vector2 p2y=vel2-p2x;

                GEO::Vector2 v1x=(p1x+p2x-(p1x-p2x))*0.5f;
                GEO::Vector2 v1y=p1y;
                GEO::Vector2 v2x=(p1x+p2x-(p2x-p1x))*0.5f;
                GEO::Vector2 v2y=p2y;

                outVel1=(v1x+v1y);
                outVel2=(v2x+v2y);
                return true;
            }
            else
            {
                outVel1=vel1;
                outVel2=vel2;
                return false;
            }
        }

        //resolves a collision between two round objects of equal mass (circle or sphere)
        bool CircleDifferentMass(const GEO::Vector2 &pos1, const GEO::Vector2 &vel1, float mass1, const GEO::Vector2 &pos2, const GEO::Vector2 &vel2, float mass2, GEO::Vector2 &outVel1, GEO::Vector2 &outVel2)
        {
            GEO::Vector2 baseAxis=(pos2-pos1);
            float baseAxistLength=baseAxis.Length();
            if (baseAxistLength>0.0001f)
            {
                baseAxis/=baseAxistLength;

                GEO::Vector2 p1x=baseAxis*(GEO::VecDot(baseAxis, vel1));
                GEO::Vector2 p1y=vel1-p1x;
                GEO::Vector2 p2x=-baseAxis*(GEO::VecDot(-baseAxis, vel2));

                GEO::Vector2 v1x=p1x*(mass1-mass2)/(mass1+mass2)+p2x*2*mass2/(mass1+mass2);
                GEO::Vector2 v1y=p1y;
                GEO::Vector2 v2x=p1x*2*mass1/(mass1+mass2)+p2x*(mass2-mass1)/(mass1+mass2);
                GEO::Vector2 v2y=p1y;

                outVel1=(v1x+v1y);
                outVel2=(v2x+v2y);
                return true;
            }
            else
            {
                outVel1=vel1;
                outVel2=vel2;
                return false;
            }
        }
    } //namespace RESOLVE
}; //namespace GEO

#endif // #ifdef MPMA_COMPILE_GEO
