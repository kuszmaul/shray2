! A matrix multiply of two n x n matrices A and B. We store them distributed blockwise, so
! locally each processor has a n x n / p array.
! The algorithm we use is based on the fact that A (B[1] ... B[p]) = (AB[1] ... AB[p])
! where p is the number of processors and B[s] is the part stored by processor s.
! We cannot calculate A B[s] right away because A is not stored in its entirety on processor s.
! So we calculate it as the inner product
!
!                 (         B(1: n / p)[s]       )
! (A[1] ... A[p]) (             ...              )
!                 ( B(1 + (p - 1) * n / p: n)[s] )


program main
    implicit none

    integer :: n, p, s
    real(KIND=8), dimension(:,:), codimension[:], allocatable :: A, B, C
    character(len=12), dimension(:), allocatable :: args
    integer :: cpu_count, cpu_count2, count_rate, count_max

    allocate(args(1))
    call get_command_argument(1, args(1))
    read (unit=args(1),fmt=*) n

    p = num_images()

    if (modulo(n, p) /= 0) then
        write (*, *) 'Please make sure n divides p'
    end if

    allocate(A(n, n / p)[*])
    allocate(B(n, n / p)[*])
    allocate(C(n, n / p)[*])

    A = 1
    B = 1
    C = 0

    sync all
    call system_clock(cpu_count, count_rate, count_max)

    do s = 1, p
        C = C + matmul(A(:,:)[s], B(1 + (s - 1) * n / p: s * n / p, :))
    end do

    sync all
    call system_clock(cpu_count2, count_rate, count_max)

    if (this_image() .eq. 1) then
        write (*, *) 2.0 * n * n * n / (real(cpu_count2 - cpu_count) / count_rate) / 1000000000.0
    end if

end program main
