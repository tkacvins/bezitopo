/******************************************************/
/*                                                    */
/* spiral.h - Cornu or Euler spirals                  */
/*                                                    */
/******************************************************/
#ifndef SPIRAL_H
#define SPIRAL_H

#include <cmath>
#include "point.h"
#include "xyz.h"
#include "segment.h"
#include "arc.h"

/* Clothance is the derivative of curvature with respect to distance
 * along the curve.
 */
xy cornu(double t); //clothance=2
xy cornu(double t,double curvature,double clothance);
double spiralbearing(double t,double curvature,double clothance);
int ispiralbearing(double t,double curvature,double clothance);
double spiralcurvature(double t,double curvature,double clothance);
void cornustats();

class spiralarc: public segment
/* station() ignores the x and y coordinates of start and end.
 * mid, midbear, etc. must be set so that the station() values
 * match start and end. This can take several iterations.
 *
 * d and s: d is the bearing at the end - the bearing at the start.
 * s is the sum of the start and end bearings - twice the chord bearing
 * (in setdelta) or - twice the midpoint bearing (in _setdelta).
 */
{
private:
  xy mid;
  double cur,clo,len;
  int midbear;
public:
  spiralarc();
  spiralarc(const segment &b);
  spiralarc(const arc &b);
  spiralarc(xyz kra,xyz fam);
  spiralarc(xyz kra,double c1,double c2,xyz fam);
  spiralarc(xyz kra,xyz mij,xyz fam,int mbear,double curvature,double clothance,double length);
  double length() const
  {
    return len;
  }
  virtual int bearing(double along)
  {
    return midbear+ispiralbearing(along-len/2,cur,clo);
  }
  int startbearing() const
  {
    return midbear+ispiralbearing(-len/2,cur,clo);
  }
  int endbearing() const
  {
    return midbear+ispiralbearing(len/2,cur,clo);
  }
  virtual double curvature(double along) const
  {
    return cur+clo*(along-len/2);
  }
  virtual double radius(double along) const
  {
    return 1/curvature(along);
  }
  virtual int getdelta()
  {
    return radtobin(cur*len);
  }
  virtual int getdelta2()
  {
    return startbearing()+endbearing()-2*dir(xy(start),xy(end));
  }
  virtual double clothance()
  {
    return clo;
  }
  virtual xyz station(double along) const;
  //virtual double sthrow();
  /* "throw" is a reserved word.
   * The throw is the minimum distance between the circles (one of which may be a line)
   * that osculate the ends of the spiral arc. It is also called the offset, but
   * offset() will create a segment or arc that is parallel to the given one.
   * Trying to offset a spiral arc will throw an error.
   */
  void _setdelta(int d,int s=0);
  void _fixends(double p);
  void split(double along,spiralarc &a,spiralarc &b);
  void setdelta(int d,int s=0);
  bool valid()
  {
    return (std::isfinite(cur) && std::isfinite(clo) && std::isfinite(len));
  }
};

#endif
