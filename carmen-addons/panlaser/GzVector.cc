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

#include <stdio.h>
#include "GzVector.hh"


//////////////////////////////////////////////////////////////////////////////
// Create a zero vector
GzVector GzVectorZero()
{
  GzVector v;

  v.u = 0;
  v.x = 0;
  v.y = 0;
  v.z = 0;

  return v;
}


//////////////////////////////////////////////////////////////////////////////
// Create a vector from the given values
GzVector GzVectorSet(double x, double y, double z)
{
  GzVector v;

  v.u = 0;
  v.x = x;
  v.y = y;
  v.z = z;

  return v;
}


//////////////////////////////////////////////////////////////////////////////
// Add one vector to another (element by element)
// c = b + a
GzVector GzVectorAdd(GzVector b, GzVector a)
{
  return GzVectorSet(b.x + a.x, b.y + a.y, b.z + a.z);
}


//////////////////////////////////////////////////////////////////////////////
// Subtract one vector from another (element by element)
// c = b - a
GzVector GzVectorSub(GzVector b, GzVector a)
{
  return GzVectorSet(b.x - a.x, b.y - a.y, b.z - a.z);
}


//////////////////////////////////////////////////////////////////////////////
// Create a quaternion from elements
GzQuatern GzQuaternSet(double u, double x, double y, double z)
{
  GzQuatern q;

  q.u = u;
  q.x = x;
  q.y = y;
  q.z = z;

  return q;
}


//////////////////////////////////////////////////////////////////////////////
// Create an identity quaternion
GzQuatern GzQuaternIdent()
{
  GzQuatern q;

  q.u = 1.0;
  q.x = 0.0;
  q.y = 0.0;
  q.z = 0.0;
  
  return q;
}


//////////////////////////////////////////////////////////////////////////////
// Create a quaternion from an axis and angle
GzQuatern GzQuaternFromAxis(double x, double y, double z, double a)
{
  double l;
  GzQuatern q;

  l = x * x + y * y + z * z;
  
  if (l > 0.0)
  {
    a *= 0.5;
    l = sin(a) / sqrt(l);
    q.u = cos(a);
    q.x = x * l;
    q.y = y * l;
    q.z = z * l;
  }
  else
  {
    q.u = 1;
    q.x = 0;
    q.y = 0;
    q.z = 0;
  }
  return q;
}


//////////////////////////////////////////////////////////////////////////////
// Create a quaternion from Euler angles
GzQuatern GzQuaternFromEuler(double roll, double pitch, double yaw)
{
  double phi, the, psi;
  GzQuatern q, p;
  GzQuatern a, b, c;

  phi = roll / 2;
  the = pitch / 2;
  psi = yaw / 2;

  c = GzQuaternSet(cos(phi), sin(phi), 0, 0);
  b = GzQuaternSet(cos(the), 0, sin(the), 0);
  a = GzQuaternSet(cos(psi), 0, 0, sin(psi));
  
  q = GzQuaternMul(a, GzQuaternMul(b, c));
  
  p.u = cos(phi) * cos(the) * cos(psi) + sin(phi) * sin(the) * sin(psi);
  p.x = sin(phi) * cos(the) * cos(psi) - cos(phi) * sin(the) * sin(psi);
  p.y = cos(phi) * sin(the) * cos(psi) + sin(phi) * cos(the) * sin(psi);
  p.z = cos(phi) * cos(the) * sin(psi) - sin(phi) * sin(the) * sin(psi);

  //printf("q = %f %f %f %f\n", q.u, q.x, q.y, q.z);
  //printf("p = %f %f %f %f\n", p.u, p.x, p.y, p.z);
  
  return q;
}

//////////////////////////////////////////////////////////////////////////////
// Create a Quaternion from a 3x3 rotation matrix
GzQuatern GzQuaternFromRot( double rot[9] )
{
  int which;
  double temp;
  GzQuatern q;

  q.u = (1 + rot[0] + rot[4] + rot[8]) / 4;
  q.x = (1 + rot[0] - rot[4] - rot[8]) / 4;
  q.y = (1 - rot[0] + rot[4] - rot[8]) / 4;
  q.z = (1 - rot[0] - rot[4] + rot[8]) / 4;

  temp = q.u;
  which = 1;
  if (q.x > temp) {
    temp = q.x;
    which = 2;
  }
  if (q.y > temp) {
    temp = q.y;
    which = 3;
  }
  if (q.z > temp) {
    which = 4;
  }

  switch(which) {
    case 1:
      q.u = sqrt(q.u);
      temp = 1.0/(4.0*q.u);
      q.x = (rot[7] - rot[5]) * temp;
      q.y = (rot[2] - rot[6]) * temp;
      q.z = (rot[3] - rot[1]) * temp;
      break;
    case 2:
      q.x = sqrt(q.x);
      temp = 1.0/(4.0*q.x);
      q.u = (rot[7] - rot[5]) * temp;
      q.y = (rot[1] + rot[3]) * temp;
      q.z = (rot[2] + rot[6]) * temp;
      break;
    case 3:
      q.y = sqrt(q.y);
      temp = 1.0/(4.0*q.y);
      q.y = (rot[2] - rot[6]) * temp;
      q.x = (rot[1] + rot[3]) * temp;
      q.z = (rot[5] + rot[7]) * temp;
      break;
    case 4:
      q.z = sqrt(q.z);
      temp = 1.0/(4.0*q.z);
      q.u = (rot[3] - rot[1]) * temp;
      q.x = (rot[2] + rot[6]) * temp;
      q.y = (rot[5] + rot[7]) * temp;
      break;
  }

  return q;
}

//////////////////////////////////////////////////////////////////////////////
// Convert quaternion to Euler angles
void GzQuaternToEuler(double *roll, double *pitch, double *yaw, GzQuatern q)
{
  double phi, the, psi;
    
  phi = atan2(2 * (q.y*q.z + q.u*q.x), (q.u*q.u - q.x*q.x - q.y*q.y + q.z*q.z));
  the = asin(-2 * (q.x*q.z - q.u * q.y));
  psi = atan2(2 * (q.x*q.y + q.u*q.z), (q.u*q.u + q.x*q.x - q.y*q.y - q.z*q.z));

  *roll = phi;
  *pitch = the;
  *yaw = psi;
  
  return;
}


//////////////////////////////////////////////////////////////////////////////
// Invert a quaternion
GzQuatern GzQuaternInverse(GzQuatern a)
{
  GzQuatern b;

  b.u = a.u;
  b.x = -a.x;
  b.y = -a.y;
  b.z = -a.z;
  
  return b;
}


//////////////////////////////////////////////////////////////////////////////
// Multiple two quaternions
GzQuatern GzQuaternMul(GzQuatern a, GzQuatern b)
{
  GzQuatern c;

  /*
    (P*Q):h = P:h*Q:h - P:i*Q:i - P:j*Q:j - P:k*Q:k
    (P*Q):i = P:h*Q:i + P:i*Q:h + P:j*Q:k - P:k*Q:j
    (P*Q):j = P:h*Q:j - P:i*Q:k + P:j*Q:h + P:k*Q:i
    (P*Q):k = P:h*Q:k + P:i*Q:j - P:j*Q:i + P:k*Q:h
  */
  
  c.u = a.u * b.u - a.x * b.x - a.y * b.y - a.z * b.z;
  c.x = a.u * b.x + a.x * b.u + a.y * b.z - a.z * b.y;
  c.y = a.u * b.y - a.x * b.z + a.y * b.u + a.z * b.x;
  c.z = a.u * b.z + a.x * b.y - a.y * b.x + a.z * b.u;
  
  return c;
}


//////////////////////////////////////////////////////////////////////////////
// Test euler angle conversions
void GzQuaternTestEuler()
{
  int i;
  GzQuatern q;
  double roll, pitch, yaw;
  double angles[][3] =
    {
      {0, 0, 0},
      {80, 0, 0},
      {0, 80, 0},
      {0, 0, 80},
      {10, 20, 30},
      {30, 20, 10},
      {45, 45, 45},
      {-45, -45, -45},
    };
  
  for (i = 0; i < (int) (sizeof(angles) / sizeof(angles[0])); i++)
  {
    roll = angles[i][0] * M_PI / 180;
    pitch = angles[i][1] * M_PI / 180;
    yaw = angles[i][2] * M_PI / 180;

    //printf("Euler: %f %f %f\n", roll * 180 / M_PI, pitch * 180 / M_PI, yaw * 180 / M_PI);

    q = GzQuaternFromEuler(roll, pitch, yaw);
    GzQuaternToEuler(&roll, &pitch, &yaw, q);

    //printf("Euler: %f %f %f\n\n", roll * 180 / M_PI, pitch * 180 / M_PI, yaw * 180 / M_PI);
  }

  return;
}



//////////////////////////////////////////////////////////////////////////////
// Add one position to another: c = b + a
GzVector GzCoordPositionAdd(GzVector bpos, GzVector apos, GzQuatern arot)
{
  GzVector cpos;
  
  // cpos = apos + arot * bpos * arot!
  cpos = GzVectorAdd(apos, GzQuaternMul(arot, GzQuaternMul(bpos, GzQuaternInverse(arot))));

  return cpos;
}


//////////////////////////////////////////////////////////////////////////////
// Subtract one position from another: c = b - a
GzVector GzCoordPositionSub(GzVector bpos, GzVector apos, GzQuatern arot)
{
  GzVector cpos;

  // cpos = arot! * (bpos - apos) * arot
  cpos = GzQuaternMul(GzQuaternInverse(arot), GzQuaternMul(GzVectorSub(bpos, apos), arot));

  return cpos;
}


//////////////////////////////////////////////////////////////////////////////
// Add one rotation to another: c = b + a
GzQuatern GzCoordRotationAdd(GzQuatern b, GzQuatern a)
{
  GzQuatern c;

  // c = a * b
  c = GzQuaternMul(a, b);

  //printf("C:%f %f %f %f\n",c.u,c.y,c.x,c.z);
  return c;
}


//////////////////////////////////////////////////////////////////////////////
// Subtract one rotation from another: c = b - a
GzQuatern GzCoordRotationSub(GzQuatern b, GzQuatern a)
{
  GzQuatern c;

  // c = b * a!
  c = GzQuaternMul(b, GzQuaternInverse(a));

  return c;
}






