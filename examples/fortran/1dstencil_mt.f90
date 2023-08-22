#define SPACEBLOCK 10000
#define TIMEBLOCK 100

module kernels_mt
contains
subroutine left(n, iterations, input, output)
    implicit none

    integer, intent(in) :: n, iterations
    real(KIND=8) :: input(n), output(n)

    integer :: t, i
    real(KIND=8) :: temp(n), constants(3)

    constants = [0.50, 0.33, 0.25]

    output(1) = input(1)

    do t = 1, iterations
        do i = 2, n + iterations - 1 - t
            output(i) = sum(input(i - 1: i + 1) * constants)
        end do
        if (t .ne. iterations) then
            temp = input
            input = output
            output = temp
        end if
    end do
end subroutine left

subroutine middle(n, iterations, input, output)
    implicit none

    integer, intent(in) :: n, iterations
    real(KIND=8) :: input(n), output(n)

    integer :: t, i
    real(KIND=8) :: temp(n), constants(3)

    constants = [0.50, 0.33, 0.25]

    output(1) = input(1)

    do t = 1, iterations
        do i = 2 + t, n + 2 * iterations - 1 - t
            output(i) = sum(input(i - 1: i + 1) * constants)
        end do
        if (t .ne. iterations) then
            temp = input
            input = output
            output = temp
        end if
    end do
end subroutine middle

subroutine right(n, iterations, input, output)
    implicit none

    integer, intent(in) :: n, iterations
    real(KIND=8) :: input(n), output(n)

    integer :: t, i
    real(KIND=8) :: temp(n), constants(3)

    constants = [0.50, 0.33, 0.25]

    output(n + iterations) = input(n + iterations)

    do t = 1, iterations
        do i = 2 + t, n + iterations - 1
            output(i) = sum(input(i - 1: i + 1) * constants)
        end do
        if (t .ne. iterations) then
            temp = input
            input = output
            output = temp
        end if
    end do
end subroutine right
end module kernels_mt

module stencilModule_mt
contains
subroutine BlockedStencil(n, input, output, iterations)
    use kernels_mt
    implicit none
    integer, intent(in) :: n, iterations
    real(KIND=8), intent(inout) :: input(n)[*], output(n)[*]

    real(KIND=8), allocatable :: inBuffer(:), outBuffer(:)
    integer :: row, rowsPerImage, ending

    allocate(inBuffer(SPACEBLOCK + 2 * iterations))
    allocate(outBuffer(SPACEBLOCK + 2 * iterations))

    rowsPerImage = (n / SPACEBLOCK + num_images() - 1) / num_images()
    ending = min(rowsPerImage, n / SPACEBLOCK - (num_images() - 1) * rowsPerImage)

    !$OMP PARALLEL DO
    do row = 1, ending
        if ((row .eq. 1) .and. (this_image() .eq. 1)) then
            inBuffer(1:SPACEBLOCK + iterations) = input(1:SPACEBLOCK + iterations)
            call left(SPACEBLOCK, iterations, inBuffer(:), outBuffer(:))
            output(1:SPACEBLOCK) = outBuffer(1:SPACEBLOCK)
        else if ((this_image() .eq. num_images()) .and. (row .eq. ending)) then
            inBuffer(1:SPACEBLOCK + iterations) = \
                input(1 + (row - 1) * SPACEBLOCK - iterations:row * SPACEBLOCK)
            call right(SPACEBLOCK, iterations, inBuffer(:), outBuffer(:))
            output((row - 1) * SPACEBLOCK + 1:row * SPACEBLOCK) = outBuffer(1:SPACEBLOCK)
        else
            if (row .eq. 1) then
                inBuffer(1:iterations) = &
                    input(rowsPerImage * SPACEBLOCK - iterations + 1: rowsPerImage * SPACEBLOCK)[this_image() - 1]
                inBuffer(iterations + 1:SPACEBLOCK + 2 * iterations) = &
                    input(1:SPACEBLOCK + iterations)
            else if (row .eq. ending) then
                inBuffer(1:SPACEBLOCK + iterations) = &
                    input((row - 1) * SPACEBLOCK - iterations + 1:row * SPACEBLOCK)
                inBuffer(SPACEBLOCK + iterations + 1:SPACEBLOCK + 2 * iterations) = \
                    input(1: iterations)[this_image() + 1]
            else
                inBuffer(1:SPACEBLOCK + 2 * iterations) = &
                input(1 + (row - 1) * SPACEBLOCK - iterations : row * SPACEBLOCK + iterations)
            end if
            call middle(SPACEBLOCK, iterations, inBuffer(:), outBuffer(:))
            output((row - 1) * SPACEBLOCK + 1:row * SPACEBLOCK) = &
                   outBuffer(iterations + 1:iterations + SPACEBLOCK)
        end if
    end do
    !$OMP END PARALLEL DO

    sync all

    deallocate(inBuffer)
    deallocate(outBuffer)
end subroutine BlockedStencil

subroutine Stencil(n, input, output, iterations)
    implicit none
    integer, intent(in) :: n, iterations
    real(KIND=8), intent(inout) :: input(n)[*], output(n)[*]

    integer :: t

    do t = TIMEBLOCK, iterations, 2 * TIMEBLOCK
        call BlockedStencil(n, input, output, TIMEBLOCK)
        call BlockedStencil(n, output, input, TIMEBLOCK)
    end do
    if (mod(iterations, TIMEBLOCK) .ne. 0) then
        call BlockedStencil(n, input, output, mod(iterations, TIMEBLOCK))
    end if
end subroutine Stencil
end module stencilModule_mt

program main
    use stencilModule_mt
    implicit none

    integer :: n, iterations
    integer :: cpu_count, cpu_count2, count_rate, count_max
    real(KIND=8), allocatable :: input(:)[:], output(:)[:]
    character(len=12), dimension(:), allocatable :: args

    allocate(args(2))
    call get_command_argument(1, args(1))
    read (unit=args(1), fmt=*) n

    call get_command_argument(2, args(2))
    read (unit=args(2), fmt=*) iterations

    if (modulo(n, SPACEBLOCK) /= 0) then
        write (*, *) 'Please make sure n is divisible by ', SPACEBLOCK
    end if

    if (modulo(iterations, 2) /= 0) then
        write (*, *) 'Please make sure iterations / TIMEBLOCK is even. Suggestion: ', &
             iterations / TIMEBLOCK / 2 * 2
    end if


    allocate(input(n / num_images())[*])
    allocate(output(n / num_images())[*])

    input = 1
    output = 1

    sync all
    call system_clock(cpu_count, count_rate, count_max)

    call Stencil(n, input, output, iterations)
    sync all
    call system_clock(cpu_count2, count_rate, count_max)

    if (this_image() .eq. 1) then
        write (*, *) 5.0 * (n - 2) * iterations / 1000000000.0 &
            / (real(cpu_count2 - cpu_count) / count_rate)
    end if

    deallocate(input)
    deallocate(output)
end program main
