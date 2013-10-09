Kernel:
1. Copy pvr-simulator/pvr-simulator directory to kernel/drivers/video/omap2/dsscomp/pvr_simulator
	PVR simulator kernel patch is available at
		pvr-simulator/0001-OMAPDSS-PVRSIM-Add-PVR-Simulator-for-testing-DSSCOMP.patch
2. Compile the kernel with pvr-simulator enabled
3. Use the zImage generated to create boot.img and use it for boot


Test code compilation:
arm-none-linux-gnueabi-gcc --static -pthread -Wall pvr-simulator/dsscomp_test/dsscomp_lcd_gralloc.c ion.c -o dsscomp_lcd_gralloc

For WB (Use this TC only for validating WB capture mode):
arm-none-linux-gnueabi-gcc --static -pthread -Wall pvr-simulator/dsscomp_test/dsscomp_lcd_gralloc_wb.c ion.c -o dsscomp_lcd_gralloc_wb

Execute:
Copy the test binary to the target
Copy TEST_IMAGES/176X144/yc.rgb32 to the test directory
Copy TEST_IMAGES/176X144/test_176x144_30fps.yuv to the test directory

Run: ./dsscomp_lcd_gralloc test=24
 It will give 3 options:
 1. PLAY A VIDEO FILE (TO VERIFY SYNC & QUEUING)           
 	The Video is Played with TILER 2D BUFFERS Using YUV-420 NV12 format 
 2. VERIFY VARIOUS STATES OF COMPOSITION           
 	Displays a NV12 image on the screen with TILER 2D buffers. This verifies all 
 	the states of the DSS composition, ACTIVE, APPLYING, APPLIED, PROGRAMMED
 	DISPLAYED and RELEASED.
 3. TO VERIFY NON-TILER BUFFER MAPPING IN DSSCOMP            
 	The ARGB32 Image is displayed with mapped TILER 1D buffers in DSSCOMP. 
