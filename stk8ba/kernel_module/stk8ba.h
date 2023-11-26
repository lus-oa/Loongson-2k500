#ifndef STK8BA_H
#define STK8BA_H

/* STK8BA寄存器 */
#define STK8BA_CHIP_ID	0x00	                /* 芯片ID寄存器     */
#define STK8BA_XOUT1	0x02 
#define STK8BA_XOUT2	0x03 
#define STK8BA_YOUT1	0x04 
#define STK8BA_YOUT2	0x05 
#define STK8BA_ZOUT1	0x06 
#define STK8BA_ZOUT2	0x07 
#define STK8BA_INTMAP1	0x19
#define STK8BA_INTMAP2	0x1a
#define STK8BA_INTSTS1  0x09
#define STK8BA_INTSTS2  0x0A
#define STK8BA_EVENTINFO1 0x0B
#define STK8BA_DATASETUP 0x13
#define STK8BA_SWRST 0x14
#define STK8BA_INTEN1 0x16
#define STK8BA_INTEN2 0x17
#define STK8BA_INTCFG1 0x20
#define STK8BA_INTCFG2 0x21
#define STK8BA_INTFCFG 0x34
#define STK8BA_OFSTX 0x38
#define STK8BA_OFSTY 0x39
#define STK8BA_OFSTZ 0x3A

#define SDA_GPIO	68
#define SCL_GPIO	69
#define INT_GPIO        100

#endif

