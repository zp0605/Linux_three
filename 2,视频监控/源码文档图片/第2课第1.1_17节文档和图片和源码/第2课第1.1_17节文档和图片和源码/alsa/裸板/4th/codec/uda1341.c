static void set_mod(int val)
{
    if (val)
    {
        GPBDAT |= (1<<2);
    }
    else
    {
        GPBDAT &= ~(1<<2);
    }
}

static void set_clk(int val)
{
    if (val)
    {
        GPBDAT |= (1<<4);
    }
    else
    {
        GPBDAT &= ~(1<<4);
    }
}

static void set_dat(int val)
{
    if (val)
    {
        GPBDAT |= (1<<3);
    }
    else
    {
        GPBDAT &= ~(1<<3);
    }
}


static void uda1341_l3_address(u8 data)
{
	int i;

    set_mod(0);
    set_clk(1);
	delay();

	for (i = 0; i < 8; i++) {
		if (data & 0x1) {
            set_clk(0);
            set_dat(1);
			delay();
            set_clk(1);
		} else {
            set_clk(0);
            set_dat(0);
			delay();
            set_clk(1);
		}
		data >>= 1;
	}

    set_mod(1);
    set_clk(1);
}


static void uda1341_l3_data(u8 data)
{
	int i;

	for (i = 0; i < 8; i++) {
		if (data & 0x1) {
            set_clk(0);
            set_dat(1);
			delay();
            set_clk(1);
		} else {
            set_clk(0);
            set_dat(0);
			delay();
            set_clk(1);
		}

		data >>= 1;
	}

}

void uda1341_init(void)
{

	/* GPB 4: L3CLOCK */
	/* GPB 3: L3DATA */
	/* GPB 2: L3MODE */


	set_mod(1);
    set_clk(1);

	uda1341_l3_address(UDA1341_REG_STATUS);
	uda1341_l3_data(0x40 | STAT0_SC_384FS | STAT0_IF_MSB|STAT0_DC_FILTER); // reset uda1341
	uda1341_l3_data(STAT1 | STAT1_ADC_ON | STAT1_DAC_ON);

	uda1341_l3_address(UDA1341_REG_DATA0);
	uda1341_l3_data(DATA0 |DATA0_VOLUME(0x0)); // maximum volume
	uda1341_l3_data(DATA1 |DATA1_BASS(0)| DATA1_TREBLE(0));
	uda1341_l3_data((DATA2 |DATA2_DEEMP_NONE) &~(DATA2_MUTE));
	uda1341_l3_data(EXTADDR(EXT2));
}

