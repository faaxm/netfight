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

#ifndef __myutil__
#define __myutil__

#define kPI 3.14159

#define sqrNum(a) ((a) * (a))
#define vectLen(a) (sqrt(sqrNum(a.x) + sqrNum(a.y)))
#define vectDist(a, b) (sqrt(sqrNum(a.x - b.x) + sqrNum(a.y - b.y)))
#define numCloseTo(a, b, r) (((a) - (r) < (b) && (a) + (r) > (b)) ? 1 : 0)

typedef struct Vect2D {
    float x, y;
} Vect2D;

typedef struct ColorRGB {
    float r, g, b;
} ColorRGB;

#endif
