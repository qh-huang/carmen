/* coords.h - include file for coords routines
**
** Copyright © 2001 by Jef Poskanzer <jef@acme.com>.
** All rights reserved.
**
** Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions
** are met:
** 1. Redistributions of source code must retain the above copyright
**    notice, this list of conditions and the following disclaimer.
** 2. Redistributions in binary form must reproduce the above copyright
**    notice, this list of conditions and the following disclaimer in the
**    documentation and/or other materials provided with the distribution.
**
** THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
** ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
** IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
** ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
** FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
** DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
** OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
** HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
** LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
** OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
** SUCH DAMAGE.
*/

#ifndef COORDS_H
#define COORDS_H

#define K0 0.9996

/* WGS-84 */
#define EquatorialRadius 6378137
#define EccentricitySquared 0.00669438

/* Other datums which could be used here instead:
**
** Name				EquatorialRadius	EccentricitySquared
** ----				----------------	-------------------
** Airy				6377563			0.00667054
** Australian National		6378160			0.006694542
** Bessel 1841			6377397			0.006674372
** Bessel 1841 (Nambia)		6377484			0.006674372
** Clarke 1866			6378206			0.006768658
** Clarke 1880			6378249			0.006803511
** Everest			6377276			0.006637847
** Fischer 1960 (Mercury) 	6378166			0.006693422
** Fischer 1968			6378150			0.006693422
** GRS 1967			6378160			0.006694605
** GRS 1980			6378137			0.00669438
** Helmert 1906			6378200			0.006693422
** Hough			6378270			0.00672267
** International		6378388			0.00672267
** Krassovsky			6378245			0.006693422
** Modified Airy		6377340			0.00667054
** Modified Everest		6377304			0.006637847
** Modified Fischer 1960	6378155			0.006693422
** South American 1969		6378160			0.006694542
** WGS 60			6378165			0.006693422
** WGS 66			6378145			0.006694542
** WGS-72			6378135			0.006694318
*/

#endif /* COORDS_H */
