/*

Copyright (C) 2004-2015 John W. Eaton

This file is part of Octave.

Octave is free software; you can redistribute it and/or modify it
under the terms of the GNU General Public License as published by the
Free Software Foundation; either version 3 of the License, or (at your
option) any later version.

Octave is distributed in the hope that it will be useful, but WITHOUT
ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
for more details.

You should have received a copy of the GNU General Public License
along with Octave; see the file COPYING.  If not, see
<http://www.gnu.org/licenses/>.

*/

#if defined (HAVE_CONFIG_H)
#  include "config.h"
#endif

#include <iostream>
#include <limits>

#include "lo-ieee.h"
#include "lo-utils.h"
#include "mx-base.h"
#include "quit.h"

#include "defun.h"
#include "errwarn.h"
#include "ovl.h"
#include "oct-lvalue.h"
#include "oct-hdf5.h"
#include "ops.h"
#include "ov-base.h"

#if defined (HAVE_HDF5)
#  define HDF5_SAVE_TYPE H5T_NATIVE_UINT8
#endif

#include "ov-base-int.h"
#include "ov-base-int.cc"
#include "ov-uint8.h"
#include "ov-type-conv.h"
#include "pr-output.h"
#include "variables.h"

#include "byte-swap.h"
#include "ls-oct-text.h"
#include "ls-utils.h"
#include "ls-hdf5.h"

// Prevent implicit instantiations on some systems (Windows, others?)
// that can lead to duplicate definitions of static data members.

extern template class OCTINTERP_API octave_base_scalar<double>;

template class octave_base_matrix<uint8NDArray>;

template class octave_base_int_matrix<uint8NDArray>;

DEFINE_OV_TYPEID_FUNCTIONS_AND_DATA (octave_uint8_matrix,
                                     "uint8 matrix", "uint8");

template class octave_base_scalar<octave_uint8>;

template class octave_base_int_scalar<octave_uint8>;

DEFINE_OV_TYPEID_FUNCTIONS_AND_DATA (octave_uint8_scalar,
                                     "uint8 scalar", "uint8");

DEFUN (uint8, args, ,
       doc: /* -*- texinfo -*-
@deftypefn {} {} uint8 (@var{x})
Convert @var{x} to unsigned 8-bit integer type.
@seealso{int8, int16, uint16, int32, uint32, int64, uint64}
@end deftypefn */)
{
  OCTAVE_TYPE_CONV_BODY (uint8);
}

/*
%!assert (class (uint8 (1)), "uint8")
%!assert (uint8 (1.25), uint8 (1))
%!assert (uint8 (1.5), uint8 (2))
%!assert (uint8 (-1.5), uint8 (0))
%!assert (uint8 (2^9), uint8 (2^8-1))
%!assert (uint8 (-2^9), uint8 (0))
*/
