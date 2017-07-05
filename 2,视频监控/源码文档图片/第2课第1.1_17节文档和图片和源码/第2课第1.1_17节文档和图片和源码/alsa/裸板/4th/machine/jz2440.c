

/* L3MODE : GPB2
 * L3DATA : GPB3
 * L3CLK  : GPB4
 */

void jz2440_init(void)
{
    GPBCON &= ~((3<<4) | (3<<6) | (3<<8));
    GPBCON |= ((1<<4) | (1<<6) | (1<<8));
    GPBDAT |= ((1<<2) | (1<<3) | (1<<4));
}

