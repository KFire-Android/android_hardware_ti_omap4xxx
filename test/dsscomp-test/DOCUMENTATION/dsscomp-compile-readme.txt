arm-none-linux-gnueabi-gcc --static -pthread -Wall main.c dsscomp_reset_ovls.c dsscomp_disp_reset.c test_image_disp.c test_all_ovls.c ion.c test_scaling.c test_chk_ovls.c test_nv12.c test_buf_types.c  test_uyvy.c test_nv12_scaling.c test_uyvy_scaling.c -o dsscomp_test

see DOCUMENTATION/pvr-simulator-readme.txt for pvr-simulator test code for
dsscomp 
