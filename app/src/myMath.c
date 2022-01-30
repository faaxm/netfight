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

#include "myMath.h"
#include "math.h"
#include "util.h"

float calcRayLineIntersect(Vect2D rpos, Vect2D rdir, Vect2D lpos1, Vect2D lpos2)
{
    float a, b; // a: dist along rdir, b: dist along ldir
    float ld1, ld2, lp1, lp2, rd1, rd2, rp1, rp2;

    ld1 = lpos1.x - lpos2.x;
    ld2 = lpos1.y - lpos2.y;
    lp1 = lpos1.x;
    lp2 = lpos1.y;

    rd1 = rdir.x;
    rd2 = rdir.y;
    rp1 = rpos.x;
    rp2 = rpos.y;

    a = -((ld2 * lp1 - ld1 * lp2 - ld2 * rp1 + ld1 * rp2) / ((-ld2) * rd1 + ld1 * rd2));
    b = -((lp2 * rd1 - lp1 * rd2 + rd2 * rp1 - rd1 * rp2) / (ld2 * rd1 - ld1 * rd2));

    if (b > -1 && b < 0)
        return a;
    else
        return MYMATH_DIST_INF + 100;
}

int lineSphereIntersection(Vect2D p1, Vect2D p2, Vect2D m, float r)
{
    float a1, a2;
    float b1, b2;
    float d;
    float f, g, h;
    float rt, t1, t2;

    b1 = p1.x - m.x;
    b2 = p1.y - m.y;
    d = vectDist(p1, p2);
    if (d != 0) {
        a1 = (p2.x - p1.x) / d;
        a2 = (p2.y - p1.y) / d;
    } else {
        a1 = a2 = 0.0f;
    }

    /* we don't want to div by 0 */
    if (a1 == 0 && a2 == 0) {
        return (vectDist(p1, m) < r);
    }

    f = a1 * a1 + a2 * a2;
    g = a1 * b1 + a2 * b2;
    h = b1 * b1 + b2 * b2;

    rt = (g * g) + (r * r - h) * f;
    if (rt > 0) {
        t1 = sqrt(rt) - g / f;
        t2 = -sqrt(rt) - g / f;
        return ((t1 < d && t1 >= 0) || (t2 < d && t2 >= 0));
    }

    return 0;
}
