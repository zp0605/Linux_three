#include <types.h>

struct RIFF_HEADER
{
    char szRiffID[4];  // 'R','I','F','F'
    DWORD dwRiffSize;
    char szRiffFormat[4]; // 'W','A','V','E'
};

struct WAVE_FORMAT
{
    WORD wFormatTag;
    WORD wChannels;
    DWORD dwSamplesPerSec;
    DWORD dwAvgBytesPerSec;
    WORD wBlockAlign;
    WORD wBitsPerSample;
};

struct FMT_BLOCK
{
    char  szFmtID[4]; // 'f','m','t',' '
    DWORD  dwFmtSize;
    WAVE_FORMAT wavFormat;
};

struct DATA_BLOCK
{
    char szDataID[4]; // 'd','a','t','a'
    DWORD dwDataSize;
};

/*
 * 事先使用UBOOT把WAV文件烧在NAND的0x60000
 */
void read_wav(unsigned int wav_buf, int *fs, int *channels, int *bits_per_sample, int *wav_size)
{
    struct FMT_BLOCK *fmtblk;
    struct DATA_BLOCK *datblk;
    
    nand_read((unsigned char *)wav_buf, 0x60000, 0x200000);

    fmtblk = (struct FMT_BLOCK *)(wav_buf + sizeof(struct RIFF_HEADER));
    *fs              = fmtblk->wavFormat.dwSamplesPerSec;
    *channels        = fmtblk->wavFormat.wChannels;
    *bits_per_sample = fmtblk->wavFormat.wBitsPerSample;

    datblk = (struct DATA_BLOCK *)fmtblk[1];
    *wav_size = datblk->dwDataSize;
}

