! A naive 5 point stencil. Only works on an even number of iterations.

! Does two iterations, overwriting input with the result after two iterations.
! Also assumes the boundary of input and output are equal at the start
module relaxModule
contains
subroutine relax(n, input, output)
    implicit none

    integer, intent(in) :: n
    real(KIND=8) :: input(n, n)[*]
    real(KIND=8) :: output(n, n)[*]

    integer :: i, j, s, p
    real(KIND=8) :: center
    real(KIND=8) :: lower
    real(KIND=8) :: upper
    real(KIND=8) :: left
    real(KIND=8) :: right

    center = 0.25
    lower = 0.25
    upper = 0.25
    left = 0.25
    right = 0.25

    s = this_image()
    p = num_images()

    ! Inner part of the computation
    do j = 2, n / p - 1
        do i = 2, n - 1
            output(i, j) = center * input(i, j) + &
                       lower * input(i - 1, j) + &
                       upper * input(i + 1, j) + &
                       left * input(i, j - 1) + &
                       right * input(i, j + 1)
        end do
    end do

    ! Left-most column
    if (s /= 1) then
        do i = 2, n - 1
            output(i, 1) = center * input(i, 1) + &
                          lower * input(i - 1, 1) + &
                          upper * input(i + 1, 1) + &
                          left * input(i, n / p)[s - 1] + &
                          right * input(i, 2)

        end do
    end if

    ! Right-most column
    if (s /= p) then
        do i = 2, n - 1
            output(i, n / p) = center * input(i, n / p) + &
                            lower * input(i - 1, n / p) + &
                            upper * input(i + 1, n / p) + &
                            left * input(i, n / p) + &
                            right * input(i, 1)[s + 1]
        end do
    end if
end subroutine relax
end module relaxModule

! Writes the output to in if the number of iterations is even, otherwise to out.
! Assumes the boundary of the output is initialized to the input
module stencilModule
contains
subroutine stencil(n, input, output, iterations)
    use relaxModule
    implicit none
    integer, intent(in) :: n
    integer, intent(in) :: iterations
    real(KIND=8), intent(inout) :: input(n,n)[*]
    real(KIND=8), intent(inout) :: output(n,n)[*]
    integer :: t

    do t = 1, iterations / 2
        call relax(n, input, output)
        sync all
        call relax(n, output, input)
        sync all
    end do

end subroutine stencil
end module stencilModule

program main
    use stencilModule
    implicit none

    integer :: n, p, iterations
    integer :: cpu_count, cpu_count2, count_rate, count_max
    real(KIND=8), dimension(:,:), codimension[:], allocatable :: input, output
    character(len=12), dimension(:), allocatable :: args

    allocate(args(2))
    call get_command_argument(1, args(1))
    read (unit=args(1), fmt=*) n

    call get_command_argument(2, args(2))
    read (unit=args(2), fmt=*) iterations

    p = num_images()

    if (modulo(n, p) /= 0) then
        write (*, *) 'Please make sure n divides p'
    end if

    if (modulo(iterations, 2) /= 0) then
        write (*, *) 'Please make sure the number of iterations is even'
    end if

    allocate(input(n, n / p)[*])
    allocate(output(n, n / p)[*])

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


