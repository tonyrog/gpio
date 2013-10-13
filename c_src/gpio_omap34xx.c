#include <stdlib.h>
#include <errno.h>
#include <sys/mman.h>

#include "gpio_drv.h"
#include "gpio_omap24xx.h"

#define OMAP34XX_GPIO1_BASE		0x48310000
#define OMAP34XX_GPIO2_BASE		0x49050000
#define OMAP34XX_GPIO3_BASE		0x49052000
#define OMAP34XX_GPIO4_BASE		0x49054000
#define OMAP34XX_GPIO5_BASE		0x49056000
#define OMAP34XX_GPIO6_BASE		0x49058000

#define GPIO_BASE1		OMAP34XX_GPIO1_BASE
#define GPIO_LEN1		0x2000

#define GPIO_BASE2		OMAP34XX_GPIO2_BASE
#define GPIO_LEN2		0xA000

typedef struct _gpio_priv_t {
    void* vaddr1;
    void* vaddr2;
    volatile uint32_t* base[6];
} gpio_priv_t;


static gpio_priv_t* init(void);
static void  final(gpio_priv_t* ctx);


static int set_output(gpio_priv_t* ctx, int reg, uint32_t mask, uint32_t value);
static int set_input(gpio_priv_t* ctx, int reg, uint32_t mask);
static uint32_t get_direction(gpio_priv_t* ctx, int reg, uint32_t mask);

static int set_dataout(gpio_priv_t* ctx, int reg, uint32_t mask);
static int clr_dataout(gpio_priv_t* ctx, int reg, uint32_t mask);
static uint32_t get_datain(gpio_priv_t* ctx, int reg, uint32_t mask);


gpio_methods_t gpio_omap34xx_meth = {
    .name     = "omap34xx",
    .max_regs = 6,
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
	ctx->vaddr1 = map_registers(GPIO_BASE1, GPIO_LEN1);
	ctx->vaddr2 = map_registers(GPIO_BASE2, GPIO_LEN2);
	if ((ctx->vaddr1 == MAP_FAILED) || (ctx->vaddr2 == MAP_FAILED))
	    goto error;
	ctx->base[0] = ctx->vaddr1;
	ctx->base[1] = (uint32_t*) (ctx->vaddr2 + 0x0000);
	ctx->base[2] = (uint32_t*) (ctx->vaddr2 + 0x2000);
	ctx->base[3] = (uint32_t*) (ctx->vaddr2 + 0x4000);
	ctx->base[4] = (uint32_t*) (ctx->vaddr2 + 0x6000);
	ctx->base[5] = (uint32_t*) (ctx->vaddr2 + 0x8000);
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
	if (ctx->vaddr1 != MAP_FAILED)
	    unmap_registers(ctx->vaddr1, GPIO_LEN1);
	if (ctx->vaddr2 != MAP_FAILED)
	    unmap_registers(ctx->vaddr2, GPIO_LEN2);
	free(ctx);
    }
}

static int set_output(gpio_priv_t* ctx, int reg, uint32_t mask, uint32_t value)
{
    volatile uint32_t* gpio_reg;
    uint32_t data;
    uint32_t v;
    DEBUGF("set_output: reg %d mask %08x value %08x", reg, mask, value);
    gpio_reg = ctx->base[reg];

    if ((data = mask & value))  *(gpio_reg + OMAP24XX_GPIO_SETDATAOUT) = data;
    if ((data = mask & ~value)) *(gpio_reg + OMAP24XX_GPIO_CLEARDATAOUT) = data;
    gpio_reg += OMAP24XX_GPIO_OE;
    v = *gpio_reg;
    v |= mask;
    *gpio_reg = v;
    return 0;
}

static int set_input(gpio_priv_t* ctx, int reg, uint32_t mask)
{
    volatile uint32_t* r;
    uint32_t v;
    DEBUGF("set_input: reg %d mask %08x", reg, mask);
    r = ctx->base[reg] + OMAP24XX_GPIO_OE;
    v = *r;
    v &= ~mask;
    *r = v;
    return 0;
}

static uint32_t get_direction(gpio_priv_t* ctx, int reg, uint32_t mask)
{
    volatile uint32_t* r;
    uint32_t v;
    DEBUGF("get_direction: reg %d mask %08x", reg, mask);
    r = ctx->base[reg] + OMAP24XX_GPIO_OE;
    v = *r;
    return v & mask;
}

static int set_dataout(gpio_priv_t* ctx, int reg, uint32_t mask)
{
    volatile uint32_t* r;
    DEBUGF("set_dataout: reg %d, mask %08x", reg, mask);
    r = ctx->base[reg] + OMAP24XX_GPIO_SETDATAOUT;
    *r = mask;
    return 0;
}

static int clr_dataout(gpio_priv_t* ctx, int reg, uint32_t mask)
{
    volatile uint32_t* r;
    DEBUGF("clr_dataout: reg %d, mask %08x", reg, mask);
    r = ctx->base[reg] + OMAP24XX_GPIO_CLEARDATAOUT;
    *r = mask;
    return 0;
}

static uint32_t get_datain(gpio_priv_t* ctx, int reg, uint32_t mask)
{
    volatile uint32_t* r;
    uint32_t omask, iv, ov;
    DEBUGF("get_mask: reg %d", reg);
    r = ctx->base[reg];
    omask = *(r + OMAP24XX_GPIO_OE);
    iv = *(r + OMAP24XX_GPIO_DATAIN) & mask;
    ov = *(r + OMAP24XX_GPIO_DATAOUT) & mask;
    return (ov & omask) | (iv & ~omask);
}
