/******************************************************/
/*                                                    */
/* curvefit.h - fit polyarc/alignment to points       */
/*                                                    */
/******************************************************/
/* Copyright 2022 Pierre Abbat.
 * This file is part of Bezitopo.
 *
 * Bezitopo is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as
 * published by the Free Software Foundation, either version 3 of the
 * License, or (at your option) any later version.
 *
 * Bezitopo is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License and Lesser General Public License
 * for more details.
 *
 * You should have received a copy of the GNU General Public License
 * and Lesser General Public License along with Bezitopo. If not, see
 * <http://www.gnu.org/licenses/>.
 */

#include <vector>
#include "circle.h"
#include "polyline.h"
#include "manyarc.h"

struct FitRec
{
  double startOff;
  std::vector<xy> endpoints;
  double endOff;
  double startCur; // used only when fitting a polyspiral
  int startBear;
  double shortDist(Circle startLine,Circle endLine) const;
};

std::vector<double> curvefitResiduals(polyarc q,std::vector<xy> points);

/* Fits a polyarc to the points. The initial polyarc is formed by fitting
 * a spiralarc to the midpoints of startLine and endLine perpendicular to both,
 * then approximating it with two arcs. If there are hints, it starts with
 * one more spiralarc than hints. The returned polyarc will start on startLine
 * and end on endLine, but the endpoints on the hints will probably be moved
 * off them.
 */
polyarc fitPolyarc(Circle startLine,std::vector<xy> points,Circle endLine,double toler,std::vector<Circle> hints=std::vector<Circle>());