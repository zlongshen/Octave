// MArray-d.cc                                            -*- C++ -*-
/*

Copyright (C) 1993, 1994, 1995 John W. Eaton

This file is part of Octave.

Octave is free software; you can redistribute it and/or modify it
under the terms of the GNU General Public License as published by the
Free Software Foundation; either version 2, or (at your option) any
later version.

Octave is distributed in the hope that it will be useful, but WITHOUT
ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
for more details.

You should have received a copy of the GNU General Public License
along with Octave; see the file COPYING.  If not, write to the Free
Software Foundation, 675 Mass Ave, Cambridge, MA 02139, USA.

*/

// Instantiate MArrays of double values.

#include "MArray.h"
#include "MArray.cc"

template class MArray<double>;
template class MArray2<double>;
template class MDiagArray<double>;

template MArray<double>
operator + (const MArray<double>& a, const double& s);

template MArray<double>
operator - (const MArray<double>& a, const double& s);

template MArray<double>
operator * (const MArray<double>& a, const double& s);

template MArray<double>
operator / (const MArray<double>& a, const double& s);

template MArray<double>
operator + (const double& s, const MArray<double>& a);

template MArray<double>
operator - (const double& s, const MArray<double>& a);

template MArray<double>
operator * (const double& s, const MArray<double>& a);

template MArray<double>
operator / (const double& s, const MArray<double>& a);

template MArray<double>
operator + (const MArray<double>& a, const MArray<double>& b);

template MArray<double>
operator - (const MArray<double>& a, const MArray<double>& b);

template MArray<double>
product (const MArray<double>& a, const MArray<double>& b); 

template MArray<double>
quotient (const MArray<double>& a, const MArray<double>& b);

template MArray<double>
operator - (const MArray<double>& a);

template MArray2<double>
operator + (const MArray2<double>& a, const double& s);

template MArray2<double>
operator - (const MArray2<double>& a, const double& s);

template MArray2<double>
operator * (const MArray2<double>& a, const double& s);

template MArray2<double>
operator / (const MArray2<double>& a, const double& s);

template MArray2<double>
operator + (const double& s, const MArray2<double>& a);

template MArray2<double>
operator - (const double& s, const MArray2<double>& a);

template MArray2<double>
operator * (const double& s, const MArray2<double>& a);

template MArray2<double>
operator / (const double& s, const MArray2<double>& a);

template MArray2<double>
operator + (const MArray2<double>& a, const MArray2<double>& b);

template MArray2<double>
operator - (const MArray2<double>& a, const MArray2<double>& b);

template MArray2<double>
product (const MArray2<double>& a, const MArray2<double>& b);

template MArray2<double>
quotient (const MArray2<double>& a, const MArray2<double>& b);

template MArray2<double>
operator - (const MArray2<double>& a);

template MDiagArray<double>
operator * (const MDiagArray<double>& a, const double& s);

template MDiagArray<double>
operator / (const MDiagArray<double>& a, const double& s);

template MDiagArray<double>
operator * (const double& s, const MDiagArray<double>& a);

template MDiagArray<double>
operator + (const MDiagArray<double>& a, const MDiagArray<double>& b);

template MDiagArray<double>
operator - (const MDiagArray<double>& a, const MDiagArray<double>& b);

template MDiagArray<double>
product (const MDiagArray<double>& a, const MDiagArray<double>& b);

template MDiagArray<double>
operator - (const MDiagArray<double>& a);

/*
;;; Local Variables: ***
;;; mode: C++ ***
;;; page-delimiter: "^/\\*" ***
;;; End: ***
*/
