! An n-body simulation. Still needs to be blocked

module accelerateModule
contains
function accelerate(pos1, pos2, mass) result(a)
    implicit none

    real(KIND=8), intent(in) :: pos1(3), pos2(3)
    real(KIND=8), intent(in) :: mass
    real(KIND=8) :: norm
    real(KIND=8) :: a(3)
    
    norm = ((pos2(1) - pos1(1)) ** 2 + (pos2(2) - pos1(2)) ** 2 + &
            (pos2(3) - pos1(3)) ** 2) ** 1.5

    a = (pos2 - pos1) * mass / norm
end function accelerate

subroutine accelerateAll(accel, positions, masses, n)
    implicit none

    integer, intent(in) :: n
    real(KIND=8) :: positions(n / num_images(), 3)[*]
    real(KIND=8) :: accel(n / num_images(), 3)[*]
    real(KIND=8) :: masses(n / num_images())[*]

    integer :: blockFactor, i, j, s, I1, J1

    blockFactor = 100

    do I1 = 1, n / num_images(), blockFactor
        do J1 = 1, n / num_images(), blockFactor 
    do i = I1, MIN(I1 + blockFactor, n / num_images())
        accel(i,:) = 0
        ! Calculate acceleration with respect to processor s
        do s = 1, num_images()
        do j = J1, MIN(J1 + blockFactor, n / num_images())
                accel(i,:) = accel(i,:) + &
                             accelerate(positions(i,:), positions(j,:)[s], masses(j)[s])
        end do
        end do
    end do
        end do
    end do
end subroutine accelerateAll
end module accelerateModule


module nbodyModule
contains
subroutine advanceIt(positions, velocities, masses, accel, dt, n)
    use accelerateModule
    implicit none

    integer, intent(in) :: n
    real(KIND=8), intent(in) :: dt
    real(KIND=8) :: masses(n / num_images())[*]
    real(KIND=8) :: positions(n / num_images(), 3)[*], velocities(n / num_images(), 3)[*]
    real(KIND=8) :: accel(n / num_images(), 3)[*]

    call accelerateAll(accel, positions, masses, n)
    velocities = velocities + accel * dt
    positions = velocities * dt
    sync all

end subroutine advanceIt
end module nbodyModule

program main
    use nbodyModule
    implicit none

    integer :: n, iterations, p, t
    real(KIND=8), dimension(:,:), codimension[:], allocatable :: positions, velocities, accel
    real(KIND=8), dimension(:), codimension[:], allocatable :: masses
    real(KIND=8) :: dt
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

    allocate(positions(n / p, 3)[*])
    allocate(velocities(n / p, 3)[*])
    allocate(accel(n / p, 3)[*])
    allocate(masses(n / p)[*])

    positions = 1
    masses = 1
    velocities = 0

    dt = 0.1

    sync all

    do t = 1, iterations
        call advanceIt(positions, velocities, masses, accel, dt, n)
    end do
end program main


