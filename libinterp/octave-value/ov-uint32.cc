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
#  define HDF5_SAVE_TYPE H5T_NATIVE_UINT32
#endif

#include "ov-base-int.h"
#include "ov-base-int.cc"
#include "ov-uint32.h"
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

template class octave_base_matrix<uint32NDArray>;

template class octave_base_int_matrix<uint32NDArray>;

DEFINE_OV_TYPEID_FUNCTIONS_AND_DATA (octave_uint32_matrix,
                                     "uint32 matrix", "uint32");

template class octave_base_scalar<octave_uint32>;

template class octave_base_int_scalar<octave_uint32>;

DEFINE_OV_TYPEID_FUNCTIONS_AND_DATA (octave_uint32_scalar,
                                     "uint32 scalar", "uint32");

DEFUN (uint32, args, ,
       doc: /* -*- texinfo -*-
@deftypefn {} {} uint32 (@var{x})
Convert @var{x} to unsigned 32-bit integer type.
@seealso{int8, uint8, int16, uint16, int32, int64, uint64}
@end deftypefn */)
{
  OCTAVE_TYPE_CONV_BODY (uint32);
}

/*
%!assert (class (uint32 (1)), "uint32")
%!assert (uint32 (1.25), uint32 (1))
%!assert (uint32 (1.5), uint32 (2))
%!assert (uint32 (-1.5), uint32 (0))
%!assert (uint32 (2^33), uint32 (2^32-1))
%!assert (uint32 (-2^33), uint32 (0))
*/
