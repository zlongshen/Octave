# Copyright (C) 1993, 1994, 1995 John W. Eaton
# 
# This file is part of Octave.
# 
# Octave is free software; you can redistribute it and/or modify it
# under the terms of the GNU General Public License as published by the
# Free Software Foundation; either version 2, or (at your option) any
# later version.
# 
# Octave is distributed in the hope that it will be useful, but WITHOUT
# ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
# FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
# for more details.
# 
# You should have received a copy of the GNU General Public License
# along with Octave; see the file COPYING.  If not, write to the Free
# Software Foundation, 675 Mass Ave, Cambridge, MA 02139, USA.

function shg ()

# usage: shg
#
# Show the graph window.  Currently, this is the same as executing
# replot without any arguments.
#
# See also: plot, semilogx, semilogy, loglog, polar, mesh, contour,
#           bar, stairs, gplot, gsplot, replot, xlabel, ylabel

  if (nargin != 0)
    warning ("shg: ignoring extra arguments");
  endif

  replot

endfunction
