      SUBROUTINE SOLSY (WM, IWM, X, TEM)
CLLL. OPTIMIZE
      INTEGER IWM
      INTEGER IOWND, IOWNS,
     1   ICF, IERPJ, IERSL, JCUR, JSTART, KFLAG, L, METH, MITER,
     2   MAXORD, MAXCOR, MSBP, MXNCF, N, NQ, NST, NFE, NJE, NQU
      INTEGER I, MEBAND, ML, MU
      DOUBLE PRECISION WM, X, TEM
      DOUBLE PRECISION ROWNS,
     1   CCMAX, EL0, H, HMIN, HMXI, HU, RC, TN, UROUND
      DOUBLE PRECISION DI, HL0, PHL0, R
      DIMENSION WM(*), IWM(*), X(*), TEM(*)
      COMMON /LS0001/ ROWNS(209),
     2   CCMAX, EL0, H, HMIN, HMXI, HU, RC, TN, UROUND,
     3   IOWND(14), IOWNS(6),
     4   ICF, IERPJ, IERSL, JCUR, JSTART, KFLAG, L, METH, MITER,
     5   MAXORD, MAXCOR, MSBP, MXNCF, N, NQ, NST, NFE, NJE, NQU
C-----------------------------------------------------------------------
C THIS ROUTINE MANAGES THE SOLUTION OF THE LINEAR SYSTEM ARISING FROM
C A CHORD ITERATION.  IT IS CALLED IF MITER .NE. 0.
C IF MITER IS 1 OR 2, IT CALLS DGETRS TO ACCOMPLISH THIS.
C IF MITER = 3 IT UPDATES THE COEFFICIENT H*EL0 IN THE DIAGONAL
C MATRIX, AND THEN COMPUTES THE SOLUTION.
C IF MITER IS 4 OR 5, IT CALLS DGBTRS.
C COMMUNICATION WITH SOLSY USES THE FOLLOWING VARIABLES..
C WM    = REAL WORK SPACE CONTAINING THE INVERSE DIAGONAL MATRIX IF
C         MITER = 3 AND THE LU DECOMPOSITION OF THE MATRIX OTHERWISE.
C         STORAGE OF MATRIX ELEMENTS STARTS AT WM(3).
C         WM ALSO CONTAINS THE FOLLOWING MATRIX-RELATED DATA..
C         WM(1) = SQRT(UROUND) (NOT USED HERE),
C         WM(2) = HL0, THE PREVIOUS VALUE OF H*EL0, USED IF MITER = 3.
C IWM   = INTEGER WORK SPACE CONTAINING PIVOT INFORMATION, STARTING AT
C         IWM(21), IF MITER IS 1, 2, 4, OR 5.  IWM ALSO CONTAINS BAND
C         PARAMETERS ML = IWM(1) AND MU = IWM(2) IF MITER IS 4 OR 5.
C X     = THE RIGHT-HAND SIDE VECTOR ON INPUT, AND THE SOLUTION VECTOR
C         ON OUTPUT, OF LENGTH N.
C TEM   = VECTOR OF WORK SPACE OF LENGTH N, NOT USED IN THIS VERSION.
C IERSL = OUTPUT FLAG (IN COMMON).  IERSL = 0 IF NO TROUBLE OCCURRED.
C         IERSL = 1 IF A SINGULAR MATRIX AROSE WITH MITER = 3.
C THIS ROUTINE ALSO USES THE COMMON VARIABLES EL0, H, MITER, AND N.
C-----------------------------------------------------------------------
      IERSL = 0
      GO TO (100, 100, 300, 400, 400), MITER
 100  CALL DGETRS ( 'N', N, 1, WM(3), N, IWM(21), X, N, INLPCK)
      RETURN
C
 300  PHL0 = WM(2)
      HL0 = H*EL0
      WM(2) = HL0
      IF (HL0 .EQ. PHL0) GO TO 330
      R = HL0/PHL0
      DO 320 I = 1,N
        DI = 1.0D0 - R*(1.0D0 - 1.0D0/WM(I+2))
        IF (DABS(DI) .EQ. 0.0D0) GO TO 390
 320    WM(I+2) = 1.0D0/DI
 330  DO 340 I = 1,N
 340    X(I) = WM(I+2)*X(I)
      RETURN
 390  IERSL = 1
      RETURN
C
 400  ML = IWM(1)
      MU = IWM(2)
      MEBAND = 2*ML + MU + 1
      CALL DGBTRS ( 'N', N, ML, MU, 1, WM(3), MEBAND, IWM(21), X, N,
     * INLPCK)
      RETURN
C----------------------- END OF SUBROUTINE SOLSY -----------------------
      END
