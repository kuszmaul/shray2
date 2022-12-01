! Program argument is a file name <name>. In the same directory there are files
! <name>_info<s>, <name>_column<s> for image numbers 1, ..., num_images().
! info contains 3 numbers, and column contains a space separated file of this many
! double precision floats.

program main
    implicit none

    integer :: info_io, column_io
    integer(KIND=8) :: nz
    integer :: M, N, s
    integer, allocatable :: col_ind(:)
    character(len=12), dimension(:), allocatable :: args
    character(len=100) :: numberString, info, column, filename

    allocate(args(1))
    call get_command_argument(1, args(1))
    read (unit=args(1), fmt=*) filename

    s = this_image()

    write(numberString, fmt='(I0)') s
    info = trim(filename) // trim("_info") // trim(numberString)
    column = trim(filename) // trim("_column") // trim(numberString)

    open(newunit=info_io, file=info)
    read(info_io, *) M, N, nz
    close(info_io)

    write (*, *) M, N, nz

    allocate(col_ind(nz))

    open(newunit=column_io, file=column)
    read(column_io, *) col_ind
    close(column_io)

end program main
