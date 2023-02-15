!#define SPACEBLOCK 10000
!#define TIMEBLOCK 100
!
!module kernels
!contains
!subroutine left(n, iterations, input, output)
!    implicit none
!
!    integer, intent(in) :: n, iterations
!    real(KIND=8) :: input(n), output(n)
!
!    integer :: t, i
!    real(KIND=8) :: temp(n), constants(3)
!
!    constants = [0.50, 0.33, 0.25]
!
!    out[1] = in[1]
!
!    do t = 1, iterations
!        do i = 2, n + iterations - 1 - t
!            output(i) = sum(input(i - 1: i + 1) * constants)
!        end do
!        if (t .ne. iterations)
!            temp = input
!            input = output
!            output = temp
!        end if
!    end do
!end subroutine left
!
!subroutine middle(n, iterations, input, output)
!    implicit none
!
!    integer, intent(in) :: n, iterations
!    real(KIND=8) :: input(n), output(n)
!
!    integer :: t, i
!    real(KIND=8) :: temp(n), constants(3)
!
!    constants = [0.50, 0.33, 0.25]
!
!    out[1] = in[1]
!
!    do t = 1, iterations
!        do i = 2 + t, n + 2 * iterations - 1 - t
!            output(i) = sum(input(i - 1: i + 1) * constants)
!        end do
!        if (t .ne. iterations)
!            temp = input
!            input = output
!            output = temp
!        end if
!    end do
!end subroutine middle
!
!subroutine right(n, iterations, input, output)
!    implicit none
!
!    integer, intent(in) :: n, iterations
!    real(KIND=8) :: input(n), output(n)
!
!    integer :: t, i
!    real(KIND=8) :: temp(n), constants(3)
!
!    constants = [0.50, 0.33, 0.25]
!
!    out[n + iterations] = in[n + iterations]
!
!    do t = 1, iterations
!        do i = 2 + t, n + iterations - 1
!            output(i) = sum(input(i - 1: i + 1) * constants)
!        end do
!        if (t .ne. iterations)
!            temp = input
!            input = output
!            output = temp
!        end if
!    end do
!end subroutine right
!end module kernels
!
!module stencilModule
!contains
!subroutine Stencil(n, input, output, iterations)
!    use kernels
!    implicit none
!    integer, intent(in) :: n, iterations
!    real(KIND=8), intent(inout) :: input(n), output(n)
!
!    real(KIND=8), allocatable :: inBuffer(:), outBuffer(:)
!    integer :: t
!
!    allocate(inBuffer(n + 2 * iterations))
!    allocate(outBuffer(n + 2 * iterations))
!
!    do row = 1, n / SPACEBLOCK
!        if (row .eq. 1)
!            inBuffer(1:SPACEBLOCK + iterations) = input(1:SPACEBLOCK + iterations)
!            left(SPACEBLOCK, iterations, inBuffer(:), outBuffer(:))
!            output(1:SPACEBLOCK) = outBuffer(1:SPACEBLOCK)
!        else if (row .eq. n / SPACEBLOCK)
!            inBuffer(1:SPACEBLOCK + iterations) = \
!                input(1 + (row - 1) * SPACEBLOCK - iterations:SPACEBLOCK + iterations)
!            right(SPACEBLOCK, iterations, inBuffer(:), outBuffer(:))
!            output(1:SPACEBLOCK) = outBuffer(1:SPACEBLOCK)
!        else
!            inBuffer(1:SPACEBLOCK + iterations) = input(1:SPACEBLOCK + iterations)
!            middle(SPACEBLOCK, iterations, inBuffer(:), outBuffer(:))
!            output(1:SPACEBLOCK) = outBuffer(1:SPACEBLOCK)
!        end if
!    end do
!
!    deallocate(inBuffer)
!    deallocate(outBuffer)
!
!end subroutine stencil
!end module stencilModule
!
!program main
!    use stencilModule
!    implicit none
!
!    integer :: n, p, iterations
!    integer :: cpu_count, cpu_count2, count_rate, count_max
!    real(KIND=8), dimension(:,:), codimension[:], allocatable :: input, output
!    character(len=12), dimension(:), allocatable :: args
!
!    allocate(args(2))
!    call get_command_argument(1, args(1))
!    read (unit=args(1), fmt=*) n
!
!    call get_command_argument(2, args(2))
!    read (unit=args(2), fmt=*) iterations
!
!    p = num_images()
!
!    if (modulo(n, p) /= 0) then
!        write (*, *) 'Please make sure n divides p'
!    end if
!
!    if (modulo(iterations, 2) /= 0) then
!        write (*, *) 'Please make sure the number of iterations is even'
!    end if
!
!    allocate(input(n, n / p)[*])
!    allocate(output(n, n / p)[*])
!
!    input = 1
!    output = 1
!
!    sync all
!    call system_clock(cpu_count, count_rate, count_max)
!
!    call stencil(n, input, output, iterations)
!    sync all
!    call system_clock(cpu_count2, count_rate, count_max)
!
!    if (this_image() .eq. 1) then
!        write (*, *) 9.0 * (n - 2) * (n - 2) * iterations / 1000000000.0 &
!            / (real(cpu_count2 - cpu_count) / count_rate)
!    end if
!end program main
