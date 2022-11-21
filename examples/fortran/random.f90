module randomModule
contains

function rand_n_prob(n, local_start, local_end, local_prob) result(j)
    implicit none
    integer, intent(in) :: n, local_start, local_end, local_prob
    integer :: randomInt
    integer :: j

    real(KIND=8) :: r

    call random_number(r)

    if (r .le. local_prob / 100) then
        call random_number(r)
        j = local_start + floor(r * (local_end - local_start)) + 1
    else
        call random_number(r)
        randomInt = floor(r * (n - local_end + local_start)) + 1
        if (randomInt .ge. local_start) then
            j = randomInt + local_end - local_start
        else
            j = randomInt
        endif
    endif
end function

subroutine random_fill(n, p, local_prob, input, output)
    implicit none
    integer, intent(in) :: n, local_prob, p
    real(KIND=8) :: input(n / p)[*], output(n / p)[*]
    integer :: j, s, i

    s = this_image()

    do i = 1, n / p
        j = rand_n_prob(n, s * n / p + 1, (s + 1) * n / p + 1, local_prob)
        output(i) = input(mod(j, n / p) + 1)[j / (n / p) + 1]
    enddo
end subroutine
end module

program main
    use randomModule
    implicit none

    integer :: n, local_start, local_end, local_prob, p, s
    real(KIND=8), dimension(:), codimension[:], allocatable :: input, output
    integer :: cpu_count, cpu_count2, count_rate, count_max
    character(len=12), dimension(:), allocatable :: args

    allocate(args(2))
    call get_command_argument(1, args(1))
    read (unit=args(1), fmt=*) n

    call get_command_argument(2, args(2))
    read (unit=args(2), fmt=*) local_prob

    p = num_images()
    s = this_image()

    local_start = s * n / p + 1
    local_end = (s + 1) * n / p + 1

    if (modulo(n, p) /= 0) then
        write (*, *) 'Please make sure n divides p'
    end if

    allocate(input(n / p)[*])
    allocate(output(n / p)[*])

    input = 1

    sync all

    call system_clock(cpu_count, count_rate, count_max)
    call random_fill(n, p, local_prob, input, output)

    sync all
    call system_clock(cpu_count2, count_rate, count_max)

    write (*, *) 'This took ', real(cpu_count2 - cpu_count) / count_rate

    deallocate(input)
    deallocate(output)
end program main
