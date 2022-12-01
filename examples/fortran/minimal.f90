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
    function spmv(mat, input, global_cols) result(output)
        implicit none
        type(CSRMatrix), intent(in) :: mat
        real(KIND=8), intent(in) :: input((global_cols + num_images() - 1) / num_images())[*]
        real(KIND=8), allocatable :: output(:)

        integer :: row, col, j, blockSize, processor, localIndex, global_cols

        ! blocksize needs to know the global size
        blockSize = (global_cols + num_images() - 1) / num_images()
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
    function steady_state(mat, iterations, global_cols) result(state)
        implicit none
        type(CSRMatrix), intent(in) :: mat
        integer, intent(in) :: iterations, global_cols
        real(KIND=8), allocatable :: state(:), input(:)[:]
        integer :: t

        allocate(input(mat%m)[*])
        allocate(state(mat%m))

        state = 1 / mat%n;

        do t = 1,iterations
            input = spmv(mat, input, global_cols)
            sync all
        end do

        state = input

        deallocate(input)
    end function


end module CSR_SPMV


program main
    use CSR_SPMV
    implicit none

    type(CSRMatrix) :: mat
    real(KIND=8), allocatable :: res(:)
!    real(KIND=8), allocatable :: input(:)[:]
    integer :: s

    s = this_image()

    if (s .eq. 1) then
        mat%m = 3
        mat%n = 5
        allocate(res(mat%m))
!        allocate(input(3)[*])
        mat%val = [0.666666666666667, 0.366555998208319, 0.300110668458348, 0.366555998208319, &
            0.300110668458348, 0.100036889486116, 0.533407112305565, 0.200073778972232, &
            0.122185332736106, 0.577703998805546, 0.244370665472212]
        mat%row_ptr = [1, 6, 9, 11]
        mat%col_ind = [1, 2, 3, 4, 5, 1, 2, 4, 1, 3, 5]
    else
        mat%m = 2
        mat%n = 5
        allocate(res(mat%m))
!        allocate(input(2)[*])
        mat%val = [0.050018444743058, 0.100036889486116, 0.283314888590275, 0.183277999104159, &
            0.061092666368053, 0.122185332736106, 0.150055334229174, 0.272240666965280]
        mat%row_ptr = [1, 5, 8]
        mat%col_ind = [1, 2, 4, 5, 1, 3, 4, 5]
    end if

    res = steady_state(mat, 2, 5)
end program main
