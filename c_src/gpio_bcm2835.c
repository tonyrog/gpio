#include <stdlib.h>
#include <errno.h>
#include <sys/mman.h>

#include "gpio_drv.h"

//
// kernel address! pysical address is 0x7E200000
//
#define GPIO_BASE		0x20200000
#define GPIO_LEN		0x100
#define GPIO_FSEL0		0  // r 0-9
#define GPIO_FSEL1		1  // r 10-19
#define GPIO_FSEL2		2  // r 20-29
#define GPIO_FSEL3		3  // r 30-39 
#define GPIO_FSEL4		4  // r 40-49
#define GPIO_FSEL5		5  // r 50-53 (rest is reserved)

#define GPIO_SET		7
#define GPIO_SET1		8
#define GPIO_CLR		10
#define GPIO_CLR1		11
#define GPIO_LEV		13
#define GPIO_LEV1		14
#define GPIO_PULLEN		37
#define GPIO_PULLCLK		38

// GPIO pin Function selection FSEL values
#define GPIO_MODE_IN		0
#define GPIO_MODE_OUT		1

typedef struct {
    void* vaddr;
    volatile uint32_t* base[2];
} gpio_priv_t;

static gpio_priv_t* init(void);
static void  final(gpio_priv_t* ctx);

static int set_output(gpio_priv_t* ctx, int reg, uint32_t mask, uint32_t value);
static int set_input(gpio_priv_t* ctx, int reg, uint32_t mask);
static uint32_t get_direction(gpio_priv_t* ctx, int reg, uint32_t mask);

static int set_dataout(gpio_priv_t* ctx, int reg, uint32_t mask);
static int clr_dataout(gpio_priv_t* ctx, int reg, uint32_t mask);
static uint32_t get_datain(gpio_priv_t* ctx, int reg, uint32_t mask);

gpio_methods_t gpio_bcm2835_meth = {
    .name          = "bcm2835",
    .max_regs      = 2,
    .init          = (init_fn_t) init,
    .final         = (final_fn_t) final,
    .set_output    = (set_output_fn_t) set_output,
    .set_input     = (set_input_fn_t) set_input,
    .get_direction = (get_direction_fn_t) get_direction,
    .set_dataout   = (set_dataout_fn_t) set_dataout,
    .clr_dataout   = (clr_dataout_fn_t) clr_dataout,
    .get_datain    = (get_datain_fn_t)  get_datain
};

static gpio_priv_t* init()
{
    gpio_priv_t* ctx = malloc(sizeof(gpio_priv_t));
    int save_error;

    if (ctx != NULL) {
	if ((ctx->vaddr = map_registers(GPIO_BASE, GPIO_LEN)) == MAP_FAILED)
	    goto error;
	ctx->base[0] = ctx->vaddr;
	ctx->base[1] = (uint32_t*) (ctx->vaddr + 0x4);
    }
    return ctx;
error:
    save_error = errno;
    final(ctx);
    errno = save_error;
    return NULL;
}

static void final(gpio_priv_t* ctx)
{
    if (ctx) {
	if (ctx->vaddr != MAP_FAILED)
	    unmap_registers(ctx->vaddr, GPIO_LEN);
	free(ctx);
    }
}

static int set_output(gpio_priv_t* ctx, int reg, uint32_t mask, uint32_t value)
{
    volatile uint32_t* r;
    uint32_t data;
    int pin = reg*32;
    DEBUGF("set_output: reg %d mask %08x value %08x", reg, mask, value);

    if (reg == 1) // 22 pins in reg=1 (total 54 pins 0..53)
	mask &= 0x3fffff;

    r = ctx->base[reg];
    if ((data = mask & value))  *(r + GPIO_SET) = data;
    if ((data = mask & ~value)) *(r + GPIO_CLR) = data;

    r = ctx->base[0];
    while(mask) {
	if (mask & 1) {
	    int freg = pin / 10;
	    int fbit = (pin % 10)*3;
	    uint32_t fsel;
	    volatile uint32_t* ri;

	    ri = r + (GPIO_FSEL0 + freg);
	    fsel = *ri;
	    fsel = (fsel & ~(7 << fbit)) | (GPIO_MODE_OUT << fbit);
	    *ri = fsel;
	}
	pin++;
	mask >>= 1;
    }
    return 0;
}

static int set_input(gpio_priv_t* ctx, int reg, uint32_t mask)
{
    volatile uint32_t* r;
    int pin = reg*32;
    DEBUGF("set_input: reg %d mask %08x", reg, mask);

    r = ctx->base[0];
    if (reg == 1) // 22 pins in reg=1 (total 54 pins 0..53)
	mask &= 0x3fffff;

    while(mask) {
	if (mask & 1) {
	    int freg = pin / 10;
	    int fbit = (pin % 10)*3;
	    uint32_t fsel;
	    volatile uint32_t* ri;

	    ri = r + (GPIO_FSEL0 + freg);
	    fsel = *ri;
	    fsel = (fsel & ~(7 << fbit)) | (GPIO_MODE_IN << fbit);
	    *ri = fsel;
	}
	pin++;
	mask >>= 1;
    }
    return 0;
}


static uint32_t get_direction(gpio_priv_t* ctx, int reg, uint32_t mask)
{
    volatile uint32_t* r;
    uint32_t value = 0;
    uint32_t bit = 0x000000001;
    int pin = reg*32;
    DEBUGF("get_direction: reg %d mask %08x", reg, mask);
    r = ctx->base[0];

    if (reg == 1) // 22 pins in reg=1 (total 54 pins 0..53)
	mask &= 0x3fffff;
    while(mask) {
	if (mask & 1) {
	    int freg = pin / 10;
	    int fbit = (pin % 10)*3;
	    uint32_t fsel;
	    volatile uint32_t* ri;
	    ri = r + (GPIO_FSEL0 + freg);
	    fsel = *ri;
	    if (((fsel >> fbit) & 7) == GPIO_MODE_OUT)
		value |= bit;
	}
	pin++;
	mask >>= 1;
	bit <<= 1;
    }
    return value;
}

// set the pins in reg to high matching bits in mask
static int set_dataout(gpio_priv_t* ctx, int reg, uint32_t mask)
{
    volatile uint32_t* r;
    DEBUGF("set_dataout: reg %d, mask %08x", reg, mask);
    r = ctx->base[reg];
    if (reg == 1) // 22 pins in reg=1 (total 54 pins 0..53)
	mask &= 0x3fffff;
    r += GPIO_SET;
    *r = mask;
    return 0;
}

// set the pins in reg to low matching bits in mask
static int clr_dataout(gpio_priv_t* ctx, int reg, uint32_t mask)
{
    volatile uint32_t* r;
    DEBUGF("clr_dataout: reg %d, mask %08x", reg, mask);
    r = ctx->base[reg];
    if (reg == 1) // 22 pins in reg=1 (total 54 pins 0..53)
	mask &= 0x3fffff;
    r += GPIO_CLR;
    *r = mask;
    return 0;
}

static uint32_t get_datain(gpio_priv_t* ctx, int reg, uint32_t mask)
{
    volatile uint32_t* r;
    DEBUGF("get_datain: reg %d", reg);
    r = ctx->base[reg];
    if (reg == 1) // 22 pins in reg=1 (total 54 pins 0..53)
	mask &= 0x3fffff;
    r += GPIO_LEV;
    return *r & mask;
}
