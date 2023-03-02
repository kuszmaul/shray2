! https://people.eecs.berkeley.edu/~aydin/csb2009.pdf

module CSR_SPMV
    implicit none

    type, public :: CSRMatrix
        ! The matrix is m x n
        integer :: m
        integer :: n
        ! Stores the non-zeroes
        real(KIND=8), allocatable :: val(:)
        ! The first non-zero of row k is the row_ptr(k)th non-zero overall.
        ! row_ptr(m + 1) is the total number of non-zeroes.
        integer, allocatable :: row_ptr(:)
        ! The kth non-zero has column-index col_ind(k)
        integer, allocatable :: col_ind(:)
    end type

contains
    ! Computes the local part of matmul(mat, output), where mat
    ! is the local part of a sparse matrix distributed blockwise along
    ! the first dimension.
    function spmv(mat, input) result(output)
        implicit none
        type(CSRMatrix), intent(in) :: mat
        real(KIND=8), intent(in) :: input((mat%n + num_images() - 1) / num_images())[*]
        real(KIND=8), allocatable :: output(:)

        integer :: row, col, j, blockSize, processor, localIndex

        blockSize = (mat%n + num_images() - 1) / num_images()
        allocate(output(mat%m))

        do concurrent (row = 1:mat%m)
            output(row) = 0.0
            do j = mat%row_ptr(row), mat%row_ptr(row + 1) - 1
                col = mat%col_ind(j)
                localIndex = modulo(col - 1, blockSize) + 1
                processor = (col - 1) / blockSize + 1
                output(row) = output(row) + mat%val(j) * input(localIndex)[processor]
            end do
        end do
    end function spmv

    ! Computes the local part of the steady state vector of a distributed stochastic matrix.
    ! mat is the local part of this matrix, distributed blockwise along the first dimension.
    function steady_state(mat, iterations) result(state)
        implicit none
        type(CSRMatrix), intent(in) :: mat
        integer, intent(in) :: iterations
        real(KIND=8), allocatable :: state(:), input(:)[:]
        integer :: t

        allocate(input(mat%m)[*])
        allocate(state(mat%m))

        state = 1 / mat%n

        do t = 1,iterations
            input = spmv(mat, input)
            sync all
        end do

        state = input

        deallocate(input)
    end function

end module CSR_SPMV


program main
    use CSR_SPMV
    implicit none

    integer :: iterations
    ! Timing
    integer :: cpu_count, cpu_count2, count_rate, count_max
    integer(KIND=8) :: totalNz
    integer :: n, global_row, blockSize, row, eyes, i
    character(len=12) :: iterationArg, nArg
    type(CSRMatrix) :: mat
    real(KIND=8), allocatable :: res(:)
    real(KIND=8) :: probabilities(12)

    ! Parse the distributed array
    call get_command_argument(1, nArg)
    call get_command_argument(2, iterationArg)
    read (nArg, *) n
    read (iterationArg, *) iterations

    blockSize = (n + num_images() - 1) / num_images()
    mat%m = merge(n - (num_images() - 1) * blockSize, blockSize, this_image() .eq. num_images())
    mat%n = n
    allocate(mat%val(11 * mat%m))
    allocate(mat%row_ptr(mat%m + 1))
    allocate(mat%col_ind(11 * mat%m))
    allocate(res(mat%n))

    probabilities = [0.0, 1.0 / 36, 2.0 / 36, 3.0 / 36, 4.0 / 36, 5.0 / 36, 6.0 / 36, &
                     5.0 / 36, 4.0 / 36, 3.0 / 36, 2.0 / 36, 1.0 / 36]
    i = 0
    do row = 1, mat%m
        global_row = (this_image() - 1) * blockSize + row
        do eyes = 2, 12
            mat%val(i) = probabilities(eyes)
            mat%col_ind(i) = modulo(global_row + eyes, n) + 1
            i = i + 1
        end do
        mat%row_ptr(row) = 11 * (row - 1) + 1
    end do
    mat%row_ptr(mat%m + 1) = 11 * mat%m + 1

    call system_clock(cpu_count, count_rate, count_max)

    res = steady_state(mat, iterations)

    call system_clock(cpu_count2, count_rate, count_max)

    totalNz = 11 * n
    if (this_image() .eq. 1) then
        write (*, *) 2.0 * real(totalNz) * iterations / 1000000000.0 / &
            (real(cpu_count2 - cpu_count) / count_rate)
    end if
end program main
