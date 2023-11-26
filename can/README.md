## 开启can0接口

```bash
ip link set can0 type can bitrate 500000 triple-sampling on
ifconfig can0 up
```

## 向can0发送数据
```bash
cansend can0 125#02.03.04.05.06.06.77.88
```

## 从can0中接收数据
```bash
candump can0 -L
```