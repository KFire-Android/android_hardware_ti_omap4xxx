Apply the kernel patches on the kernel.

Compilation:
arm-none-linux-gnueabi-gcc --static -pthread -Wall dsscomp_hdmi.c ion.c uevent.c -o dsscomp_hdmi

Execute:
./dsscomp_hdmi test=2 --> deep color mode
	select 0-->24 bit
	select 1-->30 bit
	select 2-->36 bit
Observe the HDMI_TX window for colour Dep for verification in virtio

./dsscomp_hdmi test=3 --> timing related tests
	select the proper timing which needs to be tested from the options printed on the terminal
	Observe HDMI_W2 for verifying the configuration in virtio
Note 1:	For timing related test EDID needs to be updated in the zebu. FOr virtion it is fake EDID which is used for testing.
		Refer to the verification patch for adding fake EDID for virtio. FOr Zebu EDID can be updated in the HDMI panel window.
Note 2:	Please enable HDMI displaye before starting the tests by running following command
		echo "1" > /sys/devices/platform/omapdss/display1/enabled
		
		