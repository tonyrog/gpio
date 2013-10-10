PRIVDIR := ../priv

all : $(PRIVDIR)/gpio_drv.so

override CFLAGS += -fpic -shared

OBJS = gpio_drv.o gpio_bcm2835.o gpio_omap34xx.o

$(PRIVDIR)/gpio_drv.so : $(OBJS)
	$(CC) -shared -fpic $(LDFLAGS) -o $@ $(OBJS)

lean:
	$(RM) -f $(PRIVDIR)/gpio_drv.so $(OBJS)
