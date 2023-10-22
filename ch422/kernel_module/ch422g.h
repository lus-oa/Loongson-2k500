#ifndef	CH422G_H
#define	CH422G_H

/*  设置系统参数命令 */ 

#define CH422_SYS_CMD     0x4800     // 设置系统参数命令，默认方式
#define BIT_X_INT         0x08       // 使能输入电平变化中断，为0禁止输入电平变化中断；为1并且DEC_H为0允许输出电平变化中断
#define BIT_DEC_H         0x04       // 控制开漏输出引脚高8位的片选译码
#define BIT_DEC_L         0x02       // 控制开漏输出引脚低8位的片选译码
#define BIT_IO_OE         0x01       // 控制双向输入输出引脚的三态输出，为1允许输出

/* 设置双向输入输出命令 */

#define CH422_SET_IO_CMD   0x4600    // 设置双向输入输出命令，默认方式
#define BIT_IO0_DAT        0x01      // 写入双向输入输出引脚的输出寄存器，当IO_OE=1,IO0为0输出低电平，为1输出高电平
#define BIT_IO1_DAT        0x02      // 写入双向输入输出引脚的输出寄存器，当IO_OE=1,IO1为0输出低电平，为1输出高电平
#define BIT_IO2_DAT        0x04      // 写入双向输入输出引脚的输出寄存器，当IO_OE=1,IO2为0输出低电平，为1输出高电平
#define BIT_IO3_DAT        0x08      // 写入双向输入输出引脚的输出寄存器，当IO_OE=1,IO3为0输出低电平，为1输出高电平
#define BIT_IO4_DAT        0x10      // 写入双向输入输出引脚的输出寄存器，当IO_OE=1,IO4为0输出低电平，为1输出高电平
#define BIT_IO5_DAT        0x20      // 写入双向输入输出引脚的输出寄存器，当IO_OE=1,IO5为0输出低电平，为1输出高电平
#define BIT_IO6_DAT        0x40      // 写入双向输入输出引脚的输出寄存器，当IO_OE=1,IO6为0输出低电平，为1输出高电平
#define BIT_IO7_DAT        0x80      // 写入双向输入输出引脚的输出寄存器，当IO_OE=1,IO7为0输出低电平，为1输出高电平

/* CH422常用命令码*/
#define CH422_IO_OE_BIT		0x0001	//IO7-IO0双向输入输出引脚，位1允许输出
#define CH422_A_SCAN_BIT	0x0004	//控制动态扫描控制功能，为0启用IO扩展功能、为1数码管动态显示
#define CH422_OD_EN_BIT		0x0010	//OC0-OC3输出使能，0推挽输出，1开漏输出
#define CH422_SLEEP_BIT		0x0080	//低功耗睡眠控制位
#define CH422_GET_KEY		0x4f00	//获取按键,返回按键代码
#define CH422_RD_IO		0x4d00	//输入字节2为双向引脚IO7-IO0的引脚状态

#define CH422_DIG0	0x7000	// 数码管位0显示,需另加8位数据
#define CH422_DIG1	0x7200	// 数码管位1显示,需另加8位数据
#define CH422_DIG2	0x7400	// 数码管位2显示,需另加8位数据
#define CH422_DIG3	0x7600  // 数码管位3显示,需另加8位数据

#endif
