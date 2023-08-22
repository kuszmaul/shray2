! A matrix multiply of two n x n matrices A and B. We store them distributed blockwise, so
! locally each processor has a n x n / p array.

program main
    external :: dgemm

    integer :: n, p, l
    real(KIND=8), dimension(:,:), codimension[:], allocatable :: A, B, C
    real(KIND=8), dimension(:,:), allocatable :: Bl
    character(len=12), dimension(:), allocatable :: args
    integer :: cpu_count, cpu_count2, count_rate, count_max

    allocate(args(1))
    call get_command_argument(1, args(1))
    read (unit=args(1),fmt=*) n

    p = num_images()

    if (modulo(n, p) /= 0) then
        write (*, *) 'Please make sure n divides p'
    end if

    allocate(A(n / p, n)[*])
    allocate(B(n / p, n)[*])
    allocate(Bl(n / p, n))
    allocate(C(n / p, n)[*])

    A = 1
    B = 1
    C = 0

    sync all
    call system_clock(cpu_count, count_rate, count_max)

    do l = 1, num_images()
        Bl(:,:) = B(:,:)[l]
        call dgemm('N', 'N', n / p, n, n / p, 1.0, A(1, (l - 1) * n / p + 1), &
               n / p, Bl, n / p, 1.0, C, n / p)
    end do

    sync all
    call system_clock(cpu_count2, count_rate, count_max)

    if (this_image() .eq. 1) then
        write (*, *) 2.0 * n * n * n / (real(cpu_count2 - cpu_count) / count_rate) / 1000000000.0
    end if

end program main
