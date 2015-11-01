/******************************************************/
/*                                                    */
/* polyline.cpp - polylines                           */
/*                                                    */
/******************************************************/

/* A polyline can be open or closed. Smoothing a polyline does this:
   If it is open, it tries various starting angles until it finds the shortest total length.
   If it is closed and has an odd number of points, it computes the two starting angles
   that make the ending angle the same, and chooses the one with the shortest total length.
   If it is closed and has an even number of points, it distributes the angular misclosure
   evenly among the points, and makes the shortest total length.
   */

#include <cassert>
#include <iostream>
#include "polyline.h"
#include "manysum.h"
using namespace std;

int midarcdir(xy a,xy b,xy c)
/* Returns the bearing of the arc abc at point b. May be off by 360°;
 * make sure consecutive bearings do not differ by more than 180°.
 */
{
  return dir(a,b)+dir(b,c)-dir(a,c);
}

polyline::polyline()
{
  elevation=0;
}

polyarc::polyarc(): polyline::polyline()
{
}

polyspiral::polyspiral(): polyarc::polyarc()
{
}

polyline::polyline(double e)
{
  elevation=e;
}

polyarc::polyarc(double e): polyline::polyline(e)
{
}

polyspiral::polyspiral(double e): polyarc::polyarc(e)
{
}

polyarc::polyarc(polyline &p)
{
  elevation=p.elevation;
  endpoints=p.endpoints;
  lengths=p.lengths;
  deltas.resize(lengths.size());
}

polyspiral::polyspiral(polyline &p)
{
  int i,j;
  elevation=p.elevation;
  endpoints=p.endpoints;
  lengths=p.lengths;
  deltas.resize(lengths.size());
  delta2s.resize(lengths.size());
  bearings.resize(endpoints.size());
  midbearings.resize(lengths.size());
  midpoints.resize(lengths.size());
  clothances.resize(lengths.size());
  curvatures.resize(lengths.size());
  for (i=0;i<lengths.size();i++)
  {
    j=(i+1==endpoints.size())?0:i+1;
    midpoints[i]=(endpoints[i]+endpoints[j])/2;
    midbearings[i]=dir(endpoints[i],endpoints[j]);
    if (i)
      midbearings[i]=midbearings[i-1]+foldangle(midbearings[i]-midbearings[i-1]);
    bearings[i]=midbearings[i];
  }
}

bool polyline::isopen()
{
  return endpoints.size()>lengths.size();
}

int polyline::size()
{
  return lengths.size();
}

segment polyline::getsegment(int i)
{
  i%=lengths.size();
  if (i<0)
    i+=lengths.size();
  return segment(xyz(endpoints[i],elevation),xyz(endpoints[(i+1)%endpoints.size()],elevation));
}

arc polyarc::getarc(int i)
{
  i%=deltas.size();
  if (i<0)
    i+=deltas.size();
  return arc(xyz(endpoints[i],elevation),xyz(endpoints[(i+1)%endpoints.size()],elevation),deltas[i]);
}

spiralarc polyspiral::getspiralarc(int i)
{
  i%=deltas.size();
  if (i<0)
    i+=deltas.size();
  return spiralarc(xyz(endpoints[i],elevation),xyz(midpoints[i],elevation),
		   xyz(endpoints[(i+1)%endpoints.size()],elevation),midbearings[i],
		   curvatures[i],clothances[i],lengths[i]);
}

bezier3d polyline::approx3d(double precision)
{
  bezier3d ret;
  int i;
  for (i=0;i<size();i++)
    ret+=getsegment(i).approx3d(precision);
  return ret;
}

bezier3d polyarc::approx3d(double precision)
{
  bezier3d ret;
  int i;
  for (i=0;i<size();i++)
    ret+=getarc(i).approx3d(precision);
  return ret;
}

bezier3d polyspiral::approx3d(double precision)
{
  bezier3d ret;
  int i;
  for (i=0;i<size();i++)
    ret+=getspiralarc(i).approx3d(precision);
  return ret;
}

void polyline::insert(xy newpoint,int pos)
/* Inserts newpoint in position pos. E.g. insert(xy(8,5),2) does
 * {(0,0),(1,1),(2,2),(3,3)} -> {(0,0),(1,1),(8,5),(2,2),(3,3)}.
 * If the polyline is open, inserting a point in position 0, -1, or after the last
 * (-1 means after the last) results in adding a line segment.
 * If the polyline is empty (and therefore closed), inserting a point results in
 * adding a line segment from that point to itself.
 * In all other cases, newpoint is inserted between two points and connected to
 * them with line segments.
 */
{
  bool wasopen;
  int i;
  vector<xy>::iterator ptit;
  vector<double>::iterator lenit;
  wasopen=isopen();
  if (pos<0 || pos>endpoints.size())
    pos=endpoints.size();
  ptit=endpoints.begin()+pos;
  lenit=lengths.begin()+pos;
  endpoints.insert(ptit,newpoint);
  lengths.insert(lenit,0);
  pos--;
  if (pos<0)
    if (wasopen)
      pos=0;
    else
      pos+=endpoints.size();
  for (i=0;i<2;i++)
  {
    if (pos+1<endpoints.size())
      lengths[pos]=dist(endpoints[pos],endpoints[pos+1]);
    if (pos+1==endpoints.size() && !wasopen)
      lengths[pos]=dist(endpoints[pos],endpoints[0]);
    pos++;
    if (pos>=lengths.size())
      pos=0;
  }
}

void polyarc::insert(xy newpoint,int pos)
/* Same as polyline::insert for beginning, end, and empty cases.
 * In all other cases, newpoint is inserted into an arc, whose delta is split
 * proportionally to the distances to the adjacent points.
 */
{
  bool wasopen;
  vector<xy>::iterator ptit;
  vector<int>::iterator arcit;
  vector<double>::iterator lenit;
  wasopen=isopen();
  if (pos<0 || pos>endpoints.size())
    pos=endpoints.size();
  ptit=endpoints.begin()+pos;
  lenit=lengths.begin()+pos;
  arcit=deltas.begin()+pos;
  endpoints.insert(ptit,newpoint);
  deltas.insert(arcit,0);
  lengths.insert(lenit,0);
}

void polyline::setlengths()
{
  int i;
  for (i=0;i<lengths.size();i++)
    lengths[i]=getsegment(i).length();
}

void polyarc::setlengths()
{
  int i;
  assert(lengths.size()==deltas.size());
  for (i=0;i<deltas.size();i++)
    lengths[i]=getarc(i).length();
}

void polyarc::setdelta(int i,int delta)
{
  deltas[i%deltas.size()]=delta;
}

double polyline::length()
{
  int i;
  double len;
  for (len=i=0;i<lengths.size();i++)
    len+=lengths[i];
  return len;
}

double polyline::area()
{
  int i;
  xy startpnt;
  manysum a;
  if (endpoints.size())
    startpnt=endpoints[0];
  if (isopen())
    a+=NAN;
  else
    for (i=0;i<lengths.size();i++)
    {
      a+=area3(startpnt,endpoints[i],endpoints[(i+1)%endpoints.size()]);
    }
  return a.total();
}

double polyarc::area()
{
  int i;
  xy startpnt;
  manysum a;
  if (endpoints.size())
    startpnt=endpoints[0];
  if (isopen())
    a+=NAN;
  else
    for (i=0;i<lengths.size();i++)
    {
      a+=area3(startpnt,endpoints[i],endpoints[(i+1)%endpoints.size()]);
      a+=getarc(i).diffarea();
    }
  return a.total();
}

void polyline::open()
{
  lengths.resize(endpoints.size()-1);
}

void polyarc::open()
{
  deltas.resize(endpoints.size()-1);
  lengths.resize(endpoints.size()-1);
}

void polyspiral::open()
{
  curvatures.resize(endpoints.size()-1);
  clothances.resize(endpoints.size()-1);
  midpoints.resize(endpoints.size()-1);
  midbearings.resize(endpoints.size()-1);
  delta2s.resize(endpoints.size()-1);
  deltas.resize(endpoints.size()-1);
  lengths.resize(endpoints.size()-1);
}

void polyline::close()
{
  lengths.resize(endpoints.size());
}

void polyarc::close()
{
  deltas.resize(endpoints.size());
  lengths.resize(endpoints.size());
}

void polyspiral::close()
{
  curvatures.resize(endpoints.size());
  clothances.resize(endpoints.size());
  midpoints.resize(endpoints.size());
  midbearings.resize(endpoints.size());
  delta2s.resize(endpoints.size());
  deltas.resize(endpoints.size());
  lengths.resize(endpoints.size());
}

void polyspiral::insert(xy newpoint,int pos)
/* If there is one point after insertion and the polyspiral is closed:
 * Adds a line from the point to itself.
 * If there are two points after insertion and the polyspiral is open:
 * Adds a line from one point to the other.
 * If it's closed:
 * Adds two 180° arcs, making a circle.
 * If there are at least three points after insertion:
 * Updates the bearings of the new point and two adjacent points (unless
 * it is the first or last of an open polyspiral, in which case one) to
 * match an arc passing through three points, then updates the clothances
 * and curvatures of four consecutive spirals (unless the polyspiral is open
 * and the point is one of the first two or last two) to match the bearings.
 */
{
  bool wasopen;
  vector<xy>::iterator ptit,midit;
  vector<int>::iterator arcit,brgit,d2it,mbrit;
  vector<double>::iterator lenit,cloit,crvit;
  wasopen=isopen();
  if (pos<0 || pos>endpoints.size())
    pos=endpoints.size();
  ptit=endpoints.begin()+pos;
  midit=midpoints.begin()+pos;
  lenit=lengths.begin()+pos;
  arcit=deltas.begin()+pos;
  endpoints.insert(ptit,newpoint);
  deltas.insert(arcit,0);
  lengths.insert(lenit,0);
}

void polyspiral::setbear(int i)
{
  int h,j,prevbear,nextbear,avgbear;
  i%=endpoints.size();
  if (i<0)
    i+=endpoints.size();
  h=i-1;
  j=i+1;
  if (h<0)
    if (isopen())
      h+=3;
    else
      h+=endpoints.size();
  if (j>=endpoints.size())
    if (isopen())
      j-=3;
    else
      j-=endpoints.size();
  if (endpoints.size()==2)
    if (isopen())
      bearings[i]=dir(endpoints[0],endpoints[1]);
    else
      bearings[i]=dir(endpoints[i],endpoints[j])-DEG90;
  if (endpoints.size()>2)
  {
    bearings[i]=midarcdir(endpoints[h],endpoints[i],endpoints[j]);
    prevbear=bearings[h];
    nextbear=bearings[j];
    if (i==0)
      if (isopen())
	prevbear=nextbear;
      else
	prevbear+=DEG360;
    if (i==endpoints.size()-1)
      if (isopen())
	nextbear=prevbear;
      else
	nextbear+=DEG360;
    avgbear=prevbear+(nextbear-prevbear)/2;
    bearings[i]=avgbear+foldangle(bearings[i]-avgbear);
  }
}

void polyspiral::setspiral(int i)
{
  int j;
  spiralarc s;
  j=i+1;
  if (j>=endpoints.size())
    j=0;
  s=spiralarc(xyz(endpoints[i],elevation),xyz(endpoints[j],elevation));
  s.setdelta(bearings[j]-bearings[i]+DEG360*(j<i),bearings[j]+bearings[i]+DEG360*(j<i)-2*dir(endpoints[i],endpoints[j]));
  if (lengths[i]==0)
    cerr<<"length["<<i<<"]=0"<<endl;
  deltas[i]=s.getdelta();
  delta2s[i]=s.getdelta2();
  lengths[i]=s.length();
  if (std::isnan(lengths[i]))
    cerr<<"length["<<i<<"]=nan"<<endl;
  midbearings[i]=s.bearing(lengths[i]/2);
  midpoints[i]=s.station(lengths[i]/2);
  curvatures[i]=s.curvature(lengths[i]/2);
  clothances[i]=s.clothance();
}

void polyspiral::smooth()
{
  int i;
  for (i=0;i<endpoints.size();i++)
    setbear(i);
  for (i=0;i<lengths.size();i++)
    setspiral(i);
}
