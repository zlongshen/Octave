/*

Copyright (C) 1996-2015 John W. Eaton
Copyright (C) 2008-2009 Jaroslav Hajek
Copyright (C) 2008-2009 VZLU Prague

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

#include "qr.h"
#include "qrp.h"
#include "sparse-qr.h"

#include "defun-dld.h"
#include "error.h"
#include "errwarn.h"
#include "ovl.h"
#include "utils.h"

template <typename MT>
static octave_value
get_qr_r (const qr<MT>& fact)
{
  MT R = fact.R ();
  if (R.is_square () && fact.regular ())
    return octave_value (R, MatrixType (MatrixType::Upper));
  else
    return R;
}

template <typename T>
static typename qr<T>::type
qr_type (int nargin, int nargout)
{
  return ((nargout == 0 || nargout == 1)
          ? qr<T>::raw
          : (nargin == 2) ? qr<T>::economy : qr<T>::std);
}

// [Q, R] = qr (X):      form Q unitary and R upper triangular such
//                        that Q * R = X
//
// [Q, R] = qr (X, 0):    form the economy decomposition such that if X is
//                        m by n then only the first n columns of Q are
//                        computed.
//
// [Q, R, P] = qr (X):    form QRP factorization of X where
//                        P is a permutation matrix such that
//                        A * P = Q * R
//
// [Q, R, P] = qr (X, 0): form the economy decomposition with
//                        permutation vector P such that Q * R = X (:, P)
//
// qr (X) alone returns the output of the LAPACK routine dgeqrf, such
// that R = triu (qr (X))

DEFUN_DLD (qr, args, nargout,
           doc: /* -*- texinfo -*-
@deftypefn  {} {[@var{Q}, @var{R}, @var{P}] =} qr (@var{A})
@deftypefnx {} {[@var{Q}, @var{R}, @var{P}] =} qr (@var{A}, '0')
@deftypefnx {} {[@var{C}, @var{R}] =} qr (@var{A}, @var{B})
@deftypefnx {} {[@var{C}, @var{R}] =} qr (@var{A}, @var{B}, '0')
@cindex QR factorization
Compute the QR@tie{}factorization of @var{A}, using standard @sc{lapack}
subroutines.

For example, given the matrix @code{@var{A} = [1, 2; 3, 4]},

@example
[@var{Q}, @var{R}] = qr (@var{A})
@end example

@noindent
returns

@example
@group
@var{Q} =

  -0.31623  -0.94868
  -0.94868   0.31623

@var{R} =

  -3.16228  -4.42719
   0.00000  -0.63246
@end group
@end example

The @code{qr} factorization has applications in the solution of least
squares problems
@tex
$$
\min_x \left\Vert A x - b \right\Vert_2
$$
@end tex
@ifnottex

@example
min norm(A x - b)
@end example

@end ifnottex
for overdetermined systems of equations (i.e.,
@tex
$A$
@end tex
@ifnottex
@var{A}
@end ifnottex
is a tall, thin matrix).  The QR@tie{}factorization is
@tex
$QR = A$ where $Q$ is an orthogonal matrix and $R$ is upper triangular.
@end tex
@ifnottex
@code{@var{Q} * @var{R} = @var{A}} where @var{Q} is an orthogonal matrix and
@var{R} is upper triangular.
@end ifnottex

If given a second argument of @qcode{'0'}, @code{qr} returns an
economy-sized QR@tie{}factorization, omitting zero rows of @var{R} and the
corresponding columns of @var{Q}.

If the matrix @var{A} is full, the permuted QR@tie{}factorization
@code{[@var{Q}, @var{R}, @var{P}] = qr (@var{A})} forms the
QR@tie{}factorization such that the diagonal entries of @var{R} are
decreasing in magnitude order.  For example, given the matrix
@code{a = [1, 2; 3, 4]},

@example
[@var{Q}, @var{R}, @var{P}] = qr (@var{A})
@end example

@noindent
returns

@example
@group
@var{Q} =

  -0.44721  -0.89443
  -0.89443   0.44721

@var{R} =

  -4.47214  -3.13050
   0.00000   0.44721

@var{P} =

   0  1
   1  0
@end group
@end example

The permuted @code{qr} factorization
@code{[@var{Q}, @var{R}, @var{P}] = qr (@var{A})} factorization allows the
construction of an orthogonal basis of @code{span (A)}.

If the matrix @var{A} is sparse, then compute the sparse
QR@tie{}factorization of @var{A}, using @sc{CSparse}.  As the matrix @var{Q}
is in general a full matrix, this function returns the @var{Q}-less
factorization @var{R} of @var{A}, such that
@code{@var{R} = chol (@var{A}' * @var{A})}.

If the final argument is the scalar @code{0} and the number of rows is
larger than the number of columns, then an economy factorization is
returned.  That is @var{R} will have only @code{size (@var{A},1)} rows.

If an additional matrix @var{B} is supplied, then @code{qr} returns
@var{C}, where @code{@var{C} = @var{Q}' * @var{B}}.  This allows the
least squares approximation of @code{@var{A} \ @var{B}} to be calculated
as

@example
@group
[@var{C}, @var{R}] = qr (@var{A}, @var{B})
x = @var{R} \ @var{C}
@end group
@end example
@seealso{chol, hess, lu, qz, schur, svd, qrupdate, qrinsert, qrdelete, qrshift}
@end deftypefn */)
{
  int nargin = args.length ();

  if (nargin < 1 || nargin > (args(0).is_sparse_type () ? 3 : 2))
    print_usage ();

  octave_value_list retval;

  octave_value arg = args(0);

  int arg_is_empty = empty_arg ("qr", arg.rows (), arg.columns ());

  if (arg_is_empty < 0)
    return retval;

  if (arg.is_sparse_type ())
    {
      bool economy = false;
      bool is_cmplx = false;
      int have_b = 0;

      if (arg.is_complex_type ())
        is_cmplx = true;
      if (nargin > 1)
        {
          have_b = 1;
          if (args(nargin-1).is_scalar_type ())
            {
              int val = args(nargin-1).int_value ();
              if (val == 0)
                {
                  economy = true;
                  have_b = (nargin > 2 ? 2 : 0);
                }
            }
          if (have_b > 0 && args(have_b).is_complex_type ())
            is_cmplx = true;
        }

      if (is_cmplx)
        {
          sparse_qr<SparseComplexMatrix> q (arg.sparse_complex_matrix_value ());

          if (have_b > 0)
            {
              retval = ovl (q.C (args(have_b).complex_matrix_value ()),
                            q.R (economy));
              if (arg.rows () < arg.columns ())
                warning ("qr: non minimum norm solution for under-determined problem");
            }
          else if (nargout > 1)
            retval = ovl (q.Q (), q.R (economy));
          else
            retval = ovl (q.R (economy));
        }
      else
        {
          sparse_qr<SparseMatrix> q (arg.sparse_matrix_value ());

          if (have_b > 0)
            {
              retval = ovl (q.C (args(have_b).matrix_value ()), q.R (economy));
              if (args(0).rows () < args(0).columns ())
                warning ("qr: non minimum norm solution for under-determined problem");
            }
          else if (nargout > 1)
            retval = ovl (q.Q (), q.R (economy));
          else
            retval = ovl (q.R (economy));
        }
    }
  else
    {
      if (arg.is_single_type ())
        {
          if (arg.is_real_type ())
            {
              qr<FloatMatrix>::type type
                = qr_type<FloatMatrix> (nargin, nargout);

              FloatMatrix m = arg.float_matrix_value ();

              switch (nargout)
                {
                case 0:
                case 1:
                  {
                    qr<FloatMatrix> fact (m, type);
                    retval = ovl (fact.R ());
                  }
                  break;

                case 2:
                  {
                    qr<FloatMatrix> fact (m, type);
                    retval = ovl (fact.Q (), get_qr_r (fact));
                  }
                  break;

                default:
                  {
                    qrp<FloatMatrix> fact (m, type);

                    if (type == qr<FloatMatrix>::economy)
                      retval = ovl (fact.Q (), get_qr_r (fact), fact.Pvec ());
                    else
                      retval = ovl (fact.Q (), get_qr_r (fact), fact.P ());
                  }
                  break;
                }
            }
          else if (arg.is_complex_type ())
            {
              qr<FloatComplexMatrix>::type type
                = qr_type<FloatComplexMatrix> (nargin, nargout);

              FloatComplexMatrix m = arg.float_complex_matrix_value ();

              switch (nargout)
                {
                case 0:
                case 1:
                  {
                    qr<FloatComplexMatrix> fact (m, type);
                    retval = ovl (fact.R ());
                  }
                  break;

                case 2:
                  {
                    qr<FloatComplexMatrix> fact (m, type);
                    retval = ovl (fact.Q (), get_qr_r (fact));
                  }
                  break;

                default:
                  {
                    qrp<FloatComplexMatrix> fact (m, type);
                    if (type == qr<FloatComplexMatrix>::economy)
                      retval = ovl (fact.Q (), get_qr_r (fact), fact.Pvec ());
                    else
                      retval = ovl (fact.Q (), get_qr_r (fact), fact.P ());
                  }
                  break;
                }
            }
        }
      else
        {
          if (arg.is_real_type ())
            {
              qr<Matrix>::type type = qr_type<Matrix> (nargin, nargout);

              Matrix m = arg.matrix_value ();

              switch (nargout)
                {
                case 0:
                case 1:
                  {
                    qr<Matrix> fact (m, type);
                    retval = ovl (fact.R ());
                  }
                  break;

                case 2:
                  {
                    qr<Matrix> fact (m, type);
                    retval = ovl (fact.Q (), get_qr_r (fact));
                  }
                  break;

                default:
                  {
                    qrp<Matrix> fact (m, type);
                    if (type == qr<Matrix>::economy)
                      retval = ovl (fact.Q (), get_qr_r (fact), fact.Pvec ());
                    else
                      retval = ovl (fact.Q (), get_qr_r (fact), fact.P ());
                  }
                  break;
                }
            }
          else if (arg.is_complex_type ())
            {
              qr<ComplexMatrix>::type type
                = qr_type<ComplexMatrix> (nargin, nargout);

              ComplexMatrix m = arg.complex_matrix_value ();

              switch (nargout)
                {
                case 0:
                case 1:
                  {
                    qr<ComplexMatrix> fact (m, type);
                    retval = ovl (fact.R ());
                  }
                  break;

                case 2:
                  {
                    qr<ComplexMatrix> fact (m, type);
                    retval = ovl (fact.Q (), get_qr_r (fact));
                  }
                  break;

                default:
                  {
                    qrp<ComplexMatrix> fact (m, type);
                    if (type == qr<ComplexMatrix>::economy)
                      retval = ovl (fact.Q (), get_qr_r (fact), fact.Pvec ());
                    else
                      retval = ovl (fact.Q (), get_qr_r (fact), fact.P ());
                  }
                  break;
                }
            }
          else
            err_wrong_type_arg ("qr", arg);
        }
    }

  return retval;
}

/*
%!test
%! a = [0, 2, 1; 2, 1, 2];
%!
%! [q, r] = qr (a);
%! [qe, re] = qr (a, 0);
%!
%! assert (q * r, a, sqrt (eps));
%! assert (qe * re, a, sqrt (eps));

%!test
%! a = [0, 2, 1; 2, 1, 2];
%!
%! [q, r, p] = qr (a);  # FIXME: not giving right dimensions.
%! [qe, re, pe] = qr (a, 0);
%!
%! assert (q * r, a * p, sqrt (eps));
%! assert (qe * re, a(:, pe), sqrt (eps));

%!test
%! a = [0, 2; 2, 1; 1, 2];
%!
%! [q, r] = qr (a);
%! [qe, re] = qr (a, 0);
%!
%! assert (q * r, a, sqrt (eps));
%! assert (qe * re, a, sqrt (eps));

%!test
%! a = [0, 2; 2, 1; 1, 2];
%!
%! [q, r, p] = qr (a);
%! [qe, re, pe] = qr (a, 0);
%!
%! assert (q * r, a * p, sqrt (eps));
%! assert (qe * re, a(:, pe), sqrt (eps));

%!error qr ()
%!error qr ([1, 2; 3, 4], 0, 2)

%!function retval = __testqr (q, r, a, p)
%!  tol = 100*eps (class (q));
%!  retval = 0;
%!  if (nargin == 3)
%!    n1 = norm (q*r - a);
%!    n2 = norm (q'*q - eye (columns (q)));
%!    retval = (n1 < tol && n2 < tol);
%!  else
%!    n1 = norm (q'*q - eye (columns (q)));
%!    retval = (n1 < tol);
%!    if (isvector (p))
%!      n2 = norm (q*r - a(:,p));
%!      retval = (retval && n2 < tol);
%!    else
%!      n2 = norm (q*r - a*p);
%!      retval = (retval && n2 < tol);
%!    endif
%!  endif
%!endfunction

%!test
%! t = ones (24, 1);
%! j = 1;
%!
%! if (false)  # eliminate big matrix tests
%!   a = rand (5000, 20);
%!   [q,r]   = qr (a, 0);  t(j++) = __testqr (q, r, a);
%!   [q,r]   = qr (a',0);  t(j++) = __testqr (q, r, a');
%!   [q,r,p] = qr (a, 0);  t(j++) = __testqr (q, r, a, p);
%!   [q,r,p] = qr (a',0);  t(j++) = __testqr (q, r, a', p);
%!
%!   a = a+1i*eps;
%!   [q,r]   = qr (a, 0);  t(j++) = __testqr (q, r, a);
%!   [q,r]   = qr (a',0);  t(j++) = __testqr (q, r, a');
%!   [q,r,p] = qr (a, 0);  t(j++) = __testqr (q, r, a, p);
%!   [q,r,p] = qr (a',0);  t(j++) = __testqr (q, r, a', p);
%! endif
%!
%! a = [ ones(1,15); sqrt(eps)*eye(15) ];
%! [q,r]   = qr (a);   t(j++) = __testqr (q, r, a);
%! [q,r]   = qr (a');  t(j++) = __testqr (q, r, a');
%! [q,r,p] = qr (a);   t(j++) = __testqr (q, r, a, p);
%! [q,r,p] = qr (a');  t(j++) = __testqr (q, r, a', p);
%!
%! a = a+1i*eps;
%! [q,r]   = qr (a);   t(j++) = __testqr (q, r, a);
%! [q,r]   = qr (a');  t(j++) = __testqr (q, r, a');
%! [q,r,p] = qr (a);   t(j++) = __testqr (q, r, a, p);
%! [q,r,p] = qr (a');  t(j++) = __testqr (q, r, a', p);
%!
%! a = [ ones(1,15); sqrt(eps)*eye(15) ];
%! [q,r]   = qr (a, 0);  t(j++) = __testqr (q, r, a);
%! [q,r]   = qr (a',0);  t(j++) = __testqr (q, r, a');
%! [q,r,p] = qr (a, 0);  t(j++) = __testqr (q, r, a, p);
%! [q,r,p] = qr (a',0);  t(j++) = __testqr (q, r, a', p);
%!
%! a = a+1i*eps;
%! [q,r]   = qr (a, 0);  t(j++) = __testqr (q, r, a);
%! [q,r]   = qr (a',0);  t(j++) = __testqr (q, r, a');
%! [q,r,p] = qr (a, 0);  t(j++) = __testqr (q, r, a, p);
%! [q,r,p] = qr (a',0);  t(j++) = __testqr (q, r, a', p);
%!
%! a = [ 611   196  -192   407    -8   -52   -49    29
%!       196   899   113  -192   -71   -43    -8   -44
%!      -192   113   899   196    61    49     8    52
%!       407  -192   196   611     8    44    59   -23
%!        -8   -71    61     8   411  -599   208   208
%!       -52   -43    49    44  -599   411   208   208
%!       -49    -8     8    59   208   208    99  -911
%!        29   -44    52   -23   208   208  -911    99 ];
%! [q,r] = qr (a);
%!
%! assert (all (t) && norm (q*r - a) < 5000*eps);

%!test
%! a = single ([0, 2, 1; 2, 1, 2]);
%!
%! [q, r] = qr (a);
%! [qe, re] = qr (a, 0);
%!
%! assert (q * r, a, sqrt (eps ("single")));
%! assert (qe * re, a, sqrt (eps ("single")));

%!test
%! a = single ([0, 2, 1; 2, 1, 2]);
%!
%! [q, r, p] = qr (a);  # FIXME: not giving right dimensions.
%! [qe, re, pe] = qr (a, 0);
%!
%! assert (q * r, a * p, sqrt (eps ("single")));
%! assert (qe * re, a(:, pe), sqrt (eps ("single")));

%!test
%! a = single ([0, 2; 2, 1; 1, 2]);
%!
%! [q, r] = qr (a);
%! [qe, re] = qr (a, 0);
%!
%! assert (q * r, a, sqrt (eps ("single")));
%! assert (qe * re, a, sqrt (eps ("single")));

%!test
%! a = single ([0, 2; 2, 1; 1, 2]);
%!
%! [q, r, p] = qr (a);
%! [qe, re, pe] = qr (a, 0);
%!
%! assert (q * r, a * p, sqrt (eps ("single")));
%! assert (qe * re, a(:, pe), sqrt (eps ("single")));

%!error qr ()
%!error qr ([1, 2; 3, 4], 0, 2)

%!test
%! t = ones (24, 1);
%! j = 1;
%!
%! if (false)  # eliminate big matrix tests
%!   a = rand (5000,20);
%!   [q,r]   = qr (a, 0);  t(j++) = __testqr (q, r, a);
%!   [q,r]   = qr (a',0);  t(j++) = __testqr (q, r, a');
%!   [q,r,p] = qr (a, 0);  t(j++) = __testqr (q, r, a, p);
%!   [q,r,p] = qr (a',0);  t(j++) = __testqr (q, r, a', p);
%!
%!   a = a+1i*eps ("single");
%!   [q,r]   = qr (a, 0);  t(j++) = __testqr (q, r, a);
%!   [q,r]   = qr (a',0);  t(j++) = __testqr (q, r, a');
%!   [q,r,p] = qr (a, 0);  t(j++) = __testqr (q, r, a, p);
%!   [q,r,p] = qr (a',0);  t(j++) = __testqr (q, r, a', p);
%! endif
%!
%! a = [ ones(1,15); sqrt(eps("single"))*eye(15) ];
%! [q,r]   = qr (a);   t(j++) = __testqr (q, r, a);
%! [q,r]   = qr (a');  t(j++) = __testqr (q, r, a');
%! [q,r,p] = qr (a);   t(j++) = __testqr (q, r, a, p);
%! [q,r,p] = qr (a');  t(j++) = __testqr (q, r, a', p);
%!
%! a = a+1i*eps ("single");
%! [q,r]   = qr (a);   t(j++) = __testqr (q, r, a);
%! [q,r]   = qr (a');  t(j++) = __testqr (q, r, a');
%! [q,r,p] = qr (a);   t(j++) = __testqr (q, r, a, p);
%! [q,r,p] = qr (a');  t(j++) = __testqr (q, r, a', p);
%!
%! a = [ ones(1,15); sqrt(eps("single"))*eye(15) ];
%! [q,r]   = qr (a, 0);  t(j++) = __testqr (q, r, a);
%! [q,r]   = qr (a',0);  t(j++) = __testqr (q, r, a');
%! [q,r,p] = qr (a, 0);  t(j++) = __testqr (q, r, a, p);
%! [q,r,p] = qr (a',0);  t(j++) = __testqr (q, r, a', p);
%!
%! a = a+1i*eps ("single");
%! [q,r]   = qr (a, 0);  t(j++) = __testqr (q, r, a);
%! [q,r]   = qr (a',0);  t(j++) = __testqr (q, r, a');
%! [q,r,p] = qr (a, 0);  t(j++) = __testqr (q, r, a, p);
%! [q,r,p] = qr (a',0);  t(j++) = __testqr (q, r, a',p);
%!
%! a = [ 611   196  -192   407    -8   -52   -49    29
%!       196   899   113  -192   -71   -43    -8   -44
%!      -192   113   899   196    61    49     8    52
%!       407  -192   196   611     8    44    59   -23
%!        -8   -71    61     8   411  -599   208   208
%!       -52   -43    49    44  -599   411   208   208
%!       -49    -8     8    59   208   208    99  -911
%!        29   -44    52   -23   208   208  -911    99 ];
%! [q,r] = qr (a);
%!
%! assert (all (t) && norm (q*r-a) < 5000*eps ("single"));

## The deactivated tests below can't be tested till rectangular back-subs is
## implemented for sparse matrices.

%!testif HAVE_CXSPARSE
%! n = 20;  d = 0.2;
%! a = sprandn (n,n,d) + speye (n,n);
%! r = qr (a);
%! assert (r'*r, a'*a, 1e-10);

%!testif HAVE_COLAMD
%! n = 20;  d = 0.2;
%! a = sprandn (n,n,d) + speye (n,n);
%! q = symamd (a);
%! a = a(q,q);
%! r = qr (a);
%! assert (r'*r, a'*a, 1e-10);

%!testif HAVE_CXSPARSE
%! n = 20;  d = 0.2;
%! a = sprandn (n,n,d) + speye (n,n);
%! [c,r] = qr (a, ones (n,1));
%! assert (r\c, full (a)\ones (n,1), 10e-10);

%!testif HAVE_CXSPARSE
%! n = 20;  d = 0.2;
%! a = sprandn (n,n,d) + speye (n,n);
%! b = randn (n,2);
%! [c,r] = qr (a, b);
%! assert (r\c, full (a)\b, 10e-10);

%% Test under-determined systems!!
%!#testif HAVE_CXSPARSE
%! n = 20;  d = 0.2;
%! a = sprandn (n,n+1,d) + speye (n,n+1);
%! b = randn (n,2);
%! [c,r] = qr (a, b);
%! assert (r\c, full (a)\b, 10e-10);

%!testif HAVE_CXSPARSE
%! n = 20;  d = 0.2;
%! a = 1i*sprandn (n,n,d) + speye (n,n);
%! r = qr (a);
%! assert (r'*r,a'*a,1e-10);

%!testif HAVE_COLAMD
%! n = 20;  d = 0.2;
%! a = 1i*sprandn (n,n,d) + speye (n,n);
%! q = symamd (a);
%! a = a(q,q);
%! r = qr (a);
%! assert (r'*r, a'*a, 1e-10);

%!testif HAVE_CXSPARSE
%! n = 20;  d = 0.2;
%! a = 1i*sprandn (n,n,d) + speye (n,n);
%! [c,r] = qr (a, ones (n,1));
%! assert (r\c, full (a)\ones (n,1), 10e-10);

%!testif HAVE_CXSPARSE
%! n = 20;  d = 0.2;
%! a = 1i*sprandn (n,n,d) + speye (n,n);
%! b = randn (n,2);
%! [c,r] = qr (a, b);
%! assert (r\c, full (a)\b, 10e-10);

%% Test under-determined systems!!
%!#testif HAVE_CXSPARSE
%! n = 20;  d = 0.2;
%! a = 1i*sprandn (n,n+1,d) + speye (n,n+1);
%! b = randn (n,2);
%! [c,r] = qr (a, b);
%! assert (r\c, full (a)\b, 10e-10);

*/

static
bool check_qr_dims (const octave_value& q, const octave_value& r,
                    bool allow_ecf = false)
{
  octave_idx_type m = q.rows ();
  octave_idx_type k = r.rows ();
  octave_idx_type n = r.columns ();
  return ((q.ndims () == 2 && r.ndims () == 2 && k == q.columns ())
          && (m == k || (allow_ecf && k == n && k < m)));
}

static
bool check_index (const octave_value& i, bool vector_allowed = false)
{
  return ((i.is_real_type () || i.is_integer_type ())
          && (i.is_scalar_type () || vector_allowed));
}

DEFUN_DLD (qrupdate, args, ,
           doc: /* -*- texinfo -*-
@deftypefn {} {[@var{Q1}, @var{R1}] =} qrupdate (@var{Q}, @var{R}, @var{u}, @var{v})
Given a QR@tie{}factorization of a real or complex matrix
@w{@var{A} = @var{Q}*@var{R}}, @var{Q}@tie{}unitary and
@var{R}@tie{}upper trapezoidal, return the QR@tie{}factorization of
@w{@var{A} + @var{u}*@var{v}'}, where @var{u} and @var{v} are column vectors
(rank-1 update) or matrices with equal number of columns
(rank-k update).  Notice that the latter case is done as a sequence of
rank-1 updates; thus, for k large enough, it will be both faster and more
accurate to recompute the factorization from scratch.

The QR@tie{}factorization supplied may be either full (Q is square) or
economized (R is square).

@seealso{qr, qrinsert, qrdelete, qrshift}
@end deftypefn */)
{
  octave_value_list retval;

  if (args.length () != 4)
    print_usage ();

  octave_value argq = args(0);
  octave_value argr = args(1);
  octave_value argu = args(2);
  octave_value argv = args(3);

  if (! argq.is_numeric_type () || ! argr.is_numeric_type ()
      || ! argu.is_numeric_type () || ! argv.is_numeric_type ())
    print_usage ();

  if (! check_qr_dims (argq, argr, true))
    error ("qrupdate: Q and R dimensions don't match");

  if (argq.is_real_type () && argr.is_real_type () && argu.is_real_type ()
      && argv.is_real_type ())
    {
      // all real case
      if (argq.is_single_type () || argr.is_single_type ()
          || argu.is_single_type () || argv.is_single_type ())
        {
          FloatMatrix Q = argq.float_matrix_value ();
          FloatMatrix R = argr.float_matrix_value ();
          FloatMatrix u = argu.float_matrix_value ();
          FloatMatrix v = argv.float_matrix_value ();

          qr<FloatMatrix> fact (Q, R);
          fact.update (u, v);

          retval = ovl (fact.Q (), get_qr_r (fact));
        }
      else
        {
          Matrix Q = argq.matrix_value ();
          Matrix R = argr.matrix_value ();
          Matrix u = argu.matrix_value ();
          Matrix v = argv.matrix_value ();

          qr<Matrix> fact (Q, R);
          fact.update (u, v);

          retval = ovl (fact.Q (), get_qr_r (fact));
        }
    }
  else
    {
      // complex case
      if (argq.is_single_type () || argr.is_single_type ()
          || argu.is_single_type () || argv.is_single_type ())
        {
          FloatComplexMatrix Q = argq.float_complex_matrix_value ();
          FloatComplexMatrix R = argr.float_complex_matrix_value ();
          FloatComplexMatrix u = argu.float_complex_matrix_value ();
          FloatComplexMatrix v = argv.float_complex_matrix_value ();

          qr<FloatComplexMatrix> fact (Q, R);
          fact.update (u, v);

          retval = ovl (fact.Q (), get_qr_r (fact));
        }
      else
        {
          ComplexMatrix Q = argq.complex_matrix_value ();
          ComplexMatrix R = argr.complex_matrix_value ();
          ComplexMatrix u = argu.complex_matrix_value ();
          ComplexMatrix v = argv.complex_matrix_value ();

          qr<ComplexMatrix> fact (Q, R);
          fact.update (u, v);

          retval = ovl (fact.Q (), get_qr_r (fact));
        }
    }

  return retval;
}

/*
%!shared A, u, v, Ac, uc, vc
%! A = [0.091364  0.613038  0.999083;
%!      0.594638  0.425302  0.603537;
%!      0.383594  0.291238  0.085574;
%!      0.265712  0.268003  0.238409;
%!      0.669966  0.743851  0.445057 ];
%!
%! u = [0.85082;
%!      0.76426;
%!      0.42883;
%!      0.53010;
%!      0.80683 ];
%!
%! v = [0.98810;
%!      0.24295;
%!      0.43167 ];
%!
%! Ac = [0.620405 + 0.956953i  0.480013 + 0.048806i  0.402627 + 0.338171i;
%!      0.589077 + 0.658457i  0.013205 + 0.279323i  0.229284 + 0.721929i;
%!      0.092758 + 0.345687i  0.928679 + 0.241052i  0.764536 + 0.832406i;
%!      0.912098 + 0.721024i  0.049018 + 0.269452i  0.730029 + 0.796517i;
%!      0.112849 + 0.603871i  0.486352 + 0.142337i  0.355646 + 0.151496i ];
%!
%! uc = [0.20351 + 0.05401i;
%!      0.13141 + 0.43708i;
%!      0.29808 + 0.08789i;
%!      0.69821 + 0.38844i;
%!      0.74871 + 0.25821i ];
%!
%! vc = [0.85839 + 0.29468i;
%!      0.20820 + 0.93090i;
%!      0.86184 + 0.34689i ];
%!

%!test
%! [Q,R] = qr (A);
%! [Q,R] = qrupdate (Q, R, u, v);
%! assert (norm (vec (Q'*Q - eye (5)), Inf) < 1e1*eps);
%! assert (norm (vec (triu (R)-R), Inf) == 0);
%! assert (norm (vec (Q*R - A - u*v'), Inf) < norm (A)*1e1*eps);
%!
%!test
%! [Q,R] = qr (Ac);
%! [Q,R] = qrupdate (Q, R, uc, vc);
%! assert (norm (vec (Q'*Q - eye (5)), Inf) < 1e1*eps);
%! assert (norm (vec (triu (R)-R), Inf) == 0);
%! assert (norm (vec (Q*R - Ac - uc*vc'), Inf) < norm (Ac)*1e1*eps);

%!test
%! [Q,R] = qr (single (A));
%! [Q,R] = qrupdate (Q, R, single (u), single (v));
%! assert (norm (vec (Q'*Q - eye (5,"single")), Inf) < 1e1*eps ("single"));
%! assert (norm (vec (triu (R)-R), Inf) == 0);
%! assert (norm (vec (Q*R - single (A) - single (u)*single (v)'), Inf) < norm (single (A))*1e1*eps ("single"));
%!
%!test
%! [Q,R] = qr (single (Ac));
%! [Q,R] = qrupdate (Q, R, single (uc), single (vc));
%! assert (norm (vec (Q'*Q - eye (5,"single")), Inf) < 1e1*eps ("single"));
%! assert (norm (vec (triu (R)-R), Inf) == 0);
%! assert (norm (vec (Q*R - single (Ac) - single (uc)*single (vc)'), Inf) < norm (single (Ac))*1e1*eps ("single"));
*/

DEFUN_DLD (qrinsert, args, ,
           doc: /* -*- texinfo -*-
@deftypefn {} {[@var{Q1}, @var{R1}] =} qrinsert (@var{Q}, @var{R}, @var{j}, @var{x}, @var{orient})
Given a QR@tie{}factorization of a real or complex matrix
@w{@var{A} = @var{Q}*@var{R}}, @var{Q}@tie{}unitary and
@var{R}@tie{}upper trapezoidal, return the QR@tie{}factorization of
@w{[A(:,1:j-1) x A(:,j:n)]}, where @var{u} is a column vector to be inserted
into @var{A} (if @var{orient} is @qcode{"col"}), or the
QR@tie{}factorization of @w{[A(1:j-1,:);x;A(:,j:n)]}, where @var{x} is a row
vector to be inserted into @var{A} (if @var{orient} is @qcode{"row"}).

The default value of @var{orient} is @qcode{"col"}.  If @var{orient} is
@qcode{"col"}, @var{u} may be a matrix and @var{j} an index vector
resulting in the QR@tie{}factorization of a matrix @var{B} such that
@w{B(:,@var{j})} gives @var{u} and @w{B(:,@var{j}) = []} gives @var{A}.
Notice that the latter case is done as a sequence of k insertions;
thus, for k large enough, it will be both faster and more accurate to
recompute the factorization from scratch.

If @var{orient} is @qcode{"col"}, the QR@tie{}factorization supplied may
be either full (Q is square) or economized (R is square).

If @var{orient} is @qcode{"row"}, full factorization is needed.
@seealso{qr, qrupdate, qrdelete, qrshift}
@end deftypefn */)
{
  int nargin = args.length ();

  if (nargin < 4 || nargin > 5)
    print_usage ();

  octave_value argq = args(0);
  octave_value argr = args(1);
  octave_value argj = args(2);
  octave_value argx = args(3);

  if (! argq.is_numeric_type () || ! argr.is_numeric_type ()
      || ! argx.is_numeric_type ()
      || (nargin > 4 && ! args(4).is_string ()))
    print_usage ();

  std::string orient = (nargin < 5) ? "col" : args(4).string_value ();
  bool col = (orient == "col");

  if (! col && orient != "row")
    error ("qrinsert: ORIENT must be \"col\" or \"row\"");

  if (! check_qr_dims (argq, argr, col) || (! col && argx.rows () != 1))
    error ("qrinsert: dimension mismatch");

  if (! check_index (argj, col))
    error ("qrinsert: invalid index J");

  octave_value_list retval;

  MArray<octave_idx_type> j = argj.octave_idx_type_vector_value ();

  octave_idx_type one = 1;

  if (argq.is_real_type () && argr.is_real_type () && argx.is_real_type ())
    {
      // real case
      if (argq.is_single_type () || argr.is_single_type ()
          || argx.is_single_type ())
        {
          FloatMatrix Q = argq.float_matrix_value ();
          FloatMatrix R = argr.float_matrix_value ();
          FloatMatrix x = argx.float_matrix_value ();

          qr<FloatMatrix> fact (Q, R);

          if (col)
            fact.insert_col (x, j-one);
          else
            fact.insert_row (x.row (0), j(0)-one);

          retval = ovl (fact.Q (), get_qr_r (fact));
        }
      else
        {
          Matrix Q = argq.matrix_value ();
          Matrix R = argr.matrix_value ();
          Matrix x = argx.matrix_value ();

          qr<Matrix> fact (Q, R);

          if (col)
            fact.insert_col (x, j-one);
          else
            fact.insert_row (x.row (0), j(0)-one);

          retval = ovl (fact.Q (), get_qr_r (fact));
        }
    }
  else
    {
      // complex case
      if (argq.is_single_type () || argr.is_single_type ()
          || argx.is_single_type ())
        {
          FloatComplexMatrix Q =
            argq.float_complex_matrix_value ();
          FloatComplexMatrix R =
            argr.float_complex_matrix_value ();
          FloatComplexMatrix x =
            argx.float_complex_matrix_value ();

          qr<FloatComplexMatrix> fact (Q, R);

          if (col)
            fact.insert_col (x, j-one);
          else
            fact.insert_row (x.row (0), j(0)-one);

          retval = ovl (fact.Q (), get_qr_r (fact));
        }
      else
        {
          ComplexMatrix Q = argq.complex_matrix_value ();
          ComplexMatrix R = argr.complex_matrix_value ();
          ComplexMatrix x = argx.complex_matrix_value ();

          qr<ComplexMatrix> fact (Q, R);

          if (col)
            fact.insert_col (x, j-one);
          else
            fact.insert_row (x.row (0), j(0)-one);

          retval = ovl (fact.Q (), get_qr_r (fact));
        }
    }

  return retval;
}

/*
%!test
%! [Q,R] = qr (A);
%! [Q,R] = qrinsert (Q, R, 3, u);
%! assert (norm (vec (Q'*Q - eye (5)), Inf) < 1e1*eps);
%! assert (norm (vec (triu (R) - R), Inf) == 0);
%! assert (norm (vec (Q*R - [A(:,1:2) u A(:,3)]), Inf) < norm (A)*1e1*eps);
%!test
%! [Q,R] = qr (Ac);
%! [Q,R] = qrinsert (Q, R, 3, uc);
%! assert (norm (vec (Q'*Q - eye (5)), Inf) < 1e1*eps);
%! assert (norm (vec (triu (R) - R), Inf) == 0);
%! assert (norm (vec (Q*R - [Ac(:,1:2) uc Ac(:,3)]), Inf) < norm (Ac)*1e1*eps);
%!test
%! x = [0.85082  0.76426  0.42883 ];
%!
%! [Q,R] = qr (A);
%! [Q,R] = qrinsert (Q, R, 3, x, "row");
%! assert (norm (vec (Q'*Q - eye (6)), Inf) < 1e1*eps);
%! assert (norm (vec (triu (R) - R), Inf) == 0);
%! assert (norm (vec (Q*R - [A(1:2,:);x;A(3:5,:)]), Inf) < norm (A)*1e1*eps);
%!test
%! x = [0.20351 + 0.05401i  0.13141 + 0.43708i  0.29808 + 0.08789i ];
%!
%! [Q,R] = qr (Ac);
%! [Q,R] = qrinsert (Q, R, 3, x, "row");
%! assert (norm (vec (Q'*Q - eye (6)), Inf) < 1e1*eps);
%! assert (norm (vec (triu (R) - R), Inf) == 0);
%! assert (norm (vec (Q*R - [Ac(1:2,:);x;Ac(3:5,:)]), Inf) < norm (Ac)*1e1*eps);

%!test
%! [Q,R] = qr (single (A));
%! [Q,R] = qrinsert (Q, R, 3, single (u));
%! assert (norm (vec (Q'*Q - eye (5,"single")), Inf) < 1e1*eps ("single"));
%! assert (norm (vec (triu (R) - R), Inf) == 0);
%! assert (norm (vec (Q*R - single ([A(:,1:2) u A(:,3)])), Inf) < norm (single (A))*1e1*eps ("single"));
%!test
%! [Q,R] = qr (single (Ac));
%! [Q,R] = qrinsert (Q, R, 3, single (uc));
%! assert (norm (vec (Q'*Q - eye (5,"single")), Inf) < 1e1*eps ("single"));
%! assert (norm (vec (triu (R) - R), Inf) == 0);
%! assert (norm (vec (Q*R - single ([Ac(:,1:2) uc Ac(:,3)])), Inf) < norm (single (Ac))*1e1*eps ("single"));
%!test
%! x = single ([0.85082  0.76426  0.42883 ]);
%!
%! [Q,R] = qr (single (A));
%! [Q,R] = qrinsert (Q, R, 3, x, "row");
%! assert (norm (vec (Q'*Q - eye (6,"single")), Inf) < 1e1*eps ("single"));
%! assert (norm (vec (triu (R) - R), Inf) == 0);
%! assert (norm (vec (Q*R - single ([A(1:2,:);x;A(3:5,:)])), Inf) < norm (single (A))*1e1*eps ("single"));
%!test
%! x = single ([0.20351 + 0.05401i  0.13141 + 0.43708i  0.29808 + 0.08789i ]);
%!
%! [Q,R] = qr (single (Ac));
%! [Q,R] = qrinsert (Q, R, 3, x, "row");
%! assert (norm (vec (Q'*Q - eye (6,"single")), Inf) < 1e1*eps ("single"));
%! assert (norm (vec (triu (R) - R), Inf) == 0);
%! assert (norm (vec (Q*R - single ([Ac(1:2,:);x;Ac(3:5,:)])), Inf) < norm (single (Ac))*1e1*eps ("single"));
*/

DEFUN_DLD (qrdelete, args, ,
           doc: /* -*- texinfo -*-
@deftypefn {} {[@var{Q1}, @var{R1}] =} qrdelete (@var{Q}, @var{R}, @var{j}, @var{orient})
Given a QR@tie{}factorization of a real or complex matrix
@w{@var{A} = @var{Q}*@var{R}}, @var{Q}@tie{}unitary and
@var{R}@tie{}upper trapezoidal, return the QR@tie{}factorization of
@w{[A(:,1:j-1) A(:,j+1:n)]}, i.e., @var{A} with one column deleted
(if @var{orient} is @qcode{"col"}), or the QR@tie{}factorization of
@w{[A(1:j-1,:);A(j+1:n,:)]}, i.e., @var{A} with one row deleted (if
@var{orient} is @qcode{"row"}).

The default value of @var{orient} is @qcode{"col"}.

If @var{orient} is @qcode{"col"}, @var{j} may be an index vector
resulting in the QR@tie{}factorization of a matrix @var{B} such that
@w{A(:,@var{j}) = []} gives @var{B}.  Notice that the latter case is done as
a sequence of k deletions; thus, for k large enough, it will be both faster
and more accurate to recompute the factorization from scratch.

If @var{orient} is @qcode{"col"}, the QR@tie{}factorization supplied may
be either full (Q is square) or economized (R is square).

If @var{orient} is @qcode{"row"}, full factorization is needed.
@seealso{qr, qrupdate, qrinsert, qrshift}
@end deftypefn */)
{
  int nargin = args.length ();

  if (nargin < 3 || nargin > 4)
    print_usage ();

  octave_value argq = args(0);
  octave_value argr = args(1);
  octave_value argj = args(2);

  if (! argq.is_numeric_type () || ! argr.is_numeric_type ()
      || (nargin > 3 && ! args(3).is_string ()))
    print_usage ();

  std::string orient = (nargin < 4) ? "col" : args(3).string_value ();
  bool col = orient == "col";

  if (! col && orient != "row")
    error ("qrdelete: ORIENT must be \"col\" or \"row\"");

  if (! check_qr_dims (argq, argr, col))
    error ("qrdelete: dimension mismatch");

  MArray<octave_idx_type> j = argj.octave_idx_type_vector_value ();
  if (! check_index (argj, col))
    error ("qrdelete: invalid index J");

  octave_value_list retval;

  octave_idx_type one = 1;

  if (argq.is_real_type () && argr.is_real_type ())
    {
      // real case
      if (argq.is_single_type () || argr.is_single_type ())
        {
          FloatMatrix Q = argq.float_matrix_value ();
          FloatMatrix R = argr.float_matrix_value ();

          qr<FloatMatrix> fact (Q, R);

          if (col)
            fact.delete_col (j-one);
          else
            fact.delete_row (j(0)-one);

          retval = ovl (fact.Q (), get_qr_r (fact));
        }
      else
        {
          Matrix Q = argq.matrix_value ();
          Matrix R = argr.matrix_value ();

          qr<Matrix> fact (Q, R);

          if (col)
            fact.delete_col (j-one);
          else
            fact.delete_row (j(0)-one);

          retval = ovl (fact.Q (), get_qr_r (fact));
        }
    }
  else
    {
      // complex case
      if (argq.is_single_type () || argr.is_single_type ())
        {
          FloatComplexMatrix Q =
            argq.float_complex_matrix_value ();
          FloatComplexMatrix R =
            argr.float_complex_matrix_value ();

          qr<FloatComplexMatrix> fact (Q, R);

          if (col)
            fact.delete_col (j-one);
          else
            fact.delete_row (j(0)-one);

          retval = ovl (fact.Q (), get_qr_r (fact));
        }
      else
        {
          ComplexMatrix Q = argq.complex_matrix_value ();
          ComplexMatrix R = argr.complex_matrix_value ();

          qr<ComplexMatrix> fact (Q, R);

          if (col)
            fact.delete_col (j-one);
          else
            fact.delete_row (j(0)-one);

          retval = ovl (fact.Q (), get_qr_r (fact));
        }
    }

  return retval;
}

/*
%!test
%! AA = [0.091364  0.613038  0.027504  0.999083;
%!       0.594638  0.425302  0.562834  0.603537;
%!       0.383594  0.291238  0.742073  0.085574;
%!       0.265712  0.268003  0.783553  0.238409;
%!       0.669966  0.743851  0.457255  0.445057 ];
%!
%! [Q,R] = qr (AA);
%! [Q,R] = qrdelete (Q, R, 3);
%! assert (norm (vec (Q'*Q - eye (5)), Inf) < 16*eps);
%! assert (norm (vec (triu (R) - R), Inf) == 0);
%! assert (norm (vec (Q*R - [AA(:,1:2) AA(:,4)]), Inf) < norm (AA)*1e1*eps);
%!
%!test
%! AA = [0.364554 + 0.993117i  0.669818 + 0.510234i  0.426568 + 0.041337i  0.847051 + 0.233291i;
%!       0.049600 + 0.242783i  0.448946 + 0.484022i  0.141155 + 0.074420i  0.446746 + 0.392706i;
%!       0.581922 + 0.657416i  0.581460 + 0.030016i  0.219909 + 0.447288i  0.201144 + 0.069132i;
%!       0.694986 + 0.000571i  0.682327 + 0.841712i  0.807537 + 0.166086i  0.192767 + 0.358098i;
%!       0.945002 + 0.066788i  0.350492 + 0.642638i  0.579629 + 0.048102i  0.600170 + 0.636938i ] * I;
%!
%! [Q,R] = qr (AA);
%! [Q,R] = qrdelete (Q, R, 3);
%! assert (norm (vec (Q'*Q - eye (5)), Inf) < 16*eps);
%! assert (norm (vec (triu (R) - R), Inf) == 0);
%! assert (norm (vec (Q*R - [AA(:,1:2) AA(:,4)]), Inf) < norm (AA)*1e1*eps);
%!
%!test
%! AA = [0.091364  0.613038  0.027504  0.999083;
%!       0.594638  0.425302  0.562834  0.603537;
%!       0.383594  0.291238  0.742073  0.085574;
%!       0.265712  0.268003  0.783553  0.238409;
%!       0.669966  0.743851  0.457255  0.445057 ];
%!
%! [Q,R] = qr (AA);
%! [Q,R] = qrdelete (Q, R, 3, "row");
%! assert (norm (vec (Q'*Q - eye (4)), Inf) < 1e1*eps);
%! assert (norm (vec (triu (R) - R), Inf) == 0);
%! assert (norm (vec (Q*R - [AA(1:2,:);AA(4:5,:)]), Inf) < norm (AA)*1e1*eps);
%!
%!test
%! AA = [0.364554 + 0.993117i  0.669818 + 0.510234i  0.426568 + 0.041337i  0.847051 + 0.233291i;
%!       0.049600 + 0.242783i  0.448946 + 0.484022i  0.141155 + 0.074420i  0.446746 + 0.392706i;
%!       0.581922 + 0.657416i  0.581460 + 0.030016i  0.219909 + 0.447288i  0.201144 + 0.069132i;
%!       0.694986 + 0.000571i  0.682327 + 0.841712i  0.807537 + 0.166086i  0.192767 + 0.358098i;
%!       0.945002 + 0.066788i  0.350492 + 0.642638i  0.579629 + 0.048102i  0.600170 + 0.636938i ] * I;
%!
%! [Q,R] = qr (AA);
%! [Q,R] = qrdelete (Q, R, 3, "row");
%! assert (norm (vec (Q'*Q - eye (4)), Inf) < 1e1*eps);
%! assert (norm (vec (triu (R) - R), Inf) == 0);
%! assert (norm (vec (Q*R - [AA(1:2,:);AA(4:5,:)]), Inf) < norm (AA)*1e1*eps);

%!test
%! AA = single ([0.091364  0.613038  0.027504  0.999083;
%!               0.594638  0.425302  0.562834  0.603537;
%!               0.383594  0.291238  0.742073  0.085574;
%!               0.265712  0.268003  0.783553  0.238409;
%!               0.669966  0.743851  0.457255  0.445057 ]);
%!
%! [Q,R] = qr (AA);
%! [Q,R] = qrdelete (Q, R, 3);
%! assert (norm (vec (Q'*Q - eye (5,"single")), Inf) < 1e1*eps ("single"));
%! assert (norm (vec (triu (R) - R), Inf) == 0);
%! assert (norm (vec (Q*R - [AA(:,1:2) AA(:,4)]), Inf) < norm (AA)*1e1*eps ("single"));
%!
%!test
%! AA = single ([0.364554 + 0.993117i  0.669818 + 0.510234i  0.426568 + 0.041337i  0.847051 + 0.233291i;
%!               0.049600 + 0.242783i  0.448946 + 0.484022i  0.141155 + 0.074420i  0.446746 + 0.392706i;
%!               0.581922 + 0.657416i  0.581460 + 0.030016i  0.219909 + 0.447288i  0.201144 + 0.069132i;
%!               0.694986 + 0.000571i  0.682327 + 0.841712i  0.807537 + 0.166086i  0.192767 + 0.358098i;
%!               0.945002 + 0.066788i  0.350492 + 0.642638i  0.579629 + 0.048102i  0.600170 + 0.636938i ]) * I;
%!
%! [Q,R] = qr (AA);
%! [Q,R] = qrdelete (Q, R, 3);
%! assert (norm (vec (Q'*Q - eye (5,"single")), Inf) < 1e1*eps ("single"));
%! assert (norm (vec (triu (R) - R), Inf) == 0);
%! assert (norm (vec (Q*R - [AA(:,1:2) AA(:,4)]), Inf) < norm (AA)*1e1*eps ("single"));
%!
%!test
%! AA = single ([0.091364  0.613038  0.027504  0.999083;
%!               0.594638  0.425302  0.562834  0.603537;
%!               0.383594  0.291238  0.742073  0.085574;
%!               0.265712  0.268003  0.783553  0.238409;
%!               0.669966  0.743851  0.457255  0.445057 ]);
%!
%! [Q,R] = qr (AA);
%! [Q,R] = qrdelete (Q, R, 3, "row");
%! assert (norm (vec (Q'*Q - eye (4,"single")), Inf) < 1.5e1*eps ("single"));
%! assert (norm (vec (triu (R) - R), Inf) == 0);
%! assert (norm (vec (Q*R - [AA(1:2,:);AA(4:5,:)]), Inf) < norm (AA)*1e1*eps ("single"));
%!testif HAVE_QRUPDATE
%! ## Same test as above but with more precicision
%! AA = single ([0.091364  0.613038  0.027504  0.999083;
%!               0.594638  0.425302  0.562834  0.603537;
%!               0.383594  0.291238  0.742073  0.085574;
%!               0.265712  0.268003  0.783553  0.238409;
%!               0.669966  0.743851  0.457255  0.445057 ]);
%!
%! [Q,R] = qr (AA);
%! [Q,R] = qrdelete (Q, R, 3, "row");
%! assert (norm (vec (Q'*Q - eye (4,"single")), Inf) < 1e1*eps ("single"));
%! assert (norm (vec (triu (R) - R), Inf) == 0);
%! assert (norm (vec (Q*R - [AA(1:2,:);AA(4:5,:)]), Inf) < norm (AA)*1e1*eps ("single"));
%!
%!test
%! AA = single ([0.364554 + 0.993117i  0.669818 + 0.510234i  0.426568 + 0.041337i  0.847051 + 0.233291i;
%!              0.049600 + 0.242783i  0.448946 + 0.484022i  0.141155 + 0.074420i  0.446746 + 0.392706i;
%!              0.581922 + 0.657416i  0.581460 + 0.030016i  0.219909 + 0.447288i  0.201144 + 0.069132i;
%!              0.694986 + 0.000571i  0.682327 + 0.841712i  0.807537 + 0.166086i  0.192767 + 0.358098i;
%!              0.945002 + 0.066788i  0.350492 + 0.642638i  0.579629 + 0.048102i  0.600170 + 0.636938i ]) * I;
%!
%! [Q,R] = qr (AA);
%! [Q,R] = qrdelete (Q, R, 3, "row");
%! assert (norm (vec (Q'*Q - eye (4,"single")), Inf) < 1e1*eps ("single"));
%! assert (norm (vec (triu (R) - R), Inf) == 0);
%! assert (norm (vec (Q*R - [AA(1:2,:);AA(4:5,:)]), Inf) < norm (AA)*1e1*eps ("single"));
*/

DEFUN_DLD (qrshift, args, ,
           doc: /* -*- texinfo -*-
@deftypefn {} {[@var{Q1}, @var{R1}] =} qrshift (@var{Q}, @var{R}, @var{i}, @var{j})
Given a QR@tie{}factorization of a real or complex matrix
@w{@var{A} = @var{Q}*@var{R}}, @var{Q}@tie{}unitary and
@var{R}@tie{}upper trapezoidal, return the QR@tie{}factorization
of @w{@var{A}(:,p)}, where @w{p} is the permutation @*
@code{p = [1:i-1, shift(i:j, 1), j+1:n]} if @w{@var{i} < @var{j}} @*
 or @*
@code{p = [1:j-1, shift(j:i,-1), i+1:n]} if @w{@var{j} < @var{i}}.  @*

@seealso{qr, qrupdate, qrinsert, qrdelete}
@end deftypefn */)
{
  if (args.length () != 4)
    print_usage ();

  octave_value argq = args(0);
  octave_value argr = args(1);
  octave_value argi = args(2);
  octave_value argj = args(3);

  if (! argq.is_numeric_type () || ! argr.is_numeric_type ())
    print_usage ();

  if (! check_qr_dims (argq, argr, true))
    error ("qrshift: dimensions mismatch");

  octave_idx_type i = argi.idx_type_value ();
  octave_idx_type j = argj.idx_type_value ();

  if (! check_index (argi) || ! check_index (argj))
    error ("qrshift: invalid index I or J");

  octave_value_list retval;

  if (argq.is_real_type () && argr.is_real_type ())
    {
      // all real case
      if (argq.is_single_type ()
          && argr.is_single_type ())
        {
          FloatMatrix Q = argq.float_matrix_value ();
          FloatMatrix R = argr.float_matrix_value ();

          qr<FloatMatrix> fact (Q, R);
          fact.shift_cols (i-1, j-1);

          retval = ovl (fact.Q (), get_qr_r (fact));
        }
      else
        {
          Matrix Q = argq.matrix_value ();
          Matrix R = argr.matrix_value ();

          qr<Matrix> fact (Q, R);
          fact.shift_cols (i-1, j-1);

          retval = ovl (fact.Q (), get_qr_r (fact));
        }
    }
  else
    {
      // complex case
      if (argq.is_single_type ()
          && argr.is_single_type ())
        {
          FloatComplexMatrix Q = argq.float_complex_matrix_value ();
          FloatComplexMatrix R = argr.float_complex_matrix_value ();

          qr<FloatComplexMatrix> fact (Q, R);
          fact.shift_cols (i-1, j-1);

          retval = ovl (fact.Q (), get_qr_r (fact));
        }
      else
        {
          ComplexMatrix Q = argq.complex_matrix_value ();
          ComplexMatrix R = argr.complex_matrix_value ();

          qr<ComplexMatrix> fact (Q, R);
          fact.shift_cols (i-1, j-1);

          retval = ovl (fact.Q (), get_qr_r (fact));
        }
    }

  return retval;
}

/*
%!test
%! AA = A.';
%! i = 2;  j = 4;  p = [1:i-1, shift(i:j,-1), j+1:5];
%!
%! [Q,R] = qr (AA);
%! [Q,R] = qrshift (Q, R, i, j);
%! assert (norm (vec (Q'*Q - eye (3)), Inf) < 1e1*eps);
%! assert (norm (vec (triu (R) - R), Inf) == 0);
%! assert (norm (vec (Q*R - AA(:,p)), Inf) < norm (AA)*1e1*eps);
%!
%! j = 2;  i = 4;  p = [1:j-1, shift(j:i,+1), i+1:5];
%!
%! [Q,R] = qr (AA);
%! [Q,R] = qrshift (Q, R, i, j);
%! assert (norm (vec (Q'*Q - eye (3)), Inf) < 1e1*eps);
%! assert (norm (vec (triu (R) - R), Inf) == 0);
%! assert (norm (vec (Q*R - AA(:,p)), Inf) < norm (AA)*1e1*eps);
%!
%!test
%! AA = Ac.';
%! i = 2;  j = 4;  p = [1:i-1, shift(i:j,-1), j+1:5];
%!
%! [Q,R] = qr (AA);
%! [Q,R] = qrshift (Q, R, i, j);
%! assert (norm (vec (Q'*Q - eye (3)), Inf) < 1e1*eps);
%! assert (norm (vec (triu (R) - R), Inf) == 0);
%! assert (norm (vec (Q*R - AA(:,p)), Inf) < norm (AA)*1e1*eps);
%!
%! j = 2;  i = 4;  p = [1:j-1, shift(j:i,+1), i+1:5];
%!
%! [Q,R] = qr (AA);
%! [Q,R] = qrshift (Q, R, i, j);
%! assert (norm (vec (Q'*Q - eye (3)), Inf) < 1e1*eps);
%! assert (norm (vec (triu (R) - R), Inf) == 0);
%! assert (norm (vec (Q*R - AA(:,p)), Inf) < norm (AA)*1e1*eps);

%!test
%! AA = single (A).';
%! i = 2;  j = 4;  p = [1:i-1, shift(i:j,-1), j+1:5];
%!
%! [Q,R] = qr (AA);
%! [Q,R] = qrshift (Q, R, i, j);
%! assert (norm (vec (Q'*Q - eye (3,"single")), Inf) < 1e1*eps ("single"));
%! assert (norm (vec (triu (R) - R), Inf) == 0);
%! assert (norm (vec (Q*R - AA(:,p)), Inf) < norm (AA)*1e1*eps ("single"));
%!
%! j = 2;  i = 4;  p = [1:j-1, shift(j:i,+1), i+1:5];
%!
%! [Q,R] = qr (AA);
%! [Q,R] = qrshift (Q, R, i, j);
%! assert (norm (vec (Q'*Q - eye (3,"single")), Inf) < 1e1*eps ("single"));
%! assert (norm (vec (triu (R) - R), Inf) == 0);
%! assert (norm (vec (Q*R - AA(:,p)), Inf) < norm (AA)*1e1*eps ("single"));
%!
%!test
%! AA = single (Ac).';
%! i = 2;  j = 4;  p = [1:i-1, shift(i:j,-1), j+1:5];
%!
%! [Q,R] = qr (AA);
%! [Q,R] = qrshift (Q, R, i, j);
%! assert (norm (vec (Q'*Q - eye (3,"single")), Inf) < 1e1*eps ("single"));
%! assert (norm (vec (triu (R) - R), Inf) == 0);
%! assert (norm (vec (Q*R - AA(:,p)), Inf) < norm (AA)*1e1*eps ("single"));
%!
%! j = 2;  i = 4;  p = [1:j-1, shift(j:i,+1), i+1:5];
%!
%! [Q,R] = qr (AA);
%! [Q,R] = qrshift (Q, R, i, j);
%! assert (norm (vec (Q'*Q - eye (3,"single")), Inf) < 1e1*eps ("single"));
%! assert (norm (vec (triu (R) - R), Inf) == 0);
%! assert (norm (vec (Q*R - AA(:,p)), Inf) < norm (AA)*1e1*eps ("single"));
*/
