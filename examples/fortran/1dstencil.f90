! A naive 3 point stencil.



program main
    use stencilModule
    implicit none

    integer :: n, iterations, b
    integer :: cpu_count, cpu_count2, count_rate, count_max
    real(KIND=8), dimension(:,:), codimension[:], allocatable :: input, output
    character(len=12), dimension(:), allocatable :: args

    allocate(args(2))
    call get_command_argument(1, args(1))
    read (unit=args(1), fmt=*) n

    call get_command_argument(2, args(2))
    read (unit=args(2), fmt=*) iterations

    if (modulo(iterations, b) /= 0) then
        write (*, *) 'Please make sure n divides ', b
    end if

    allocate(input(n))
    allocate(output(n))

    input = 1
    output = 1

    sync all
    call system_clock(cpu_count, count_rate, count_max)

    call stencil(n, input, output, iterations)
    sync all
    call system_clock(cpu_count2, count_rate, count_max)

    if (this_image() .eq. 1) then
        write (*, *) 9.0 * (n - 2) * (n - 2) * iterations / 1000000000.0 &
            / (real(cpu_count2 - cpu_count) / count_rate)
    end if
end program main


