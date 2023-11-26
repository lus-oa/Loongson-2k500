ip link set can0 type can bitrate 500000 triple-sampling on
ifconfig can0 up
cansend can0 125#02.03.04.05.06.06.77.88
candump