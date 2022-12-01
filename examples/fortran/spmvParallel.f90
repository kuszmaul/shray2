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
        allocate(output(blockSize))

!        do concurrent (row = 1:mat%m)
         do row = 1,mat%m
!            print '(/,3x,"image = ",i6, 3x, "row = ",i6)', this_image(), row
            output(row) = 0.0
            do j = mat%row_ptr(row), mat%row_ptr(row + 1) - 1
                col = mat%col_ind(j)
                localIndex = modulo(col - 1, blockSize) + 1
                processor = (col - 1) / blockSize + 1
!                print '(/,3x,"image = ",i6, 3x, "global index = ",i6, 3x, "local index = ",i6)', &
!                    this_image(), col, localIndex
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
        real(KIND=8), allocatable :: state(:)

        real(KIND=8), allocatable :: input(:)[:]
        integer :: i

        allocate(input((mat%n + this_image() - 1) / this_image())[*])

        state = 1 / mat%n;

        do i = 1,iterations
!            write(*, *) i
            input = spmv(mat, input)
            sync all
            input = state
        end do

        state = input

        deallocate(input)
    end function

end module CSR_SPMV


program main
    use CSR_SPMV
    implicit none

    integer :: info_io, row_io, column_io, values_io, s
    integer(KIND=8) :: nz
    character(len=12), dimension(:), allocatable :: args
    character(len=100) :: numberString, info, row, column, values, filename
    type(CSRMatrix) :: mat
    real(KIND=8), allocatable :: res(:)

    ! Parse the distributed array
    allocate(args(1))
    call get_command_argument(1, args(1))
    read (unit=args(1), fmt=*) filename

    s = this_image()

    write(numberString, fmt='(I0)') s
    info = trim(filename) // trim("_info") // trim(numberString)
    column = trim(filename) // trim("_column") // trim(numberString)
    row = trim(filename) // trim("_row") // trim(numberString)
    values = trim(filename) // trim("_values") // trim(numberString)

    open(newunit=info_io, file=info)
    read(info_io, *) mat%m, mat%n, nz
    close(info_io)

    allocate(mat%val(nz))
    allocate(mat%row_ptr(mat%m + 1))
    allocate(mat%col_ind(nz))
    allocate(res(mat%n))

    open(newunit=row_io, file=row)
    read(row_io, *) mat%row_ptr
    close(row_io)

    open(newunit=column_io, file=column)
    read(column_io, *) mat%col_ind
    close(column_io)

    open(newunit=values_io, file=values)
    read(values_io, *) mat%val
    close(values_io)

    res = steady_state(mat, 1)

end program main
