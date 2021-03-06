/*

Copyright (C) 2016 John W. Eaton

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

// The fpucw macros are provided by gnulib.  We don't include gnulib
// headers directly in Octave's C++ source files to avoid problems that
// may be caused by the way that gnulib overrides standard library
// functions.

#if defined (HAVE_CONFIG_H)
#  include "config.h"
#endif

#if defined (HAVE_FPU_CONTROL_H)
#  include <fpu_control.h>
#endif

#include "fpucw.h"

#include "fpucw-wrapper.h"

#if ! defined (_FPU_DEFAULT)
#  if defined __i386__ || defined __x86_64__
#    define _FPU_DEFAULT 0x037f
#  else
#    define _FPU_DEFAULT 0
#  endif
#endif

void
octave_set_default_fpucw (void)
{
  fpucw_t cw = GET_FPUCW ();

  if (cw != _FPU_DEFAULT)
    SET_FPUCW (_FPU_DEFAULT);
}
