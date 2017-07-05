#include <s3c24xx.h>

extern void delay(void);


/* UDA1341 Register bits */
#define UDA1341_ADDR 0x14

#define UDA1341_REG_DATA0 (UDA1341_ADDR + 0)
#define UDA1341_REG_STATUS (UDA1341_ADDR + 2)

/* status control */
#define STAT0 (0x00)
#define STAT0_RST (1 << 6)
#define STAT0_SC_MASK (3 << 4)
#define STAT0_SC_512FS (0 << 4)
#define STAT0_SC_384FS (1 << 4)
#define STAT0_SC_256FS (2 << 4)
#define STAT0_IF_MASK (7 << 1)
#define STAT0_IF_I2S (0 << 1)
#define STAT0_IF_LSB16 (1 << 1)
#define STAT0_IF_LSB18 (2 << 1)
#define STAT0_IF_LSB20 (3 << 1)
#define STAT0_IF_MSB (4 << 1)
#define STAT0_IF_LSB16MSB (5 << 1)
#define STAT0_IF_LSB18MSB (6 << 1)
#define STAT0_IF_LSB20MSB (7 << 1)
#define STAT0_DC_FILTER (1 << 0)
#define STAT0_DC_NO_FILTER (0 << 0)

#define STAT1 (0x80)
#define STAT1_DAC_GAIN (1 << 6) /* gain of DAC */
#define STAT1_ADC_GAIN (1 << 5) /* gain of ADC */
#define STAT1_ADC_POL (1 << 4) /* polarity of ADC */
#define STAT1_DAC_POL (1 << 3) /* polarity of DAC */
#define STAT1_DBL_SPD (1 << 2) /* double speed playback */
#define STAT1_ADC_ON (1 << 1) /* ADC powered */
#define STAT1_DAC_ON (1 << 0) /* DAC powered */

/* data0 direct control */
#define DATA0 (0x00)
#define DATA0_VOLUME_MASK (0x3f)
#define DATA0_VOLUME(x) (x)

#define DATA1 (0x40)
#define DATA1_BASS(x) ((x) << 2)
#define DATA1_BASS_MASK (15 << 2)
#define DATA1_TREBLE(x) ((x))
#define DATA1_TREBLE_MASK (3)

#define DATA2 (0x80)
#define DATA2_PEAKAFTER (0x1 << 5)
#define DATA2_DEEMP_NONE (0x0 << 3)
#define DATA2_DEEMP_32KHz (0x1 << 3)
#define DATA2_DEEMP_44KHz (0x2 << 3)
#define DATA2_DEEMP_48KHz (0x3 << 3)
#define DATA2_MUTE (0x1 << 2)
#define DATA2_FILTER_FLAT (0x0 << 0)
#define DATA2_FILTER_MIN (0x1 << 0)
#define DATA2_FILTER_MAX (0x3 << 0)
/* data0 extend control */
#define EXTADDR(n) (0xc0 | (n))
#define EXTDATA(d) (0xe0 | (d))

#define EXT0 0
#define EXT0_CH1_GAIN(x) (x)
#define EXT1 1
#define EXT1_CH2_GAIN(x) (x)
#define EXT2 2
#define EXT2_MIC_GAIN_MASK (7 << 2)
#define EXT2_MIC_GAIN(x) ((x) << 2)
#define EXT2_MIXMODE_DOUBLEDIFF (0)
#define EXT2_MIXMODE_CH1 (1)
#define EXT2_MIXMODE_CH2 (2)
#define EXT2_MIXMODE_MIX (3)
#define EXT4 4
#define EXT4_AGC_ENABLE (1 << 4)
#define EXT4_INPUT_GAIN_MASK (3)
#define EXT4_INPUT_GAIN(x) ((x) & 3)
#define EXT5 5
#define EXT5_INPUT_GAIN(x) ((x) >> 2)
#define EXT6 6
#define EXT6_AGC_CONSTANT_MASK (7 << 2)
#define EXT6_AGC_CONSTANT(x) ((x) << 2)
#define EXT6_AGC_LEVEL_MASK (3)
#define EXT6_AGC_LEVEL(x) (x)

#define AUDIO_NAME "UDA1341"
#define AUDIO_NAME_VERBOSE "UDA1341 audio driver"

#define AUDIO_FMT_MASK (AFMT_S16_LE)
#define AUDIO_FMT_DEFAULT (AFMT_S16_LE)

#define AUDIO_CHANNELS_DEFAULT 2
#define AUDIO_RATE_DEFAULT 44100

#define AUDIO_NBFRAGS_DEFAULT 8
#define AUDIO_FRAGSIZE_DEFAULT 8192

#define S_CLOCK_FREQ 384

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


static void uda1341_l3_address(unsigned char data)
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


static void uda1341_l3_data(unsigned char data)
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
	uda1341_l3_data(0x40 | STAT0_SC_384FS | STAT0_DC_FILTER); // reset uda1341
	uda1341_l3_data(STAT1 | STAT1_ADC_ON | STAT1_DAC_ON);

	uda1341_l3_address(UDA1341_REG_DATA0);
	uda1341_l3_data(DATA0 |DATA0_VOLUME(0x0)); // maximum volume
	uda1341_l3_data(DATA1 |DATA1_BASS(0)| DATA1_TREBLE(0));
	uda1341_l3_data((DATA2 |DATA2_DEEMP_NONE) &~(DATA2_MUTE));
	uda1341_l3_data(EXTADDR(EXT2));
}

/*
 * volume : 0~100, 0表示最小音量
 */
void uda1341_set_volume(int volume)
{
    /* uda1341
     * 0~63, 0表示最大音量
     */
    int val = 63 - volume * 63 / 100;
    uda1341_l3_address(UDA1341_REG_DATA0);
    uda1341_l3_data(val);    
}
    
