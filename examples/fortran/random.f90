module randomModule
contains

! With probability local_prob% gives a number in [local_start, local_end[, with
! probability (100 - local_prob)% gives a number in [0, n[.
function rand_n_prob(local_start, local_end, local_prob, input_size) result(j)
    implicit none
    integer, intent(in) :: local_start, local_end, local_prob, input_size
    integer :: randomInt
    integer :: j

    real(KIND=8) :: r

    call random_number(r)

    if (r .le. local_prob / 100) then
        call random_number(r)
        j = local_start + floor(r * (local_end - local_start))
    else
        call random_number(r)
        randomInt = floor(r * (input_size - local_end + local_start))
        if (randomInt .ge. local_start) then
            j = randomInt + local_end - local_start
        else
            j = randomInt
        endif
    endif
end function

function random_fill(n, p, local_prob, input, input_size) result(res)
    implicit none
    integer, intent(in) :: n, local_prob, p, input_size
    real(KIND=8) :: input(n / p)[*]
    real(KIND=8) :: res
    integer :: j, s, i

    res = 0.0

    s = this_image()

    do i = 1, n
        j = rand_n_prob(s * input_size / p, (s + 1) * input_size / p, local_prob, input_size)
        res = res + input(mod(j, input_size / p) + 1)[j / (input_size / p) + 1]
    enddo
end function
end module

program main
    use randomModule
    use iso_fortran_env, only : stderr=>ERROR_UNIT
    implicit none

    integer :: n, local_start, local_end, local_prob, p, s, input_size
    real(KIND=8), dimension(:), codimension[:], allocatable :: input
    real(KIND=8) :: res
    integer :: cpu_count, cpu_count2, count_rate, count_max
    character(len=12), dimension(:), allocatable :: args

    allocate(args(3))
    call get_command_argument(1, args(1))
    read (unit=args(1), fmt=*) input_size

    call get_command_argument(2, args(2))
    read (unit=args(2), fmt=*) n

    call get_command_argument(3, args(3))
    read (unit=args(3), fmt=*) local_prob

    p = num_images()
    s = this_image()

    ! start inclusive, end exclusive
    local_start = s * input_size / p
    local_end = (s + 1) * input_size / p

    if (modulo(input_size, p) /= 0) then
        write (*, *) 'Please make sure p divides input_size'
    end if

    allocate(input(input_size / p)[*])

    input = 1

    sync all

    call system_clock(cpu_count, count_rate, count_max)
    res = random_fill(n, p, local_prob, input, input_size)

    sync all
    call system_clock(cpu_count2, count_rate, count_max)

    if (s .eq. 1) then
        write (*, *) real(cpu_count2 - cpu_count) / count_rate
    end if


    if (res .eq. n) then
        write (0, *) "Success!"
    else
        write (0, *) "Failure!"
    end if

    deallocate(input)
end program main
