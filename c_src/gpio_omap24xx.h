#ifndef __GPIO_OMAP24XX_H__
#define __GPIO_OMAP24XX_H__

#define OMAP24XX_GPIO_REVISION		(0x0000/4)
#define OMAP24XX_GPIO_SYSCONFIG		(0x0010/4)
#define OMAP24XX_GPIO_SYSSTATUS		(0x0014/4)
#define OMAP24XX_GPIO_IRQSTATUS1	(0x0018/4)
#define OMAP24XX_GPIO_IRQSTATUS2	(0x0028/4)
#define OMAP24XX_GPIO_IRQENABLE2	(0x002c/4)
#define OMAP24XX_GPIO_IRQENABLE1	(0x001c/4)
#define OMAP24XX_GPIO_WAKE_EN		(0x0020/4)
#define OMAP24XX_GPIO_CTRL		(0x0030/4)
#define OMAP24XX_GPIO_OE		(0x0034/4)
#define OMAP24XX_GPIO_DATAIN		(0x0038/4)
#define OMAP24XX_GPIO_DATAOUT		(0x003c/4)
#define OMAP24XX_GPIO_LEVELDETECT0	(0x0040/4)
#define OMAP24XX_GPIO_LEVELDETECT1	(0x0044/4)
#define OMAP24XX_GPIO_RISINGDETECT	(0x0048/4)
#define OMAP24XX_GPIO_FALLINGDETECT	(0x004c/4)
#define OMAP24XX_GPIO_DEBOUNCE_EN	(0x0050/4)
#define OMAP24XX_GPIO_DEBOUNCE_VAL	(0x0054/4)
#define OMAP24XX_GPIO_CLEARIRQENABLE1	(0x0060/4)
#define OMAP24XX_GPIO_SETIRQENABLE1	(0x0064/4)
#define OMAP24XX_GPIO_CLEARWKUENA	(0x0080/4)
#define OMAP24XX_GPIO_SETWKUENA		(0x0084/4)
#define OMAP24XX_GPIO_CLEARDATAOUT	(0x0090/4)
#define OMAP24XX_GPIO_SETDATAOUT	(0x0094/4)

#endif
