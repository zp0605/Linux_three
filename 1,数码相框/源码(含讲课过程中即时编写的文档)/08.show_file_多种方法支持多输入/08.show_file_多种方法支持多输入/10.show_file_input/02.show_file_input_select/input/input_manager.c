
#include <config.h>
#include <input_manager.h>
#include <string.h>
#include <sys/select.h>

static PT_InputOpr g_ptInputOprHead;
static fd_set g_tRFds;
static int g_iMaxFd = -1;

int RegisterInputOpr(PT_InputOpr ptInputOpr)
{
	PT_InputOpr ptTmp;

	if (!g_ptInputOprHead)
	{
		g_ptInputOprHead   = ptInputOpr;
		ptInputOpr->ptNext = NULL;
	}
	else
	{
		ptTmp = g_ptInputOprHead;
		while (ptTmp->ptNext)
		{
			ptTmp = ptTmp->ptNext;
		}
		ptTmp->ptNext	  = ptInputOpr;
		ptInputOpr->ptNext = NULL;
	}

	return 0;
}


void ShowInputOpr(void)
{
	int i = 0;
	PT_InputOpr ptTmp = g_ptInputOprHead;

	while (ptTmp)
	{
		printf("%02d %s\n", i++, ptTmp->name);
		ptTmp = ptTmp->ptNext;
	}
}

int AllInputDevicesInit(void)
{
	PT_InputOpr ptTmp = g_ptInputOprHead;
	int iError = -1;

	FD_ZERO(&g_tRFds);

	while (ptTmp)
	{
		if (0 == ptTmp->DeviceInit())
		{
			FD_SET(ptTmp->iFd, &g_tRFds);
			if (g_iMaxFd < ptTmp->iFd)
				g_iMaxFd = ptTmp->iFd;
			iError = 0;
		}
		ptTmp = ptTmp->ptNext;
	}

	g_iMaxFd++;
	return iError;
}

int GetInputEvent(PT_InputEvent ptInputEvent)
{

	/* 用select函数监测stdin,touchscreen,
	   有数据时再调用它们的GetInputEvent或获得具体事件
	 */

	PT_InputOpr ptTmp = g_ptInputOprHead;
	fd_set tRFds;
	int iRet;

	tRFds = g_tRFds;

	iRet = select(g_iMaxFd, &tRFds, NULL, NULL, NULL);
	if (iRet > 0)
	{
		while (ptTmp)
		{
			if (FD_ISSET(ptTmp->iFd, &tRFds))
			{
				if(0 == ptTmp->GetInputEvent(ptInputEvent))
				{
					return 0;
				}
			}
			ptTmp = ptTmp->ptNext;
		}
	}

	return -1;
}

int InputInit(void)
{
	int iError;
	iError = StdinInit();
	iError |= TouchScreenInit();
	return iError;
}

