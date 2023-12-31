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

module conj_mod
    use, intrinsic :: ieee_arithmetic, only : ieee_is_nan

contains
    function loc_ind(j, blocksize) result(i)
        integer(8), intent(in) :: j, blocksize
        integer(8) :: i

        i = modulo(j - 1, blocksize) + 1
    end function loc_ind

    function loc_proc(j, blocksize) result(s)
        integer(8), intent(in) :: j, blocksize
        integer(8) :: s

        s = (j - 1) / blocksize + 1
    end function loc_proc

    ! j is global index
    function glob_r(a, j) result(res)
        integer(8), intent(in) :: j
        double precision, intent(in) :: a(:)[*]
        double precision :: res
        integer(8) :: blocksize

        blocksize = size(a)

        res = a(loc_ind(j, blocksize))[loc_proc(j, blocksize)]
    end function glob_r

    ! j is global index
    function glob_i(a, j) result(res)
        integer(8), intent(in) :: j
        integer(8), intent(in) :: a(:)[*]
        integer(8) :: res
        integer(8) :: blocksize

        blocksize = size(a)

        res = a(loc_ind(j, blocksize))[loc_proc(j, blocksize)]
    end function glob_i

    ! j is local index
    function loc_r(a, j) result(res)
        integer(8), intent(in) :: j
        double precision, intent(in) :: a(:)[*]
        double precision :: res
        integer(8) :: blocksize, k

        blocksize = size(a)
        k = (this_image() - 1) * blocksize + j

        res = a(loc_ind(k, blocksize))[loc_proc(k, blocksize)]
    end function loc_r

    ! j is local index
    function loc_i(a, j) result(res)
        integer(8), intent(in) :: j
        integer(8), intent(in) :: a(:)[*]
        integer(8) :: res
        integer(8) :: blocksize, k

        blocksize = size(a)
        k = (this_image() - 1) * blocksize + j

        res = a(loc_ind(k, blocksize))[loc_proc(k, blocksize)]
    end function loc_i

    subroutine conj_grad ( rnorm, a, colidx, rowstr, x, p, q, r, z, naa, nzz)
!---------------------------------------------------------------------
!---------------------------------------------------------------------

!---------------------------------------------------------------------
!  Floaging point arrays here are named as in NPB1 spec discussion of
!  CG algorithm
!---------------------------------------------------------------------

      implicit none

      integer   cgit, cgitmax
      integer(8) j, k, naa, nz_block, na_block, nzz, na_local

      double precision   d, sumr, rho, rho0, alpha, beta, rnorm, suml, d_acc, sum_acc, rho_acc
      double precision, allocatable :: all_rhos[:], all_ds[:], all_sums[:]
      double precision ::  x(:)[*], p(:)[*], q(:)[*], r(:)[*], z(:)[*], a(:)[*]
      integer(8) ::  colidx(:)[*], rowstr(:)[*]

      data      cgitmax / 25 /

      allocate(all_rhos[*], all_ds[*], all_sums[*])

      nz_block = (nzz + num_images() - 1) / num_images()

      na_block = (naa + num_images() - 1) / num_images()
      if (na_block * this_image() .gt. naa) then
          na_local = naa - na_block * (this_image() - 1)
      else
          na_local = na_block
      end if

      rho = 0.0d0
      sumr = 0.0d0

!---------------------------------------------------------------------
!  Initialize the CG algorithm:
!---------------------------------------------------------------------
      do j=1,na_local
         q(j) = 0.0d0
         z(j) = 0.0d0
         r(j) = x(j)
         p(j) = r(j)
      enddo

    sync all;

!---------------------------------------------------------------------
!  rho = r.r
!  Now, obtain the norm of r: First, sum squares of r elements locally...
!---------------------------------------------------------------------
      do j=1, na_local
         rho = rho + r(j)*r(j)
      enddo

    all_rhos = rho
    sync all

    rho_acc = 0
    do j = 1, num_images()
        rho_acc = rho_acc + all_rhos[j]
    end do
    rho = rho_acc

    !write(*, *) "rho = ", rho, " at image ", this_image()

!---------------------------------------------------------------------
!---->
!  The conj grad iteration loop
!---->
!---------------------------------------------------------------------
      do cgit = 1, cgitmax

!---------------------------------------------------------------------
!  Save a temporary of rho and initialize reduction variables
!---------------------------------------------------------------------
         rho0 = rho
         d = 0.d0
         rho = 0.d0

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

!   if (this_image() .eq. 2) then
!        do j=1,1
         do j=1,na_local
            suml = 0.d0
            ! j is local index of q, not of rowstr, so we cannot use loc_i!
            do k=glob_i(rowstr, (this_image() - 1) * size(q) + j), &
                glob_i(rowstr, (this_image() - 1) * size(q) + j + 1) - 1
!               write(*, *) "suml += ", glob_r(a, k), " * ", glob_r(p, glob_i(colidx, k))
               suml = suml + glob_r(a, k) * glob_r(p, glob_i(colidx, k))
            enddo
            q(j) = suml
         enddo
!end if

    !write(*, *) "q(", (this_image() - 1) * na_block + 1, ") = ", q(1)

    sync all

!---------------------------------------------------------------------
!  Obtain p.q
!---------------------------------------------------------------------
         do j=1, na_local
            d = d + p(j)*q(j)
         enddo

        ! WARNING: does not work with mpich
        all_ds = d
        sync all
        d_acc = 0
        do j = 1, num_images()
            d_acc = d_acc + all_ds[j]
        end do
        d = d_acc
!        write(*, *) "d is ", d, " at image ", this_image()


!---------------------------------------------------------------------
!  Obtain alpha = rho / (p.q)
!---------------------------------------------------------------------
         alpha = rho0 / d

!---------------------------------------------------------------------
!  Obtain z = z + alpha*p
!  and    r = r - alpha*q
!---------------------------------------------------------------------
         do j=1, na_local
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

    all_rhos = rho
    sync all
    rho_acc = 0
    do j = 1, num_images()
        rho_acc = rho_acc + all_rhos[j]
    end do
    rho = rho_acc

!    write(*, *) "rho after = ", rho, " at image ", this_image()


!---------------------------------------------------------------------
!  Obtain beta:
!---------------------------------------------------------------------
         beta = rho / rho0

!---------------------------------------------------------------------
!  p = r + beta*p
!---------------------------------------------------------------------
    do j=1, na_local
        p(j) = r(j) + beta*p(j)
    enddo

    sync all

      enddo                             ! end of do cgit=1,cgitmax

!---------------------------------------------------------------------
!  Compute residual norm explicitly:  ||r|| = ||x - A.z||
!  First, form A.z
!  The partition submatrix-vector multiply
!---------------------------------------------------------------------
      do j=1,na_local
         suml = 0.d0
         !write(*, *) "k from ", loc_i(rowstr, j), " to ", loc_i(rowstr, j + 1) - 1
         do k=loc_i(rowstr,j),loc_i(rowstr,j+1)-1
            suml = suml + glob_r(a, k) * glob_r(z, glob_i(colidx, k))
         enddo
         r(j) = suml
      enddo

    sync all;


!---------------------------------------------------------------------
!  At this point, r contains A.z
!---------------------------------------------------------------------
      do j=1, na_local
         suml = x(j) - r(j)
         sumr  = sumr + suml*suml
      enddo

    all_sums = sumr
    sync all
    sum_acc = 0
    do j = 1, num_images()
        sum_acc = sum_acc + all_sums[j]
    end do
    sumr = sum_acc
    !write(*, *) "sum is ", sum, " at image ", this_image()

      rnorm = sqrt( sumr )



      return
      end                               ! end of routine conj_grad
end module conj_mod


!---------------------------------------------------------------------
!---------------------------------------------------------------------
      program cg
!---------------------------------------------------------------------
!---------------------------------------------------------------------

      use, intrinsic :: ieee_arithmetic, only : ieee_is_nan
      use conj_mod

      implicit none

      double precision, allocatable ::  &
     &                         a(:)[:],  &
     &                         x(:)[:],  &
     &                         z(:)[:],  &
     &                         p(:)[:],  &
     &                         q(:)[:],  &
     &                         r(:)[:]

! ... partition size
      integer(8)               naa, nzz

      double precision         amult, tran

      double precision   zeta, randlc
      double precision   rnorm
      double precision   norm_temp1,norm_temp2,norm_temp3, norm_temp1_acc, norm_temp2_acc
      double precision, allocatable :: all_norm_temp1[:], all_norm_temp2[:]

      double precision   t, mflops
      character          class
      logical            verified
      double precision   zeta_verify_value, epsilon, err

      ! Parameters
      integer(8) :: na, nz, nonzer, niter, na_local, nz_local, &
          na_block, nz_block, row_block, row_local, recl_a, recl_col, recl_row
      integer(8), allocatable :: colidx(:)[:], rowstr(:)[:]
      double precision :: shift

      ! IO
      integer :: io_a, io_col, io_row

      ! Index variables
      integer(8) :: i, j, it

      ! Commandline arguments
      character(len=12) :: arg
      character(len=100) :: matdir

      ! Timing
      integer :: cpu_count, cpu_count2, count_rate, count_max

      call get_command_argument(1, arg)
      call get_command_argument(2, matdir)
      read(unit=arg, fmt=*) class

      if (class .eq. 'S') then
          na = 1400
          nonzer = 7
          niter = 15
          shift = 10.d0
          zeta_verify_value = 8.5971775078648d0
      else if (class .eq. 'W') then
          na = 7000
          nonzer = 8
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

    nz_block = (nz + num_images() - 1) / num_images()
    if (nz_block * this_image() .gt. nz) then
        nz_local = nz - nz_block * (this_image() - 1) + 1
    else
        nz_local = nz_block
    end if

    na_block = (na + num_images() - 1) / num_images()
    if (na_block * this_image() .gt. na) then
        na_local = na - na_block * (this_image() - 1) + 1
    else
        na_local = na_block
    end if

    row_block = (na + 1 + num_images() - 1) / num_images()
    if (row_block * this_image() .gt. na + 1) then
        row_local = na + 1 - row_block * (this_image() - 1) + 1
    else
        row_local = row_block
    end if

      allocate (  &
     &          colidx(nz_block)[*], rowstr(row_block)[*],  &
     &          a(nz_block)[*],  &
     &          x(na_block)[*],  &
     &          z(na_block)[*],  &
     &          p(na_block)[*],  &
     &          q(na_block)[*],  &
     &          r(na_block)[*])

     allocate(all_norm_temp1[*], all_norm_temp2[*])

      inquire(iolength=recl_a) a(1:nz_block)
      inquire(iolength=recl_col) colidx(1:nz_block)
      inquire(iolength=recl_row) rowstr(1:row_block)

      open(newunit=io_a, file=trim(matdir) // "/a.cg." // class, form='unformatted', &
          access='direct', recl=recl_a)
      open(newunit=io_col, file=trim(matdir) // "/colidx.cg." // class, form='unformatted', &
          access='direct', recl=recl_col)
      open(newunit=io_row, file=trim(matdir) // "/rowstr.cg." // class, form='unformatted', &
          access='direct', recl=recl_row)

      read(io_a, rec=this_image()) a

      read(io_col, rec=this_image()) colidx

      do i = 1, nz_local
          ! C ordering -> Fortran ordering
          colidx(i) = colidx(i) + 1
      end do

      read(io_row, rec=this_image()) rowstr

      do i = 1, row_local
          ! C ordering -> Fortran ordering
          rowstr(i) = rowstr(i) + 1
      end do

      close(io_a)
      close(io_col)
      close(io_row)

      sync all

      if (this_image() .eq. 1) then
          write( *,1000 )
          write( *,1001 ) na
          write( *,1002 ) niter
          write( *,* )
     1000 format(//,' NAS Parallel Benchmarks (NPB3.4-OMP)',  &
         &          ' - CG Benchmark', /)
     1001 format(' Size: ', i11 )
     1002 format(' Iterations:                  ', i5 )
     end if

      naa = na
      nzz = nz

!---------------------------------------------------------------------
!  Inialize random number generator
!---------------------------------------------------------------------
      tran    = 314159265.0D0
      amult   = 1220703125.0D0
      zeta    = randlc( tran, amult )

!---------------------------------------------------------------------
!  set starting vector to (1, 1, .... 1)
!---------------------------------------------------------------------
      do i = 1, na_local
         x(i) = 1.0D0
      enddo
      do j=1, na_local
         q(j) = 0.0d0
         z(j) = 0.0d0
         r(j) = 0.0d0
         p(j) = 0.0d0
      enddo

    sync all

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
         call conj_grad ( rnorm, a, colidx, rowstr, x, p, q, r, z, na, nzz)

!---------------------------------------------------------------------
!  zeta = shift + 1/(x.z)
!  So, first: (x.z)
!  Also, find norm of z
!  So, first: (z.z)
!---------------------------------------------------------------------
         norm_temp1 = 0.0d0
         norm_temp2 = 0.0d0
         do j=1, na_local
            norm_temp1 = norm_temp1 + x(j)*z(j)
            norm_temp2 = norm_temp2 + z(j)*z(j)
         enddo

        all_norm_temp1 = norm_temp1
        all_norm_temp2 = norm_temp2
        sync all

        norm_temp1_acc = 0.0d0
        norm_temp2_acc = 0.0d0

        do j = 1, num_images()
            norm_temp1_acc = norm_temp1_acc + all_norm_temp1[j]
            norm_temp2_acc = norm_temp2_acc + all_norm_temp2[j]
        end do

        norm_temp1 = norm_temp1_acc
        norm_temp2 = norm_temp2_acc

         norm_temp3 = 1.0d0 / sqrt( norm_temp2 )

         !write(*, *) "norm_temp3 is ", norm_temp3, " at ", this_image()


!---------------------------------------------------------------------
!  Normalize z to obtain x
!---------------------------------------------------------------------
         do j=1, na_local
            x(j) = norm_temp3*z(j)
         enddo

    sync all

      enddo                              ! end of do one iteration untimed


!---------------------------------------------------------------------
!  set starting vector to (1, 1, .... 1)
!---------------------------------------------------------------------
!
!
!
      do i = 1, na_local
         x(i) = 1.0D0
      enddo

    sync all

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

         call conj_grad ( rnorm, a, colidx, rowstr, x, p, q, r, z, na, nzz)

!---------------------------------------------------------------------
!  zeta = shift + 1/(x.z)
!  So, first: (x.z)
!  Also, find norm of z
!  So, first: (z.z)
!---------------------------------------------------------------------
         norm_temp1 = 0.0d0
         norm_temp2 = 0.0d0
         do j=1, na_local
            norm_temp1 = norm_temp1 + x(j)*z(j)
            norm_temp2 = norm_temp2 + z(j)*z(j)
         enddo

        all_norm_temp1[this_image()] = norm_temp1
        all_norm_temp2[this_image()] = norm_temp2
        sync all

        norm_temp1_acc = 0.0d0
        norm_temp2_acc = 0.0d0

        do j = 1, num_images()
            norm_temp1_acc = norm_temp1_acc + all_norm_temp1[j]
            norm_temp2_acc = norm_temp2_acc + all_norm_temp2[j]
        end do

        norm_temp1 = norm_temp1_acc
        norm_temp2 = norm_temp2_acc


         norm_temp3 = 1.0d0 / sqrt( norm_temp2 )


         zeta = shift + 1.0d0 / norm_temp1
         if (this_image() .eq. 1) then
            if( it .eq. 1 ) write( *,9000 )
                write( *,9001 ) it, rnorm, zeta
        end if

 9000    format( /,'   iteration           ||r||                 zeta' )
 9001    format( 4x, i5, 6x, e21.14, f20.13 )

!---------------------------------------------------------------------
!  Normalize z to obtain x
!---------------------------------------------------------------------
         do j=1, na_local
            x(j) = norm_temp3*z(j)
         enddo


      enddo                              ! end of main iter inv pow meth

    call system_clock(cpu_count2, count_rate, count_max)
!---------------------------------------------------------------------
!  End of timed section
!---------------------------------------------------------------------
    t = real((cpu_count2 - cpu_count) / count_rate)

    if (this_image() .eq. 1) then
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


       mflops = 1.0d-6 * 2*niter*dble( na )  &
     &               * ( 3.+nonzer*dble(nonzer+1)  &
     &                 + 25.*(5.+nonzer*dble(nonzer+1))  &
     &                 + 3. ) / t

    write(*, *) mflops, " Mflops/s"
    end if
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
