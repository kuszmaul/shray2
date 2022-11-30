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
    function spmv(mat, input) result(output)
        implicit none
        type(CSRMatrix), intent(in) :: mat
        real(KIND=8), intent(in) :: input(:)
        real(KIND=8) :: output(mat%m)

        integer :: row, j

        do concurrent (row = 1:mat%m)
            output(row) = 0.0
            do j = mat%row_ptr(row), mat%row_ptr(row + 1) - 1
                output(row) = output(row) + mat%val(j) * input(mat%col_ind(j))
            end do
        end do

   end function spmv

end module CSR_SPMV


program main
    use CSR_SPMV
    implicit none


end program main
