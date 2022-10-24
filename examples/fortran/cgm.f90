! From https://en.wikipedia.org/wiki/Conjugate_gradient_method
! A must be symmetric positive semi-definite
subroutine cgm(A, b, n, m, max_iterations, error, x)! result(x)
    implicit none
    integer, intent(in) :: n, m, max_iterations
    real, intent(in) :: A(n, m), b(n), error
    real, intent(out) :: x(m)

!    real, allocatable :: x(:), p(:), residual(:)
    real, allocatable :: p(:), residual(:), Ap(:)
    real :: alpha, beta, residualNorm
    integer :: k

!    allocate(x(m))
    allocate(p(m))
    allocate(Ap(m))
    allocate(residual(m))

    x = 1
    residual = b - matmul(A, x)
    p = residual

    do k = 1, max_iterations
        Ap = matmul(A, p)
        alpha = sum(residual * residual) / sum(p * Ap)
        x = x + alpha * p
        residual = residual - alpha * Ap
        residualNorm = sqrt(sum(residual * residual))
        ! stopping condition
        if (residualNorm .le. error) then
            exit
        endif
        beta = sum(residual * residual)
        p = residual + beta * p
    end do

    deallocate(p)
    deallocate(residual) 

end subroutine cgm

program main
    implicit none

    integer :: n, max_iterations
    real, allocatable :: A(:, :)
    real, allocatable :: x(:), b(:)
    real :: error
    character(len=12), dimension(:), allocatable :: args

    allocate(args(2)) 
    call get_command_argument(1, args(1))
    call get_command_argument(2, args(2))
    read (unit=args(1),fmt=*) n
    read (unit=args(2),fmt=*) max_iterations 

    allocate(A(n, n))
    allocate(b(n))
    allocate(x(n))

    ! This is not positive semi-definitive as all columns are linearly dependent
    A = 1
    b = 1
    error = 0.01

    call cgm(A, b, n, n, max_iterations, error, x) 

    write (*, *) x(1)
end program main
