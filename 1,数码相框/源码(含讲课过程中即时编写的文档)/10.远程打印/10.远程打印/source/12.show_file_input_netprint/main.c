#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <config.h>
#include <draw.h>
#include <encoding_manager.h>
#include <fonts_manager.h>
#include <disp_manager.h>
#include <input_manager.h>
#include <string.h>


/* ./show_file [-s Size] [-f freetype_font_file] [-h HZK] <text_file> */
int main(int argc, char **argv)
{
	int iError;
	unsigned int dwFontSize = 16;
	char acHzkFile[128];
	char acFreetypeFile[128];
	char acTextFile[128];

	char acDisplay[128];

	int bList = 0;

	T_InputEvent tInputEvent;

	acHzkFile[0]  = '\0';
	acFreetypeFile[0] = '\0';
	acTextFile[0] = '\0';

	strcpy(acDisplay, "fb");

	iError = DebugInit();
	if (iError)
	{
		DBG_PRINTF("DebugInit error!\n");
		return -1;
	}

	InitDebugChanel();
	
	while ((iError = getopt(argc, argv, "ls:f:h:d:")) != -1)
	{
		switch(iError)
		{
			case 'l':
			{
				  bList = 1;
				  break;
			}
			case 's':
			{
				  dwFontSize = strtoul(optarg, NULL, 0);
				  break;
			}
			case 'f':
			{
				  strncpy(acFreetypeFile, optarg, 128);
				  acFreetypeFile[127] = '\0';
				  break;
			}			
			case 'h':
			{
					strncpy(acHzkFile, optarg, 128);
					acHzkFile[127] = '\0';
					break;
			}
			case 'd':
			{
				strncpy(acDisplay, optarg, 128);
				acDisplay[127] = '\0';
				break;
			}
			default:
			{
					DBG_PRINTF("Usage: %s [-s Size] [-d display] [-f font_file] [-h HZK] <text_file>\n", argv[0]);
					DBG_PRINTF("Usage: %s -l\n", argv[0]);
					return -1;
					break;
			}
		}
	}

	if (!bList && (optind >= argc))
	{
		DBG_PRINTF("Usage: %s [-s Size] [-d display] [-f font_file] [-h HZK] <text_file>\n", argv[0]);
		DBG_PRINTF("Usage: %s -l\n", argv[0]);
		return -1;
	}
		
	iError = DisplayInit();
	if (iError)
	{
		DBG_PRINTF("DisplayInit error!\n");
		return -1;
	}

	iError = FontsInit();
	if (iError)
	{
		DBG_PRINTF("FontsInit error!\n");
		return -1;
	}

	iError = EncodingInit();
	if (iError)
	{
		DBG_PRINTF("EncodingInit error!\n");
		return -1;
	}


	iError = InputInit();
	if (iError)
	{
		DBG_PRINTF("InputInit error!\n");
		return -1;
	}



	if (bList)
	{
		DBG_PRINTF("supported display:\n");
		ShowDispOpr();

		DBG_PRINTF("supported font:\n");
		ShowFontOpr();

		DBG_PRINTF("supported encoding:\n");
		ShowEncodingOpr();

		DBG_PRINTF("supported input:\n");
		ShowInputOpr();

		DBG_PRINTF("supported debug chanel:\n");
		ShowDebugOpr();
		
		return 0;
	}

	strncpy(acTextFile, argv[optind], 128);
	acTextFile[127] = '\0';
		
	iError = OpenTextFile(acTextFile);
	if (iError)
	{
		DBG_PRINTF("OpenTextFile error!\n");
		return -1;
	}

	iError = SetTextDetail(acHzkFile, acFreetypeFile, dwFontSize);
	if (iError)
	{
		DBG_PRINTF("SetTextDetail error!\n");
		return -1;
	}

	DBG_PRINTF("%s %s %d\n", __FILE__, __FUNCTION__, __LINE__);

	iError = SelectAndInitDisplay(acDisplay);
	if (iError)
	{
		DBG_PRINTF("SelectAndInitDisplay error!\n");
		return -1;
	}

	iError = AllInputDevicesInit();
	if (iError)
	{
		DBG_PRINTF("Error AllInputDevicesInit\n");
		return -1;
	}
	
	DBG_PRINTF("%s %s %d\n", __FILE__, __FUNCTION__, __LINE__);
	iError = ShowNextPage();
	DBG_PRINTF("%s %s %d\n", __FILE__, __FUNCTION__, __LINE__);
	if (iError)
	{
		DBG_PRINTF("Error to show first page\n");
		return -1;
	}

	DBG_PRINTF("Enter 'n' to show next page, 'u' to show previous page, 'q' to exit: ");

	while (1)
	{

		if (0 == GetInputEvent(&tInputEvent))
		{
			if (tInputEvent.iVal == INPUT_VALUE_DOWN)
			{
				ShowNextPage();
			}
			else if (tInputEvent.iVal == INPUT_VALUE_UP)
			{
				ShowPrePage();			
			}
			else if (tInputEvent.iVal == INPUT_VALUE_EXIT)
			{
				return 0;
			}
		}
	}
	return 0;
}

