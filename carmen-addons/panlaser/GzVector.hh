/*
 *  Gazebo - Outdoor Multi-Robot Simulator
 *  Copyright (C) 2003  
 *     Nate Koenig & Andrew Howard
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */
/* Desc: Vector classes and functions
 * Author: Andrew Howard
 * Date: 11 Jun 2003
 * CVS: $Id$
 */

#ifndef GZVECTOR_H_
#define GZVECTOR_H_

#include <math.h>
//#include "GzGlobal.hh"

// Basic 3 vector; also doubles for quaternions
class GzVector
{
  public: double u, x, y, z;
};


// Create a zero vector
GzVector GzVectorZero();

// Create a vector from the given values
GzVector GzVectorSet(double x, double y, double z);

// Add one vector to another (element by element)
GzVector GzVectorAdd(GzVector b, GzVector a);

// Subtract one vector from another (element by element)
GzVector GzVectorSub(GzVector b, GzVector a);

// Quatnernion
typedef GzVector GzQuatern;

/*
class GzQuatern
{
  public: double u, x, y, z;
};
*/

// Create an identity quaternion
GzQuatern GzQuaternIdent();

// Create a quaternion from elements
GzQuatern GzQuaternSet(double u, double x, double y, double z);

// Invert a quaternion
GzQuatern GzQuaternInverse(GzQuatern a);

// Create a quaternion from an axis and angle
GzQuatern GzQuaternFromAxis(double x, double y, double z, double a);

// Create a quaternion from Euler angles
GzQuatern GzQuaternFromEuler(double roll, double pitch, double yaw);

// Create a quaternion from a 3x3 rotation matrix
GzQuatern GzQuaternFromRot(double rot[9]);

// Convert quaternion to Euler angles
void GzQuaternToEuler(double *roll, double *pitch, double *yaw, GzQuatern q);

// Multiply two quaternions
GzQuatern GzQuaternMul(GzQuatern a, GzQuatern b);


// Test euler angle conversions
void GzQuaternTestEuler();

// Add one position to another: c = b + a
GzVector GzCoordPositionAdd(GzVector bpos, GzVector apos, GzQuatern arot);

// Subtract one position from another: c = b - a
GzVector GzCoordPositionSub(GzVector bpos, GzVector apos, GzQuatern arot);

// Add one rotation to another: c = b + a
GzQuatern GzCoordRotationAdd(GzQuatern b, GzQuatern a);

// Subtract one rotation from another: c = b - a
GzQuatern GzCoordRotationSub(GzQuatern b, GzQuatern a);


#endif
