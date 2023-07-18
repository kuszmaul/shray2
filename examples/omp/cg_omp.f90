!-------------------------------------------------------------------------!
!                                                                         !
!        N  A  S     P A R A L L E L     B E N C H M A R K S  3.4         !
!                                                                         !
!                       O p e n M P     V E R S I O N                     !
!                                                                         !
!                                   C G                                   !
!                                                                         !
!-------------------------------------------------------------------------!
!                                                                         !
!    This benchmark is an OpenMP version of the NPB CG code.              !
!    It is described in NAS Technical Report 99-011.                      !
!                                                                         !
!    Permission to use, copy, distribute and modify this software         !
!    for any purpose with or without fee is hereby granted.  We           !
!    request, however, that all derived work reference the NAS            !
!    Parallel Benchmarks 3.4. This software is provided "as is"           !
!    without express or implied warranty.                                 !
!                                                                         !
!    Information on NPB 3.4, including the technical report, the          !
!    original specifications, source code, results and information        !
!    on how to submit new results, is available at:                       !
!                                                                         !
!           http://www.nas.nasa.gov/Software/NPB/                         !
!                                                                         !
!    Send comments or suggestions to  npb@nas.nasa.gov                    !
!                                                                         !
!          NAS Parallel Benchmarks Group                                  !
!          NASA Ames Research Center                                      !
!          Mail Stop: T27A-1                                              !
!          Moffett Field, CA   94035-1000                                 !
!                                                                         !
!          E-mail:  npb@nas.nasa.gov                                      !
!          Fax:     (650) 604-3957                                        !
!                                                                         !
!-------------------------------------------------------------------------!


!---------------------------------------------------------------------
!
! Authors: M. Yarrow
!          C. Kuszmaul
!          H. Jin
!
!---------------------------------------------------------------------


!---------------------------------------------------------------------
!---------------------------------------------------------------------

module conj_mod_s
contains
      subroutine conj_grad ( rnorm, a, colidx, rowstr, x, p, q, r, z, naa)
!---------------------------------------------------------------------
!---------------------------------------------------------------------

!---------------------------------------------------------------------
!  Floaging point arrays here are named as in NPB1 spec discussion of
!  CG algorithm
!---------------------------------------------------------------------

      implicit none

      integer   cgit, cgitmax
      integer(8) j, k, naa

      double precision   d, sum, rho, rho0, alpha, beta, rnorm, suml
      double precision ::  x(:), p(:), q(:), r(:), z(:), a(:)
      integer(8) ::  colidx(:), rowstr(:)

      data      cgitmax / 25 /


      rho = 0.0d0
      sum = 0.0d0

!$omp parallel default(shared) private(j,k,cgit,suml,alpha,beta)  &
!$omp&  shared(d,rho0,rho,sum)

!---------------------------------------------------------------------
!  Initialize the CG algorithm:
!---------------------------------------------------------------------
!$omp do
      do j=1,naa
         q(j) = 0.0d0
         z(j) = 0.0d0
         r(j) = x(j)
         p(j) = r(j)
      enddo
!$omp end do


!---------------------------------------------------------------------
!  rho = r.r
!  Now, obtain the norm of r: First, sum squares of r elements locally...
!---------------------------------------------------------------------
!$omp do reduction(+:rho)
      do j=1, naa
         rho = rho + r(j)*r(j)
      enddo
!$omp end do

    !$omp master
    !write(*, *) "rho = ", rho
    !$omp end master
!---------------------------------------------------------------------
!---->
!  The conj grad iteration loop
!---->
!---------------------------------------------------------------------
      do cgit = 1, cgitmax

!$omp master
!---------------------------------------------------------------------
!  Save a temporary of rho and initialize reduction variables
!---------------------------------------------------------------------
         rho0 = rho
         d = 0.d0
         rho = 0.d0
!$omp end master
!$omp barrier

!---------------------------------------------------------------------
!  q = A.p
!  The partition submatrix-vector multiply: use workspace w
!---------------------------------------------------------------------
!
!  NOTE: this version of the multiply is actually (slightly: maybe %5)
!        faster on the sp2 on 16 nodes than is the unrolled-by-2 version
!        below.   On the Cray t3d, the reverse is true, i.e., the
!        unrolled-by-two version is some 10% faster.
!        The unrolled-by-8 version below is significantly faster
!        on the Cray t3d - overall speed of code is 1.5 times faster.
!
!$omp do
!        do j=701,701
         do j=1,naa
            suml = 0.d0
!            write(*, *) "k from ", rowstr(j), " to ", rowstr(j + 1) - 1
            do k=rowstr(j),rowstr(j+1)-1
!               write(*, *) "suml += ", a(k), " * ", p(colidx(k))
               suml = suml + a(k)*p(colidx(k))
            enddo
            q(j) = suml
         enddo
!$omp end do

        !$omp master
        !write(*, *) "q(1) = ", q(1)
        !write(*, *) "q(701) = ", q(701)
        !write(*, *) "p(1) = ", p(1)
        !write(*, *) "p(701) = ", p(701)
        !$omp end master
!---------------------------------------------------------------------
!  Obtain p.q
!---------------------------------------------------------------------
!$omp do reduction(+:d)
         do j=1, naa
            d = d + p(j)*q(j)
         enddo
!$omp end do

    !$omp master
    !write(*, *) "d is ", d
    !$omp end master

!---------------------------------------------------------------------
!  Obtain alpha = rho / (p.q)
!---------------------------------------------------------------------
         alpha = rho0 / d

!---------------------------------------------------------------------
!  Obtain z = z + alpha*p
!  and    r = r - alpha*q
!---------------------------------------------------------------------
!$omp do reduction(+:rho)
         do j=1, naa
            z(j) = z(j) + alpha*p(j)
            r(j) = r(j) - alpha*q(j)
!         enddo

!---------------------------------------------------------------------
!  rho = r.r
!  Now, obtain the norm of r: First, sum squares of r elements locally...
!---------------------------------------------------------------------
!         do j=1, lastcol-firstcol+1
            rho = rho + r(j)*r(j)
         enddo
!$omp end do

    !$omp master
    !write(*, *) "rho after = ", rho
    !$omp end master
!---------------------------------------------------------------------
!  Obtain beta:
!---------------------------------------------------------------------
         beta = rho / rho0

!---------------------------------------------------------------------
!  p = r + beta*p
!---------------------------------------------------------------------
!$omp do
         do j=1, naa
            p(j) = r(j) + beta*p(j)
         enddo
!$omp end do


      enddo                             ! end of do cgit=1,cgitmax


!---------------------------------------------------------------------
!  Compute residual norm explicitly:  ||r|| = ||x - A.z||
!  First, form A.z
!  The partition submatrix-vector multiply
!---------------------------------------------------------------------
!$omp do
      do j=1,naa
         suml = 0.d0
         do k=rowstr(j),rowstr(j+1)-1
            suml = suml + a(k)*z(colidx(k))
         enddo
         r(j) = suml
      enddo
!$omp end do


!---------------------------------------------------------------------
!  At this point, r contains A.z
!---------------------------------------------------------------------
!$omp do reduction(+:sum)
      do j=1, naa
         suml = x(j) - r(j)
         sum  = sum + suml*suml
      enddo
!$omp end do nowait
!$omp end parallel

    !$omp master
    !write(*, *) "sum is ", sum
    !$omp end master

      rnorm = sqrt( sum )



      return
      end                               ! end of routine conj_grad
end module conj_mod_s


!---------------------------------------------------------------------
!---------------------------------------------------------------------
      program cg
!---------------------------------------------------------------------
!---------------------------------------------------------------------

      use, intrinsic :: ieee_arithmetic, only : ieee_is_nan
      use conj_mod_s

      implicit none

      double precision, allocatable ::  &
     &                         a(:),  &
     &                         x(:),  &
     &                         z(:),  &
     &                         p(:),  &
     &                         q(:),  &
     &                         r(:)

! ... partition size
      integer(8)               naa, nzz

      double precision         amult, tran
!$omp threadprivate (amult, tran)

      double precision   zeta, randlc
      double precision   rnorm
      double precision   norm_temp1,norm_temp2,norm_temp3

      double precision   t, gflops
      character          class
      logical            verified
      double precision   zeta_verify_value, epsilon, err

!$    integer   omp_get_max_threads
!$    external  omp_get_max_threads


      ! Parameters
      integer(8) :: na, nz, nonzer, niter
      integer(8), allocatable :: colidx(:), rowstr(:)
      double precision :: shift

      ! IO
      integer :: io_a, io_col, io_row, recl_a, recl_col, recl_row

      ! Index variables
      integer(8) :: i, j, it, k

      ! Commandline arguments
      character(len=12) :: arg

      ! Timing
      integer :: cpu_count, cpu_count2, count_rate, count_max

      call get_command_argument(1, arg)
      read(unit=arg, fmt=*) class

      if (class .eq. 'S') then
          na = 1400
          nonzer = 7
          niter = 15
          shift = 10.d0
          zeta_verify_value = 8.5971775078648d0
      else if (class .eq. 'W') then
          na = 7000
          nonzer = 7
          niter = 15
          shift = 12.d0
          zeta_verify_value = 10.362595087124d0
      else if (class .eq. 'A') then
          na = 14000
          nonzer = 11
          niter = 15
          shift = 20.d0
          zeta_verify_value = 17.130235054029d0
      else if (class .eq. 'B') then
          na = 75000
          nonzer = 13
          niter = 75
          shift = 60.d0
          zeta_verify_value = 22.712745482631d0
      else if (class .eq. 'C') then
          na = 150000
          nonzer = 15
          niter = 75
          shift = 110.d0
          zeta_verify_value = 28.973605592845d0
      else if (class .eq. 'D') then
          na = 1500000
          nonzer = 21
          niter = 100
          shift = 500.d0
          zeta_verify_value = 52.514532105794d0
      else if (class .eq. 'E') then
          na = 9000000
          nonzer = 26
          niter = 100
          shift = 1.5d3
          zeta_verify_value = 77.522164599383d0
      else if (class .eq. 'F') then
          na = 54000000
          nonzer = 31
          niter = 100
          shift = 5.0d3
          zeta_verify_value = 107.3070826433d0
      else
          class = 'U'
          write(*, *) "Class undefined"
          na = 1
          nonzer = 1
          niter = 1
          shift = 1.d0
          zeta_verify_value = 1.d0
      end if

    nz = na * (nonzer + 1) * (nonzer + 1) + na * (nonzer + 2)

      allocate (  &
     &          colidx(nz), rowstr(na+1),  &
     &          a(nz),  &
     &          x(na),  &
     &          z(na),  &
     &          p(na),  &
     &          q(na),  &
     &          r(na))


      inquire(iolength=recl_a) a(1:nz)
      inquire(iolength=recl_col) colidx(1:nz)
      inquire(iolength=recl_row) rowstr(1:na + 1)

      open(newunit=io_a, file="a.cg." // class, form='unformatted', &
          access='direct', recl=recl_a)
      open(newunit=io_col, file="colidx.cg." // class, form='unformatted', &
          access='direct', recl=recl_col)
      open(newunit=io_row, file="rowstr.cg." // class, form='unformatted', &
          access='direct', recl=recl_row)

      read(io_a, rec=1) a

      read(io_col, rec=1) colidx

      do i = 1, nz
          ! C ordering -> Fortran ordering
          colidx(i) = colidx(i) + 1
      end do

      read(io_row, rec=1) rowstr

      do i = 1, na + 1
          ! C ordering -> Fortran ordering
          rowstr(i) = rowstr(i) + 1
      end do

      close(io_a)
      close(io_col)
      close(io_row)

      write( *,1000 )
      write( *,1001 ) na
      write( *,1002 ) niter
!$    write( *,1003 ) omp_get_max_threads()
      write( *,* )
 1000 format(//,' NAS Parallel Benchmarks (NPB3.4-OMP)',  &
     &          ' - CG Benchmark', /)
 1001 format(' Size: ', i11 )
 1002 format(' Iterations:                  ', i5 )
 1003 format(' Number of available threads: ', i5)

      naa = na
      nzz = nz

!---------------------------------------------------------------------
!  Inialize random number generator
!---------------------------------------------------------------------
!$omp parallel default(shared) private(i,j,k,zeta)
      tran    = 314159265.0D0
      amult   = 1220703125.0D0
      zeta    = randlc( tran, amult )

!---------------------------------------------------------------------
!  set starting vector to (1, 1, .... 1)
!---------------------------------------------------------------------
!$omp do
      do i = 1, na
         x(i) = 1.0D0
      enddo
!$omp end do nowait
!$omp do
      do j=1, na
         q(j) = 0.0d0
         z(j) = 0.0d0
         r(j) = 0.0d0
         p(j) = 0.0d0
      enddo
!$omp end do nowait
!$omp end parallel

      zeta  = 0.0d0

!---------------------------------------------------------------------
!---->
!  Do one iteration untimed to init all code and data page tables
!---->                    (then reinit, start timing, to niter its)
!---------------------------------------------------------------------
      do it = 1, 1

!---------------------------------------------------------------------
!  The call to the conjugate gradient routine:
!---------------------------------------------------------------------
         call conj_grad ( rnorm, a, colidx, rowstr, x, p, q, r, z, na)

!---------------------------------------------------------------------
!  zeta = shift + 1/(x.z)
!  So, first: (x.z)
!  Also, find norm of z
!  So, first: (z.z)
!---------------------------------------------------------------------
         norm_temp1 = 0.0d0
         norm_temp2 = 0.0d0
!$omp parallel default(shared) private(j,norm_temp3)
!$omp do reduction(+:norm_temp1,norm_temp2)
         do j=1, na
            norm_temp1 = norm_temp1 + x(j)*z(j)
            norm_temp2 = norm_temp2 + z(j)*z(j)
         enddo
!$omp end do

         norm_temp3 = 1.0d0 / sqrt( norm_temp2 )

         !$omp master
!        write(*, *) "norm_temp3 is ", norm_temp3
        !$omp end master
!---------------------------------------------------------------------
!  Normalize z to obtain x
!---------------------------------------------------------------------
!$omp do
         do j=1, na
            x(j) = norm_temp3*z(j)
         enddo
!$omp end do nowait
!$omp end parallel


      enddo                              ! end of do one iteration untimed


!---------------------------------------------------------------------
!  set starting vector to (1, 1, .... 1)
!---------------------------------------------------------------------
!
!
!
!$omp parallel do default(shared) private(i)
      do i = 1, na
         x(i) = 1.0D0
      enddo
!$omp end parallel do

      zeta  = 0.0d0


!---------------------------------------------------------------------
!---->
!  Main Iteration for inverse power method
!---->
!---------------------------------------------------------------------

    call system_clock(cpu_count, count_rate, count_max)

      do it = 1, niter

!---------------------------------------------------------------------
!  The call to the conjugate gradient routine:
!---------------------------------------------------------------------

         call conj_grad ( rnorm, a, colidx, rowstr, x, p, q, r, z, na)

!---------------------------------------------------------------------
!  zeta = shift + 1/(x.z)
!  So, first: (x.z)
!  Also, find norm of z
!  So, first: (z.z)
!---------------------------------------------------------------------
         norm_temp1 = 0.0d0
         norm_temp2 = 0.0d0
!$omp parallel default(shared) private(j,norm_temp3)
!$omp do reduction(+:norm_temp1,norm_temp2)
         do j=1, na
            norm_temp1 = norm_temp1 + x(j)*z(j)
            norm_temp2 = norm_temp2 + z(j)*z(j)
         enddo
!$omp end do


         norm_temp3 = 1.0d0 / sqrt( norm_temp2 )


!$omp master
         zeta = shift + 1.0d0 / norm_temp1
         if( it .eq. 1 ) write( *,9000 )
         write( *,9001 ) it, rnorm, zeta
!$omp end master

 9000    format( /,'   iteration           ||r||                 zeta' )
 9001    format( 4x, i5, 6x, e21.14, f20.13 )

!---------------------------------------------------------------------
!  Normalize z to obtain x
!---------------------------------------------------------------------
!$omp do
         do j=1, na
            x(j) = norm_temp3*z(j)
         enddo
!$omp end do nowait
!$omp end parallel


      enddo                              ! end of main iter inv pow meth

    call system_clock(cpu_count2, count_rate, count_max)
!---------------------------------------------------------------------
!  End of timed section
!---------------------------------------------------------------------
    t = real((cpu_count2 - cpu_count) / count_rate)

      write(*,100)
 100  format(' Benchmark completed ')

      epsilon = 1.d-10
      if (class .ne. 'U') then

!         err = abs( zeta - zeta_verify_value)
         err = abs( zeta - zeta_verify_value )/zeta_verify_value
         if( (.not.ieee_is_nan(err)) .and. (err .le. epsilon) ) then
            verified = .TRUE.
            write(*, 200)
            write(*, 201) zeta
            write(*, 202) err
 200        format(' VERIFICATION SUCCESSFUL ')
 201        format(' Zeta is    ', E20.13)
 202        format(' Error is   ', E20.13)
         else
            verified = .FALSE.
            write(*, 300)
            write(*, 301) zeta
            write(*, 302) zeta_verify_value
 300        format(' VERIFICATION FAILED')
 301        format(' Zeta                ', E20.13)
 302        format(' The correct zeta is ', E20.13)
         endif
      else
         verified = .FALSE.
         write (*, 400)
         write (*, 401)
         write (*, 201) zeta
 400     format(' Problem size unknown')
 401     format(' NO VERIFICATION PERFORMED')
      endif


       gflops = 1.0d-9 * 2*niter*dble( na )  &
     &               * ( 3.+nonzer*dble(nonzer+1)  &
     &                 + 25.*(5.+nonzer*dble(nonzer+1))  &
     &                 + 3. ) / t

    write(*, *) gflops, " Gflops/s"

      end                              ! end main


!---------------------------------------------------------------------
!---------------------------------------------------------------------

      double precision function randlc (x, a)

!---------------------------------------------------------------------
!---------------------------------------------------------------------

!---------------------------------------------------------------------
!
!   This routine returns a uniform pseudorandom double precision number in the
!   range (0, 1) by using the linear congruential generator
!
!   x_{k+1} = a x_k  (mod 2^46)
!
!   where 0 < x_k < 2^46 and 0 < a < 2^46.  This scheme generates 2^44 numbers
!   before repeating.  The argument A is the same as 'a' in the above formula,
!   and X is the same as x_0.  A and X must be odd double precision integers
!   in the range (1, 2^46).  The returned value RANDLC is normalized to be
!   between 0 and 1, i.e. RANDLC = 2^(-46) * x_1.  X is updated to contain
!   the new seed x_1, so that subsequent calls to RANDLC using the same
!   arguments will generate a continuous sequence.
!
!   This routine should produce the same results on any computer with at least
!   48 mantissa bits in double precision floating point data.  On 64 bit
!   systems, double precision should be disabled.
!
!   David H. Bailey     October 26, 1990
!
!---------------------------------------------------------------------

      implicit none

      double precision r23,r46,t23,t46,a,x,t1,t2,t3,t4,a1,a2,x1,x2,z
      parameter (r23 = 0.5d0 ** 23, r46 = r23 ** 2, t23 = 2.d0 ** 23,  &
     &  t46 = t23 ** 2)

!---------------------------------------------------------------------
!   Break A into two parts such that A = 2^23 * A1 + A2.
!---------------------------------------------------------------------
      t1 = r23 * a
      a1 = int (t1)
      a2 = a - t23 * a1

!---------------------------------------------------------------------
!   Break X into two parts such that X = 2^23 * X1 + X2, compute
!   Z = A1 * X2 + A2 * X1  (mod 2^23), and then
!   X = 2^23 * Z + A2 * X2  (mod 2^46).
!---------------------------------------------------------------------
      t1 = r23 * x
      x1 = int (t1)
      x2 = x - t23 * x1
      t1 = a1 * x2 + a2 * x1
      t2 = int (r23 * t1)
      z = t1 - t23 * t2
      t3 = t23 * z + a2 * x2
      t4 = int (r46 * t3)
      x = t3 - t46 * t4
      randlc = r46 * x

      return
      end
