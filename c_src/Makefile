#@BEGIN-C_SRC-DEFAULT-RULES@
ERL=$(shell readlink -f `which erl`)
ERL_TOP=$(ERL:%/bin/erl=%)
OS=$(shell uname -s)
CFLAGS=-MMD -MF .$<.d -I$(ERL_TOP)/usr/include
#@END-C_SRC-DEFAULT-RULES@

PRIVDIR := ../priv

all : $(PRIVDIR)/gpio_drv.so

override CFLAGS += -fpic -shared

OBJS = gpio_drv.o gpio_bcm2835.o gpio_omap34xx.o

$(PRIVDIR)/gpio_drv.so : $(OBJS)
	$(CC) -shared -fpic $(LDFLAGS) -o $@ $(OBJS)

lean:
	$(RM) -f $(PRIVDIR)/gpio_drv.so $(OBJS)

%.o:	%.c
	$(CC) $(CFLAGS) -c -fPIC $<

-include ".*.d"
