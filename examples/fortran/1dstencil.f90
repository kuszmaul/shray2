#define SPACEBLOCK 10000
#define TIMEBLOCK 100

module kernels
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
end module kernels

module stencilModule
contains
subroutine BlockedStencil(n, input, output, iterations)
    use kernels
    implicit none
    integer, intent(in) :: n, iterations
    real(KIND=8), intent(inout) :: input(n)[*], output(n)[*]

    real(KIND=8), allocatable :: inBuffer(:), outBuffer(:)
    integer :: row, blocksize, ending

    allocate(inBuffer(n + 2 * iterations))
    allocate(outBuffer(n + 2 * iterations))

    blocksize = (n / SPACEBLOCK + num_images() - 1) / num_images()
    ending = min(blocksize, n / SPACEBLOCK - (num_images() - 1) * blocksize)

    do row = 1, ending
        if ((row .eq. 1) .and. (this_image() .eq. 1)) then
            inBuffer(1:SPACEBLOCK + iterations) = input(1:SPACEBLOCK + iterations)
            call left(SPACEBLOCK, iterations, inBuffer(:), outBuffer(:))
            output(1:SPACEBLOCK) = outBuffer(1:SPACEBLOCK)
        else if ((this_image() .eq. num_images()) .and. (row .eq. ending)) then
            inBuffer(1:SPACEBLOCK + iterations) = \
                input(1 + (row - 1) * SPACEBLOCK - iterations:SPACEBLOCK + iterations)
            call right(SPACEBLOCK, iterations, inBuffer(:), outBuffer(:))
            output(1:SPACEBLOCK) = outBuffer(1:SPACEBLOCK)
        else
            if (row .eq. 1) then
                inBuffer(iterations + 1:SPACEBLOCK + 2 * iterations) = \
                    input(1:SPACEBLOCK + iterations)
                inBuffer(1:iterations) = \
                    input(blocksize - iterations + 1: blocksize)[this_image() - 1]
            else if (row .eq. ending) then
                inBuffer(1:SPACEBLOCK + iterations) = input(1:SPACEBLOCK + iterations)
            else
                inBuffer(1:SPACEBLOCK + 2 * iterations) = \
                input(1 + row * SPACEBLOCK - iterations : row * SPACEBLOCK + iterations)
            end if
            call middle(SPACEBLOCK, iterations, inBuffer(:), outBuffer(:))
            output(1:SPACEBLOCK) = outBuffer(1:SPACEBLOCK)
        end if
    end do

    sync all

    deallocate(inBuffer)
    deallocate(outBuffer)
end subroutine BlockedStencil

subroutine Stencil(n, input, output, iterations)
    implicit none
    integer, intent(in) :: n, iterations
    real(KIND=8), intent(inout) :: input(n)[*], output(n)[*]

    integer :: t
    real(KIND=8), allocatable :: temp(:)[:]

    do t = TIMEBLOCK, iterations, TIMEBLOCK
        call BlockedStencil(n, input, output, TIMEBLOCK)
        temp = output
        output = input
        input = temp
    end do
    if (mod(iterations, TIMEBLOCK) .ne. 0) then
        call BlockedStencil(n, input, output, mod(iterations, TIMEBLOCK))
    else
        ! We did one buffer swap too many
        temp = output
        output = input
        input = temp
    end if
end subroutine Stencil
end module stencilModule

program main
    use stencilModule
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
        write (*, *) 'Please make sure n divides ', SPACEBLOCK
    end if

    allocate(input(n)[*])
    allocate(output(n)[*])

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
