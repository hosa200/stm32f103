/* gpio.c
 * (c) Tom Trebisky  7-2-2017, 9-12-2017
 *
 * This is documented in section 9 of the reference manual.
 */

/* One of the 3 gpios */
struct gpio {
	// volatile unsigned long cr[2];
	volatile unsigned long crl;	/* 0x00 */
	volatile unsigned long crh;	/* 0x04 */
	volatile unsigned long idr;	/* 0x08 - input data (16 bits) */
	volatile unsigned long odr;	/* 0x0C - output data (16 bits) */
	volatile unsigned long bsrr;	/* 0x10 - set/reset register */
	volatile unsigned long brr;	/* 0x14 - reset register */
	volatile unsigned long lock;	/* 0x18 - lock registers (17 bits) */
};

/* We can reset a bit in two places, either via the brr,
 * or the top 16 bits of the bsrr.
 */

#define GPIOA_BASE	((struct gpio *) 0x40010800)
#define GPIOB_BASE	((struct gpio *) 0x40010C00)
#define GPIOC_BASE	((struct gpio *) 0x40011000)

#define MODE_IN		0x00	/* Input */
#define MODE_OUT_10	0x01	/* Output, 10 Mhz */
#define MODE_OUT_2	0x02	/* Output, 2 Mhz */
#define MODE_OUT_50	0x03	/* Output, 50 Mhz */

/* Output configurations */
#define CONF_GP_PP	0x0	/* GPIO - Pull up/down */
#define CONF_GP_OD	0x4	/* GPIO - Open drain */
#define CONF_ALT_PP	0x8	/* Alternate function - Pull up/down */
#define CONF_ALT_OD	0xC	/* Alternate function - Open drain */

/* Input configurations */
#define CONF_IN_ANALOG	0x0	/* Analog input */
#define CONF_IN_FLOAT	0x4	/* normal (as reset) */
#define CONF_IN_PP	0x8	/* with push/pull */

/* Change these two to select the output pin/port */
#ifdef notdef
#define BLINK_GPIO	GPIOC_BASE
#define BLINK_BIT	13
#define BLINK_GPIO	GPIOB_BASE
#define BLINK_BIT	0
#endif

#define BLINK_GPIO	GPIOA_BASE
#define BLINK_BIT	0

//#define BLINK_CONF	(MODE_OUT_50 | CONF_GP_OD)
#define BLINK_CONF	(MODE_OUT_50 | CONF_GP_PP)

struct gpio *gp;
unsigned long on_mask;
unsigned long off_mask;

static void
bit_output ( struct gpio *gp, int bit )
{
	int conf;
	int shift;

	if ( bit < 8 ) {
	    shift = bit * 4;
	    conf = gp->crl & ~(0xf<<shift);
	    gp->crl = conf | BLINK_CONF << shift;
	} else {
	    shift = (bit - 8) * 4;
	    conf = gp->crh & ~(0xf<<shift);
	    gp->crh = conf | BLINK_CONF << shift;
	}
}

static void
output_all ( struct gpio *gp )
{
	int i;

	for ( i=0; i<16; i++ )
	    bit_output ( gp, i );
}

void
led_init ( void )
{
	int bit = BLINK_BIT;

	gp = BLINK_GPIO;

	bit_output ( gp, bit );

	off_mask = 1 << bit;
	on_mask = 1 << (bit+16);
}

/* reset turns the LED on (pulls the port low) */
void
led_on ( void )
{
	gp->bsrr = on_mask;
}

/* set turns the LED off (pulls the port high) */
void
led_off ( void )
{
	gp->bsrr = off_mask;
}

/* This yields a 1 Mhz waveform */
/* For the heck of it, reset the bit using the brr */
void
led_fast ( void )
{
	for ( ;; ) {
	    // gp->bsrr = on_mask;
	    // gp->bsrr = on_mask;
	    gp->brr = off_mask;
	    gp->brr = off_mask;
	    gp->bsrr = off_mask;
	}
}

/* I used this little test and an oscilloscope to find out what pins
 * would yield a waveform without mischief.
 * 
 * A0 to A12 are OK (A13 to A15 do not exist)
 * B0 to B1 are OK
 * B2 is "BOOT1", so forget using it.
 * B3 and B4 are weird
 * B5 to B15 are fine
 * C13 is fine (but is the onboard LED)
 * C14 and C15 are weird (they are the crystal, so forget it).
 */
void
led_test ( void )
{
	struct gpio *gpa = GPIOA_BASE;
	struct gpio *gpb = GPIOB_BASE;
	struct gpio *gpc = GPIOC_BASE;

	output_all ( gpa );
	output_all ( gpb );
	bit_output ( gpc, 13 );

	for ( ;; ) {
	    gpa->brr = 0xffff;
	    gpb->brr = 0xffff;
	    gpc->brr = 0xffff;
	    gpa->bsrr = 0xffff;
	    gpb->bsrr = 0xffff;
	    gpc->bsrr = 0xffff;
	}
}

/* THE END */
