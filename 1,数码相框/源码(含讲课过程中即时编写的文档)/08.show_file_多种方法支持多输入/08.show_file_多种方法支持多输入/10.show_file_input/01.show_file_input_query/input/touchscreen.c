#include <config.h>
#include <input_manager.h>
#include <stdlib.h>

#include <tslib.h>

/* 参考tslib里的ts_print.c */

static struct tsdev *g_tTSDev;
static int giXres;
static int giYres;

/* 注意: 由于要用到LCD的分辨率, 此函数要在SelectAndInitDisplay之后调用 */
static int TouchScreenDevInit(void)
{
	char *pcTSName = NULL;

	if ((pcTSName = getenv("TSLIB_TSDEVICE")) != NULL ) 
	{
		g_tTSDev = ts_open(pcTSName, 1);
	}
	else
	{
		g_tTSDev = ts_open("/dev/event0", 1);
	}

	if (!g_tTSDev) {
		DBG_PRINTF("ts_open error!\n");
		return -1;
	}

	if (ts_config(g_tTSDev)) {
		DBG_PRINTF("ts_config error!\n");
		return -1;
	}

	if (GetDispResolution(&giXres, &giYres))
	{
		return -1;
	}

	return 0;
}

static int TouchScreenDevExit(void)
{
	return 0;
}


static int isOutOf500ms(struct timeval *ptPreTime, struct timeval *ptNowTime)
{
	int iPreMs;
	int iNowMs;
	
	iPreMs = ptPreTime->tv_sec * 1000 + ptPreTime->tv_usec / 1000;
	iNowMs = ptNowTime->tv_sec * 1000 + ptNowTime->tv_usec / 1000;

	return (iNowMs > iPreMs + 500);
}

static int TouchScreenGetInputEvent(PT_InputEvent ptInputEvent)
{
	struct ts_sample tSamp;
	int iRet;

	static struct timeval tPreTime;
	
	iRet = ts_read(g_tTSDev, &tSamp, 1);
	
	if (iRet < 0) {
		return -1;
	}

	/* 处理数据 */
	if (isOutOf500ms(&tPreTime, &tSamp.tv))
	{
		/* 如果此次触摸事件发生的时间, 距上次事件超过了500ms */
		tPreTime = tSamp.tv;
		ptInputEvent->tTime = tSamp.tv;
		ptInputEvent->iType = INPUT_TYPE_TOUCHSCREEN;

		if (tSamp.y < giYres/3)
		{
			ptInputEvent->iVal = INPUT_VALUE_UP;
		}
		else if (tSamp.y > 2*giYres/3)
		{
			ptInputEvent->iVal = INPUT_VALUE_DOWN;
		}
		else
		{
			ptInputEvent->iVal = INPUT_VALUE_UNKNOWN;
		}
		return 0;
		
	}
	else
	{
		return -1;
	}
	

	return 0;
}


static T_InputOpr g_tTouchScreenOpr = {
	.name          = "touchscreen",
	.DeviceInit    = TouchScreenDevInit,
	.DeviceExit    = TouchScreenDevExit,
	.GetInputEvent = TouchScreenGetInputEvent,
};

int TouchScreenInit(void)
{
	return RegisterInputOpr(&g_tTouchScreenOpr);
}

