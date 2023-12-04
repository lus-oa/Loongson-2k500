// Copyright (c) 2023 Shandong University
// Copyright (c) 2023 Junchi Ren, Jinrun Yang

# 字符转换点阵

| [单片机-LCD-LED-OLED中文点阵生成](https://www.zhetao.com/fontarray.html)

网页中使用16*16生成字符点阵，但由于生成器和设备所采用的编码方式不同，需要本工具进行转换。需要注意的是，使用网页进行生成时，需要逐字生成，不可一次性输入多个字符

如网页中生成的
```
static const unsigned char bitmap_bytes[] = {
    0x02, 0x00, 
    0x02, 0x00, 
    0x02, 0x00, 
    0x02, 0x00, 
    0x02, 0x00, 
    0x02, 0x00, 
    0x03, 0xf8, 
    0x02, 0x00, 
    0x02, 0x00, 
    0x02, 0x00, 
    0x02, 0x00, 
    0x02, 0x00, 
    0x02, 0x00, 
    0x02, 0x00, 
    0xff, 0xfe, 
    0x00, 0x00
};
```

将其复制到 `generate.c` 的bitmap中，程序运行后会在控制台输出适用于龙芯试验箱点阵的编码数据
```
```