module dist
contains
    function loc_ind(j, blocksize) result(i)
        integer(8), intent(in) :: j, blocksize
        integer(8) :: i

        i = modulo(j - 1, blocksize) + 1
    end function loc_ind

    function loc_proc(j, blocksize) result(s)
        integer(8), intent(in) :: j, blocksize
        integer(8) :: s

        s = (j - 1) / blocksize + 1
    end function loc_proc

    ! j is global index
    function glob_r(a, j) result(res)
        integer(8), intent(in) :: j
        double precision, intent(in) :: a(:)[*]
        double precision :: res
        integer(8) :: blocksize

        blocksize = size(a)

        res = a(loc_ind(j, blocksize))[loc_proc(j, blocksize)]
    end function glob_r

    ! j is global index
    function glob_i(a, j) result(res)
        integer(8), intent(in) :: j
        integer(8), intent(in) :: a(:)[*]
        integer(8) :: res
        integer(8) :: blocksize

        blocksize = size(a)

        res = a(loc_ind(j, blocksize))[loc_proc(j, blocksize)]
    end function glob_i

    ! j is local index
    function loc_r(a, j) result(res)
        integer(8), intent(in) :: j
        double precision, intent(in) :: a(:)[*]
        double precision :: res
        integer(8) :: blocksize, k

        blocksize = size(a)
        k = (this_image() - 1) * blocksize + j

        res = a(loc_ind(k, blocksize))[loc_proc(k, blocksize)]
    end function loc_r

    ! j is local index
    function loc_i(a, j) result(res)
        integer(8), intent(in) :: j
        integer(8), intent(in) :: a(:)[*]
        integer(8) :: res
        integer(8) :: blocksize, k

        blocksize = size(a)
        k = (this_image() - 1) * blocksize + j

        res = a(loc_ind(k, blocksize))[loc_proc(k, blocksize)]
    end function loc_i

    function all_sum(a) result(res)
        double precision, intent(in) :: a
        double precision :: res
        double precision, allocatable :: all_a[:]
        integer :: i

        allocate(all_a[*])
        all_a = a
        sync all

        res = 0

        do i = 1, num_images()
            res = res + all_a[i]
        end do

        deallocate(all_a)
    end function all_sum

end module dist

program main
    use dist
    implicit none

    integer(8) :: na, nz, i, nz_block
    integer(8), allocatable :: colidx(:)[:]
    double precision :: a, a_acc
    double precision, allocatable :: a_all[:]

    write(*, *) "Hello from rank ", this_image(), " out of ", num_images()
    na = 1000
    nz = 10000
    nz_block = (nz + num_images() - 1) / num_images()

    allocate(colidx(nz_block)[*], a_all[*])

    do i = 1, nz_block
        colidx(i) = (this_image() - 1) * nz_block + i
    end do

    sync all

    if (this_image() .eq. 1) then
        do i = 1, nz
            write(*, '(A, I5, A, I5)') "Index: ", i, &
                ", value (should be index): ", glob_i(colidx, i)
        end do
    end if

    sync all

    a = this_image()
    a_all = this_image()
    sync all
    write(*, *) a_all[1], a_all[2]

    a_acc = 0
    do i = 1, num_images()
        write(*, *) this_image(), i, a_all[i]
        a_acc = a_acc + a_all[i]
    end do
    a = a_acc

    write(*, *) a, " should be ", num_images() * (num_images() + 1) / 2

    deallocate(colidx)

end program main
