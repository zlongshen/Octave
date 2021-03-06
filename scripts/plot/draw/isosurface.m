## Copyright (C) 2009-2016 Martin Helm
##
## This file is part of Octave.
##
## Octave is free software; you can redistribute it and/or modify it
## under the terms of the GNU General Public License as published by
## the Free Software Foundation; either version 3 of the License, or (at
## your option) any later version.
##
## Octave is distributed in the hope that it will be useful, but
## WITHOUT ANY WARRANTY; without even the implied warranty of
## MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
## General Public License for more details.
##
## You should have received a copy of the GNU General Public License
## along with Octave; see the file COPYING.  If not, see
## <http://www.gnu.org/licenses/>.

## -*- texinfo -*-
## @deftypefn  {} {[@var{fv}] =} isosurface (@var{val}, @var{iso})
## @deftypefnx {} {[@var{fv}] =} isosurface (@var{val})
## @deftypefnx {} {[@var{fv}] =} isosurface (@var{x}, @var{y}, @var{z}, @var{val}, @var{iso})
## @deftypefnx {} {[@var{fv}] =} isosurface (@var{x}, @var{y}, @var{z}, @var{val})
## @deftypefnx {} {[@var{fvc}] =} isosurface (@dots{}, @var{col})
## @deftypefnx {} {[@var{fv}] =} isosurface (@dots{}, "noshare")
## @deftypefnx {} {[@var{fv}] =} isosurface (@dots{}, "verbose")
## @deftypefnx {} {[@var{f}, @var{v}] =} isosurface (@dots{})
## @deftypefnx {} {[@var{f}, @var{v}, @var{c}] =} isosurface (@dots{})
## @deftypefnx {} {} isosurface (@dots{})
##
## Calculate isosurface of 3-D data.
##
## If called with one output argument and the first input argument
## @var{val} is a three-dimensional array that contains the data of an
## isosurface geometry and the second input argument @var{iso} keeps the
## isovalue as a scalar value then return a structure array @var{fv}
## that contains the fields @var{faces} and @var{vertices} at computed
## points @command{[x, y, z] = meshgrid (1:l, 1:m, 1:n)}. The output
## argument @var{fv} can directly be taken as an input argument for the
## @command{patch} function.
##
## If @var{iso} is omitted or empty, a "good" value for an isosurface is
## determined from @var{val}.
##
## If called with further input arguments @var{x}, @var{y} and @var{z}
## which are three--dimensional arrays with the same size as @var{val} or
## vectors with lengths corresponding to the dimensions of @var{val}, then the
## volume data is taken at those given points. If @var{x}, @var{y} or @var{z}
## are empty, the grid corresponds to the indices in the respective direction
## (see @command{meshgrid}).
##
## If called with the input argument @var{col} which is a
## three-dimensional array of the same size as @var{val}, take
## those values for the interpolation of coloring the isosurface
## geometry. In this case, the structure array @var{fv} has the additional field
## @var{facevertexcdata}.
##
## If given the string input argument @qcode{"noshare"}, vertices might be
## returned multiple times for different faces. The default behavior is to
## search vertices shared by adjacent faces with @command{unique} which might be
## time consuming.  
## The string input argument @qcode{"verbose"} is only for compatibility and
## has no effect.  
## The string input arguments must be passed after the other arguments.
##
## If called with two or three output arguments, return the information about
## the faces @var{f}, vertices @var{v} and color data @var{c} as separate arrays
## instead of a single structure array.
##
## If called with no output argument, the isosurface geometry is directly
## processed with the @command{patch} command.
##
## For example,
##
## @example
## @group
## [x, y, z] = meshgrid (1:5, 1:5, 1:5);
## val = rand (5, 5, 5);
## isosurface (x, y, z, val, .5);
## @end group
## @end example
##
## @noindent
## will directly draw a random isosurface geometry in a graphics window.
##
## Another example for an isosurface geometry with different additional
## coloring
## @c Set example in small font to prevent overfull line
##
## @smallexample
## N = 15;    # Increase number of vertices in each direction
## iso = .4;  # Change isovalue to .1 to display a sphere
## lin = linspace (0, 2, N);
## [x, y, z] = meshgrid (lin, lin, lin);
## val = abs ((x-.5).^2 + (y-.5).^2 + (z-.5).^2);
## figure (); # Open another figure window
##
## subplot (2,2,1); view (-38, 20);
## [f, v] = isosurface (x, y, z, val, iso);
## p = patch ("Faces", f, "Vertices", v, "EdgeColor", "none");
## set (gca, "PlotBoxAspectRatioMode", "manual", ...
##           "PlotBoxAspectRatio", [1 1 1]);
## isonormals (x, y, z, val, p)
## set (p, "FaceColor", "green", "FaceLighting", "gouraud");
## light ("Position", [1 1 5]);
##
## subplot (2,2,2); view (-38, 20);
## p = patch ("Faces", f, "Vertices", v, "EdgeColor", "blue");
## set (gca, "PlotBoxAspectRatioMode", "manual", ...
##           "PlotBoxAspectRatio", [1 1 1]);
## isonormals (x, y, z, val, p)
## set (p, "FaceColor", "none", "EdgeLighting", "gouraud");
## light ("Position", [1 1 5]);
##
## subplot (2,2,3); view (-38, 20);
## [f, v, c] = isosurface (x, y, z, val, iso, y);
## p = patch ("Faces", f, "Vertices", v, "FaceVertexCData", c, ...
##            "FaceColor", "interp", "EdgeColor", "none");
## set (gca, "PlotBoxAspectRatioMode", "manual", ...
##           "PlotBoxAspectRatio", [1 1 1]);
## isonormals (x, y, z, val, p)
## set (p, "FaceLighting", "gouraud");
## light ("Position", [1 1 5]);
##
## subplot (2,2,4); view (-38, 20);
## p = patch ("Faces", f, "Vertices", v, "FaceVertexCData", c, ...
##            "FaceColor", "interp", "EdgeColor", "blue");
## set (gca, "PlotBoxAspectRatioMode", "manual", ...
##           "PlotBoxAspectRatio", [1 1 1]);
## isonormals (x, y, z, val, p)
## set (p, "FaceLighting", "gouraud");
## light ("Position", [1 1 5]);
## @end smallexample
##
## @seealso{isonormals, isocolors}
## @end deftypefn

## Author: Martin Helm <martin@mhelm.de>

## FIXME: Add support for string input argument "verbose" (needs changes to __marching_cube__.m)

function varargout = isosurface (varargin)

  if (nargin < 1 || nargin > 8 || nargout > 3)
    print_usage ();
  endif

  [x, y, z, val, iso, colors, noshare, verbose] = __get_check_isosurface_args__ (nargout, varargin{:});

  calc_colors = ! isempty (colors);
  if (calc_colors)
    [fvc.faces, fvc.vertices, fvc.facevertexcdata] = ...
      __marching_cube__ (x, y, z, val, iso, colors);
  else
    [fvc.faces, fvc.vertices] = __marching_cube__ (x, y, z, val, iso);
  endif

  if (isempty (fvc.vertices) || isempty (fvc.faces))
    warning ("isosurface: triangulation is empty");
  endif

  if (!noshare)
    [fvc.faces, fvc.vertices, J] = __unite_shared_vertices__ (fvc.faces, fvc.vertices);

    if (calc_colors)
      #fvc.facevertexcdata = fvc.facevertexcdata(vertices_idx);
      fvc.facevertexcdata(J) = []; # share very close vertices
    endif
  endif

  switch (nargout)
    case 0
      ## plot the calculated surface
      if (calc_colors)
        pa = patch ("Faces", fvc.faces, "Vertices", fvc.vertices,
                    "FaceVertexCData", fvc.facevertexcdata,
                    "FaceColor", "flat", "EdgeColor", "none");
      else
        pa = patch ("Faces", fvc.faces, "Vertices", fvc.vertices,
                    "FaceColor", "g", "EdgeColor", "k");
      endif
      if (! ishold ())
        set (gca (), "View", [-37.5, 30], "Box", "off");
      endif
    case 1
      varargout = {fvc};
    case 2
      varargout = {fvc.faces, fvc.vertices};
    otherwise ## 3
      varargout = {fvc.faces, fvc.vertices, fvc.facevertexcdata};
  endswitch

endfunction

function [x, y, z, data, iso, colors, noshare, verbose] = __get_check_isosurface_args__ (nout, varargin)
## get arguments from input and check values

  x = [];
  y = [];
  z = [];
  data = [];
  iso = [];
  colors = [];

  ## default values
  noshare = false;
  verbose = false;

  nin = length (varargin);
  num_string_inputs = 0;
  for i_arg = (nin:-1:nin-1)
    ## check whether last maximum 2 input arguments are strings and assign parameters
    if (!ischar (varargin{i_arg}) || i_arg < 1)
      ## string arguments must be at the end
      break
    end
    switch (tolower (varargin{i_arg}))
      case {"v", "verbose"}
        verbose = true;
        num_string_inputs++;
      case {"n", "noshare"}
        noshare = true;
        num_string_inputs++;
      otherwise
        error ("isosurface: parameter '%s' not supported", varargin{i_arg})
    endswitch
  endfor

  ## assign arguments
  switch (nin - num_string_inputs)
    case 1 ## isosurface (val, ...)
      data = varargin{1};
    case 2 ## isosurface (val, iso, ...) or isosurface (val, col, ...) 
      data = varargin{1};
      if (isscalar (varargin{2}) || isempty (varargin{2}))
        iso = varargin{2};
      else
        colors = varargin{2};
      endif
    case 3 ## isosurface (val, iso, col, ...) 
      data = varargin{1};
      iso = varargin{2};
      colors = varargin{3};
    case 4 ## isosurface (x, y, z, val, ...)
      x = varargin{1};
      y = varargin{2};
      z = varargin{3};
      data = varargin{4};
    case 5 ## isosurface (x, y, z, val, iso, ...) or isosurface (x, y, z, val, col, ...)
      x = varargin{1};
      y = varargin{2};
      z = varargin{3};
      data = varargin{4};
      if (isscalar (varargin{5}) || isempty (varargin{5}))
        iso = varargin{5};
      else
        colors = varargin{5};
      endif
    case 6 ## isosurface (x, y, z, val, iso, col, ...)
      x = varargin{1};
      y = varargin{2};
      z = varargin{3};
      data = varargin{4};
      iso = varargin{5};
      colors = varargin{6};
    otherwise
      error ("isosurface: wrong number of input arguments")
  endswitch

  ## check dimensions of data
  data_size = size (data);
  if (ndims (data) != 3 || any (data_size(1:3) < 2))
    error ("isosurface: VAL must be a non-singleton 3-dimensional matrix");
  endif

  if (isempty (x))
    x = 1:size (data, 2);
  endif
  if (isempty (y))
    y = 1:size (data, 1);
  endif
  if (isempty (z))
    z = 1:size (data, 3);
  endif

  ## check x
  if (isvector (x) && length (x) == data_size(2))
    x = repmat (x(:)', [data_size(1) 1 data_size(3)]);
  elseif (! size_equal (data, x))
    error ("isosurface: X must match the size of VAL");
  endif

  ## check y
  if (isvector (y) && length (y) == data_size(1))
    y = repmat (y(:), [1 data_size(2) data_size(3)]);
  elseif (! size_equal (data, y))
    error ("isosurface: Y must match the size of VAL");
  endif

  ## check z
  if (isvector (z) && length (z) == data_size(3))
    z = repmat (reshape (z(:), [1 1 length(z)]), [data_size(1) data_size(2) 1]);
  elseif (! size_equal (data, z))
    error ("isosurface: Z must match the size of VAL");
  endif

  ## check iso
  if (isempty (iso))
    ## calculate "good" iso value from data
    iso = __calc_isovalue_from_data__ (data);
  endif

  if ~isscalar (iso)
    error ("isosurface: ISO must be a scalar")
  endif

  ## check colors
  if (! isempty (colors))
    if (! size_equal (data, colors))
      error ("isosurface: COL must match the size of VAL")
    endif
    if (nout == 2)
      warning ("isosurface: colors will be calculated, but no output argument to receive it.");
    endif
  elseif (nout == 3)
    error ("isosurface: COL must be passed to return C")
  endif

endfunction


%!demo
%! clf;
%! [x,y,z] = meshgrid (-2:0.5:2, -2:0.5:2, -2:0.5:2);
%! val = x.^2 + y.^2 + z.^2;
%! isosurface (x, y, z, val, 1);
%! axis equal;
%! title ('isosurface of a sphere');

%!demo
%! x = 0:2;
%! y = 0:3;
%! z = 0:1;
%! [xx, yy, zz]  = meshgrid (x, y, z);
%! val        = [0, 0, 0; 0, 0, 0; 0, 0, 1; 0, 0, 1];
%! val(:,:,2) = [0, 0, 0; 0, 0, 1; 0, 1, 2; 0, 1, 2];
%! iso = 0.8;
%% three arguments, no output
%! figure;
%! subplot (2, 2, 1); isosurface (val, iso, yy); view(3)
%% six arguments, no output (x, y, z are vectors)
%! subplot (2, 2, 2); isosurface (x, y, z, val, iso, yy); view (3)
%% six arguments, no output (x, y, z are matrices)
%! subplot (2, 2, 3); isosurface (xx, yy, zz, val, iso, yy); view (3)
%% six arguments, no output (mixed x, y, z) and option "noshare"
%! subplot (2, 2, 4); isosurface (x, yy, z, val, iso, yy, "noshare"); view (3)
%! annotation("textbox", [0 0.95 1 0.1], ...
%!    "String", ["Apart from the first plot having a different scale, " ...
%!               "all four plots must look the same.\n" ...
%!               "The last plot might have different colors but must have " ...
%!               "the same shape."], ...
%!    "HorizontalAlignment", "left");

%!shared x, y, z, xx, yy, zz, val, iso
%! x = 0:2;
%! y = 0:3;
%! z = 0:1;
%! [xx, yy, zz]  = meshgrid (x, y, z);
%! val        = [0, 0, 0; 0, 0, 0; 0, 0, 1; 0, 0, 1];
%! val(:,:,2) = [0, 0, 0; 0, 0, 1; 0, 1, 2; 0, 1, 2];
%! iso = 0.8;

## one argument, one output
%!test
%! fv = isosurface (val);
%! assert (isfield (fv, "vertices"), true);
%! assert (isfield (fv, "faces"), true);
%! assert (size (fv.vertices), [5 3]);
%! assert (size (fv.faces), [3 3]);

## two arguments (second is ISO), one output
%!test
%! fv = isosurface (val, iso);
%! assert (isfield (fv, "vertices"), true);
%! assert (isfield (fv, "faces"), true);
%! assert (size (fv.vertices), [11 3]);
%! assert (size (fv.faces), [10 3]);

## two arguments (second is COL), one output
%!test
%! fvc = isosurface (val, yy);
%! assert (isfield (fvc, "vertices"), true);
%! assert (isfield (fvc, "faces"), true);
%! assert (isfield (fvc, "facevertexcdata"), true);
%! assert (size (fvc.vertices), [5 3]);
%! assert (size (fvc.faces), [3 3]);
%! assert (size (fvc.facevertexcdata), [5 1]);

## three arguments, one output
%!test
%! fvc = isosurface (val, iso, yy);
%! assert (isfield (fvc, "vertices"), true);
%! assert (isfield (fvc, "faces"), true);
%! assert (isfield (fvc, "facevertexcdata"), true);
%! assert (size (fvc.vertices), [11 3]);
%! assert (size (fvc.faces), [10 3]);
%! assert (size (fvc.facevertexcdata), [11 1]);

## four arguments, one output
%!test
%! fv = isosurface (x, [], z, val);
%! assert (isfield (fv, "vertices"), true);
%! assert (isfield (fv, "faces"), true);
%! assert (size (fv.vertices), [5 3]);
%! assert (size (fv.faces), [3 3]);

## five arguments (fifth is ISO), one output
%!test
%! fv = isosurface (xx, y, [], val, iso);
%! assert (isfield (fv, "vertices"), true);
%! assert (isfield (fv, "faces"), true);
%! assert (size (fv.vertices), [11 3]);
%! assert (size (fv.faces), [10 3]);

## five arguments (fifth is COL), one output
%!test
%! fvc = isosurface ([], yy, z, val, yy);
%! assert (isfield (fvc, "vertices"), true);
%! assert (isfield (fvc, "faces"), true);
%! assert (isfield (fvc, "facevertexcdata"), true);
%! assert (size (fvc.vertices), [5 3]);
%! assert (size (fvc.faces), [3 3]);
%! assert (size (fvc.facevertexcdata), [5 1]);

## six arguments, one output
%!test
%! fvc = isosurface (xx, yy, zz, val, iso, yy);
%! assert (isfield (fvc, "vertices"), true);
%! assert (isfield (fvc, "faces"), true);
%! assert (isfield (fvc, "facevertexcdata"), true);
%! assert (size (fvc.vertices), [11 3]);
%! assert (size (fvc.faces), [10 3]);
%! assert (size (fvc.facevertexcdata), [11 1]);

## five arguments (fifth is ISO), two outputs
%!test
%! [f, v] = isosurface (x, y, z, val, iso);
%! assert (size (f), [10 3]);
%! assert (size (v), [11 3]);

## six arguments, three outputs
%!test
%! [f, v, c] = isosurface (x, y, z, val, iso, yy);
%! assert (size (f), [10 3]);
%! assert (size (v), [11 3]);
%! assert (size (c), [11 1]);

## two arguments (second is ISO) and one string, one output
%!test
%! fv = isosurface (val, iso, "verbose");
%! assert (isfield (fv, "vertices"), true);
%! assert (isfield (fv, "faces"), true);
%! assert (size (fv.vertices), [11 3]);
%! assert (size (fv.faces), [10 3]);

## six arguments and two strings, one output
%!test
%! fvc = isosurface (xx, yy, zz, val, iso, yy, "v", "noshare");
%! assert (isfield (fvc, "vertices"), true);
%! assert (isfield (fvc, "faces"), true);
%! assert (isfield (fvc, "facevertexcdata"), true);
%! assert (size (fvc.vertices), [20 3]);
%! assert (size (fvc.faces), [10 3]);
%! assert (size (fvc.facevertexcdata), [20 1]);

## five arguments (fifth is COL) and two strings (different order), one output
%!test
%! fvc = isosurface (xx, yy, zz, val, yy, "n", "v");
%! assert (isfield (fvc, "vertices"), true);
%! assert (isfield (fvc, "faces"), true);
%! assert (isfield (fvc, "facevertexcdata"), true);
%! assert (size (fvc.vertices), [7 3]);
%! assert (size (fvc.faces), [3 3]);
%! assert (size (fvc.facevertexcdata), [7 1]);

## test for each error and warning
%!test
%!error <X must match the size of VAL> x = 1:2:24; fvc = isosurface (x, y, z, val, iso);
%!error <Y must match the size of VAL> y = -14:6:11; fvc = isosurface (x, y, z, val, iso);
%!error <Z must match the size of VAL> z = linspace (16, 18, 5); fvc = isosurface (x, y, z, val, iso);
%!error <X must match the size of VAL> x = 1:2:24; [xx, yy, zz] = meshgrid (x, y, z); fvc = isosurface (xx, yy, zz, val, iso);
%!error <X must match the size of VAL> y = -14:6:11; [xx, yy, zz] = meshgrid (x, y, z); fvc = isosurface (xx, yy, zz, val, iso);
%!error <X must match the size of VAL> z = linspace (16, 18, 3); [xx, yy, zz] = meshgrid (x, y, z); fvc = isosurface (xx, yy, zz, val, iso);
%!error <VAL must be a non-singleton 3-dimensional matrix> val = reshape(1:6*8, [6 8]); fvc = isosurface (val, iso);
%!error <VAL must be a non-singleton 3-dimensional matrix> val = reshape(1:6*8, [6 1 8]); fvc = isosurface (val, iso);
%!error <ISO must be a scalar> fvc = isosurface (val, [iso iso], yy);
%!error <COL must match the size of VAL> fvc = isosurface (val, [iso iso]);
%!warning <colors will be calculated, but no output argument to receive it.> [f, v] = isosurface (val, iso, yy);
%!error <COL must be passed to return C> [f, v, c] = isosurface (val, iso);
%!error <Invalid call to isosurface> fvc = isosurface ();
%!error <Invalid call to isosurface> [f, v, c, a] = isosurface (val, iso);
%!error <wrong number of input arguments> fvc = isosurface (xx, yy, zz, val, iso, yy, 5);
%!error <parameter 'test_string' not supported> fvc = isosurface (val, iso, "test_string");

## test for __calc_isovalue_from_data__
## FIXME: private function cannot be tested, unless bug #38776 is resolved.
%!xtest
%! assert (__calc_isovalue_from_data__ (1:5), 3.02);
