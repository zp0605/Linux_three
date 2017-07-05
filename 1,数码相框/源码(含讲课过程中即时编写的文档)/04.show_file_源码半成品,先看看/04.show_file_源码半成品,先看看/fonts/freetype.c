
#include <config.h>
#include <fonts_manager.h>
#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_GLYPH_H

static int FreeTypeFontInit(char *pcFontFile, unsigned int dwFontSize);
static int FreeTypeGetFontBitmap(unsigned int code, int iPenX, int iPenY, PT_FontBitMap ptFontBitMap);

static T_FontOpr g_tFreeTypeFontOpr = {
	.name          = "freetype",
	.FontInit      = FreeTypeFontInit,
	.GetFontBitmap = FreeTypeGetFontBitmap,
};

static FT_Library g_tLibrary;
static FT_Face g_tFace;
static FT_GlyphSlot g_tSlot;

static int FreeTypeFontInit(char *pcFontFile, unsigned int dwFontSize)
{
	int iError;

	/* ��ʾʸ������ */
	iError = FT_Init_FreeType(&g_tLibrary );			   /* initialize library */
	/* error handling omitted */
	if (iError)
	{
		DBG_PRINTF("FT_Init_FreeType failed\n");
		return -1;
	}
	
	iError = FT_New_Face(g_tLibrary, pcFontFile, 0, &g_tFace); /* create face object */
	/* error handling omitted */
	if (iError)
	{
		DBG_PRINTF("FT_Init_FreeType failed\n");		
		return -1;
	}
	
	g_tSlot = g_tFace->glyph;

	iError = FT_Set_Pixel_Sizes(g_tFace, dwFontSize, 0);
	if (iError)
	{
		DBG_PRINTF("FT_Set_Pixel_Sizes failed : %d\n", dwFontSize);
		return -1;
	}
	
	
	return 0;
}

static int FreeTypeGetFontBitmap(unsigned int dwCode, int iPenX, int iPenY, PT_FontBitMap ptFontBitMap)
{
	int iError;
#if 0
	FT_Vector tPen;

	tPen.x = 0;
	tPen.y = 0;
	
	/* set transformation */
	FT_Set_Transform(g_tFace, 0, &tPen);
#endif

	/* load glyph image into the slot (erase previous one) */
	iError = FT_Load_Char(g_tFace, dwCode, FT_LOAD_RENDER );
	if (iError)
	{
		DBG_PRINTF("FT_Load_Char error for code : 0x%x\n", dwCode);
		return -1;
	}

	ptFontBitMap->iXLeft    = iPenX + g_tSlot->bitmap_left;
	ptFontBitMap->iYTop     = iPenY - g_tSlot->bitmap_top;
	ptFontBitMap->iXMax     = ptFontBitMap->iXLeft + g_tSlot->bitmap.width;
	ptFontBitMap->iYMax     = ptFontBitMap->iYTop  + g_tSlot->bitmap.rows;
	ptFontBitMap->iBpp      = 8;
	ptFontBitMap->pucBuffer = g_tSlot->bitmap.buffer;

	return 0;
}

int FreeTypeInit(void)
{
	return RegisterFontOpr(&g_tFreeTypeFontOpr);
}

