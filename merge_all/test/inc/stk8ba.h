#ifndef __STK8BA_H__
#define __STK8BA_H__

#define STK8BA_ORI_SIZE		6
#define STK8BA_XYZ_SIZE		3

extern int read_stk8ba_xyz(int *databuf);
extern int read_stk8ba_origin(unsigned char *databuf);
extern int open_stk8ba(void);
extern int close_stk8ba(void);

#endif

