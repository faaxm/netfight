/***
 * Copyright (C) Falko Axmann.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 ****/

#ifndef __mymath__
#define __mymath__

#include "util.h"

#define MYMATH_DIST_INF 999999

float calcRayLineIntersect(Vect2D rpos, Vect2D rdir, Vect2D lpos1, Vect2D lpos2);
int lineSphereIntersection(Vect2D p1, Vect2D p2, Vect2D m, float r);

#endif
