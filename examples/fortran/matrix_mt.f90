! A matrix multiply of two n x n matrices A and B. We store them distributed
! blockwise, so locally each processor has a n / p x n array.

module parallel_matmul
contains
    function matmulp(A, B) result(C)
        implicit none
        real(KIND=8), intent(in) :: A(:,:), B(:,:)
        real(KIND=8) :: C(size(A, 1), size(B, 2))
        integer, external :: omp_get_num_threads
        integer :: n, i, threads

        n = size(B, 2)

        !$omp parallel
        threads = omp_get_num_threads()
        !$omp do
        do i = 1, threads
           C(:, (i - 1) * n / threads + 1: i * n / threads) = &
                matmul(A, B(:, (i - 1) * n / threads + 1: i * n / threads))
        end do
        !$omp end do
        !$omp end parallel

    end function matmulp
end module parallel_matmul

program main
    use parallel_matmul
    implicit none

    integer :: n, p, l
    real(KIND=8), dimension(:,:), codimension[:], allocatable :: A, B, C
    character(len=12), dimension(:), allocatable :: args
    integer :: cpu_count, cpu_count2, count_rate, count_max
    real(KIND=8), allocatable :: Bl(:,:), Al(:,:)

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
    allocate(Al(n / p, n / p))
    allocate(C(n / p, n)[*])

    A = 1
    B = 1
    C = 0

    sync all
    call system_clock(cpu_count, count_rate, count_max)

    do l = 1, num_images()
        Bl(:,:) = B(:, :)[l]
        Al(:,:) = A(:,1 + (l - 1) * n / p: l * n / p)
        C = C + matmulp(Al, Bl)
    end do

    sync all
    call system_clock(cpu_count2, count_rate, count_max)

    if (this_image() .eq. 1) then
        write (*, *) 2.0 * n * n * n / (real(cpu_count2 - cpu_count) / count_rate) / 1000000000.0
    end if
end program main
