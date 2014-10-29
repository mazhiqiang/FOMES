/*Modified by mzq in 20141019*/
#include "stdafx.h"
#include "CBufMS.h"
//#include "..\SMTLogManage\SMTLogManage\SMTLogManage.h" 
//#include "TypeConvert.h"

#include "mmsystem.h" 
#pragma comment(lib, "winmm.lib")

#define ModifiedByMzq
#define SHUTOFF_CHECKCODE
#define PCIDEBUG_
typedef struct
{
	short cmdID;
	short sSpeedX;
	short sSpeedY;
	short errNum;
	long ulPositionX;
	long ulPositionY;	
}CmdForTest;
CmdForTest cmdNO1;
ULONG countNUM = 0;
/*	
	FunctionName：			fnInit
	FunctionModifiedTime：	20141019
	FunctionPurpose：		initial PCI device
*/
errorCode CSingleton::fnInit()
{
	if (!m_bIsInited)
	{
		m_bIsInited = true;
		if(PCIPro.fnPciInit())
		{
			return (PCIPro.fnPciStartThread())?err_Success:err_PCI_Intterupt_Invalid;
		}
	}
	return err_Success;
}
/*	
	FunctionName：			fnSet(Get)CWnd
	FunctionModifiedTime：	20141019
	FunctionPurpose：		set the handle to winform
*/
void CSingleton::fnSetCWnd(CWnd* cwnd)
{
	m_pCWnd = cwnd;
}
CWnd* CSingleton::fnGetCWnd()
{
	return m_pCWnd;
}
/*	
	FunctionName：			fnBuffRoute
	FunctionModifiedTime：	20141019
	FunctionPurpose：		route buff from processing to memory
*/
errorCode CSingleton::fnBuffRoute(unsigned char* m_ControlComd,int len,int* ComdID)
{
	if (len<0)
	{
		return err_MS_Memory_Route_Invalid;
	}
	int uicount = 0;
	unsigned char* buf = new unsigned char[len];
	if(buf == NULL)return err_MS_Memory_Route_Invalid;
	for (uicount = 0;uicount < len;uicount++)
	{
		*(buf+uicount) = *(m_ControlComd+uicount);
	}
	m_sCurMaster.pucData = buf;
	m_sCurMaster.iCmd = m_uicmdid;
	m_sCurMaster.iLength = len;
	while(RAMWrite == StatusMaster) ;
	if(RAMWrite != StatusMaster)
	{
		StatusMaster = InterfaceWrite;
		if(UNADVANCE == m_bAdvance)
			m_vBufMaster.push_back(m_sCurMaster);
		else if(ADVANCE == m_bAdvance)
			m_vBufAdvance.push_back(m_sCurMaster);
		*ComdID = m_uicmdid;
		m_uicmdid++;
		StatusMaster = Idle;	
		fnDelay();
		return err_Success;
	}else
	{
		DELETE_POINT(buf);
		return err_MS_Push_Invalid;
	}
}
/*	
	FunctionName：			fnRead
	FunctionModifiedTime：	20141019
	FunctionPurpose：		read memory to get buffer
*/
errorCode CSingleton::fnRead(void)
{
	unsigned char* pucRemark = new unsigned char[REMARK_LENGTH];
	BufData updateData;
	int icount;
	if(PCIPro.fnPciReadMem(BASE_ADDRESS + UPDATE_REMARK_ADDRESS_OFFSET,REMARK_LENGTH,pucRemark))
	{
		updateData.iCmd =((DataRemark *)pucRemark)->ulCmdid;
		updateData.iLength = ((DataRemark *)pucRemark)->usLength;
		updateData.iCheckCode =(((DataRemark *)pucRemark)->usCheckCode)&0xFF;
	}else 
	{
		DELETE_POINT(pucRemark);
		return err_PCI_Read_Memory_Invalid;
	}
	unsigned char* pucData = new unsigned char[updateData.iLength];
	unsigned char ucVeryTimes = 0;
	do{
		if(PCIPro.fnPciReadMem(BASE_ADDRESS + UPDATE_DATA_ADDRESS_OFFSET,updateData.iLength,pucData))
		{
			int icheckword = 0;
			for (icount = 0;icount <updateData.iLength;icount++)
			{
				icheckword += *(pucData + icount);
			}
			if(true&&((icheckword&LOW8_MASK) != updateData.iCheckCode))
			{
				//MessageBox(LINK.m_pCWnd->m_hWnd,0,0,0);
				ucVeryTimes++;
				if(ucVeryTimes >= READ_TRY)
				{
					DELETE_POINT(pucRemark);
					DELETE_POINT(pucData);
					m_bSynLock = WRITE;
					return err_MS_Check_Code_Invalid;
				}			
			}
			else
			{
				ucVeryTimes = READ_TRY;
				updateData.pucData = pucData;
				while(InterfaceRead == StatusSlave) ;
				if(InterfaceRead != StatusSlave)
				{
					StatusSlave = RAMRead;
					if (m_vBufSlave.size()>BUF_SLAVE_SIZE)
					{
						fnBuffPop(m_vBufSlave[0].iCmd,&m_vBufSlave);
					}
					m_vBufSlave.push_back(updateData);
					StatusSlave = Idle;
				}else 
					return err_PCI_Read_Memory_Invalid;
			}
		}else  
		{
			DELETE_POINT(pucRemark);
			DELETE_POINT(pucData);
			m_ucReadRetry++;
			return	err_PCI_Read_Memory_Invalid;
		}
	}while(ucVeryTimes < READ_TRY);
	

	DELETE_POINT(pucRemark);
	while(InterfaceWrite == StatusMaster) ;
	if(InterfaceWrite != StatusMaster)
	{
		StatusMaster = RAMWrite;
		if(!(err_Success==fnBuffPop(updateData.iCmd,&m_vBufMaster)))
		{
			StatusMaster = Idle;
			m_ucReadRetry++;
			return err_MS_Memory_Free_Invalid;
		}
		StatusMaster = Idle;
	}
	else
	{
		return err_MS_Memory_Route_Invalid;
	}
	m_ucInvalidRetry = 0;
	m_ucErrorRetry = 0;
	m_ucReadRetry = 0;
	m_bSynLock = WRITE;
	return err_Success;
}
/*	
	FunctionName：			fnWrite
	FunctionModifiedTime：	20141019
	FunctionPurpose：		write memory to buffer
*/
errorCode CSingleton::fnWrite(void)
{
	while(InterfaceWrite == StatusMaster) ;
	if(InterfaceWrite != StatusMaster)
	{

		StatusMaster = RAMWrite;
		if (0 != m_vBufMaster.size()||0 != m_vBufAdvance.size())
		{	
			if(0 != m_vBufMaster.size()&&err_Success==fnHardProc(&m_vBufMaster[BUFFER_ZERO])) 
			{
				StatusMaster = Idle;
				m_bSynLock = READ;
				m_ucInvalidRetry = 0;
				m_ucErrorRetry = 0;
#if LogComd
				string Logcontent = "PCI Write {ComdID =" ;
				Logcontent += TypeConvert <short, string>(m_vBufMaster[BUFFER_ZERO].iCmd);
				Logcontent += "}" ;
				Logcontent += "{Data =" ;
				int itmpi;
				for(itmpi = 0;itmpi<m_vBufMaster[BUFFER_ZERO].iLength/2;itmpi++)
				{
					Logcontent += TypeConvert <short, string>(*((short*)m_vBufMaster[BUFFER_ZERO].pucData + itmpi));
					Logcontent += "，";
				}
				Logcontent += "}";
				LogManage& CurLog=LogManage::GetInit();//获取日志类实例，直到程序结束前手动析构掉
				WriteLogMessage(Logcontent ,SMT_WARN,LOG_FDINFO);
#endif
				return err_Success;
			}else
			{					
				StatusMaster = Idle;
				m_ucInvalidRetry++;
				return err_MS_Memory_Trans_Invalid;
			}					
		}else 
		{
			StatusMaster = Idle;
			return err_PCI_Write_Memory_Invalid;
		}
	}
	else
	{
		return err_PCI_Write_Memory_Invalid;
	}
	return err_Success;
}
void fnFakeCMD()
{
#ifdef PCIDEBUG
	if (0 == m_vBufMaster.size())
	{
		countNUM++;
		if (countNUM%6<3)
		{
			cmdNO1.cmdID	= 0x0FA6;
			cmdNO1.sSpeedX	= 0x3AAA;
			cmdNO1.sSpeedY	= 0x3AAA;
			cmdNO1.ulPositionX	= 0x000186A0;
			cmdNO1.ulPositionY	= 0x000186A0;
			cmdNO1.errNum	= 0x0000;
		}else
		{
			cmdNO1.cmdID	= 0x0FA6;
			cmdNO1.sSpeedX	= 0x3AAA;
			cmdNO1.sSpeedY	= 0x3AAA;
			cmdNO1.ulPositionX	= 0x00000000;
			cmdNO1.ulPositionY	= 0x00000000;
			cmdNO1.errNum	= 0x0000;
		}

		int x;
		fnSendToBuffer((BYTE*)(&cmdNO1),sizeof(CmdForTest),&x);
	}
#endif
}
/*	
	FunctionName：			fnBuffTrans
	FunctionModifiedTime：	20141019
	FunctionPurpose：		send or get cmd in turn 
*/
errorCode CSingleton::fnBuffTrans()
{
#if 0	
	if (0)
	{
		return err_MS_Memory_Trans_Invalid;
	}
	if (m_ucInvalidRetry >= UPDATE_ERROR_RETRY 
		|| m_ucErrorRetry >= UPDATE_INVALID_RETRY||m_ucReadRetry >= UPDATE_ERROR_RETRY)
	{

		//这里捅上去,sendmessage
		//目前出错的解决办法是清空内存，这样的错误来自硬件
		if(err_Success == fnFreeAllMemoryAndData())
		{
			m_bSynLock = WRITE;//写状态
			//m_bAsyClock = false;//解锁
			m_ucInvalidRetry = 0;
			m_ucErrorRetry = 0;
			m_ucReadRetry = 0;
		}
		if (m_ucInvalidRetry >= UPDATE_ERROR_RETRY )//校验码出错
		{
		}else if (m_ucErrorRetry >= UPDATE_INVALID_RETRY)//指令无意义
		{
		}else if (m_ucReadRetry >= UPDATE_ERROR_RETRY)//PCI通道有错
		{
		}

		return err_MS_IS_ABORT;
	}
#endif
	ULONG ulFeedbackData;
	ULONG ulFeedbackInterruptFlag;
	ULONG ulFeedbackWriteStatus;
	fnFakeCMD();
	/*-----------------------
  -->| CPLD Register for INT |
	  -----------------------*/
	if(PCIPro.fnPciReadMem(BASE_ADDRESS + UPDATE_INTERRUPT_FLAG_ADDRESS_OFFSET,ulFeedbackInterruptFlag))
	{//Check Interrupt Status
		if(UPDATE_IS_RETURNED == (ulFeedbackInterruptFlag&LOW16_MASK))
		{//Interrupt Arrived
	/*---------
  -->| RAM INT |
      ---------*/
			if(PCIPro.fnPciReadMem(BASE_ADDRESS + UPDATE_BACK_ADDRESS_OFFSET,ulFeedbackData))
			{//Check Status
				switch (ulFeedbackData&LOW16_MASK)
				{
				case UPDATE_FEEDBACK_INFO_IS_READY:
					{//Read
						switch(fnRead())
						{
						case err_Success:
							break;
						case err_MS_Check_Code_Invalid:
							{
								int ickeckcount = 0;
								do{
									if(err_Success == fnRead())
										break;
									else
										ickeckcount++;
								}while(ickeckcount < READ_TRY);
							}
							break;
						case err_MS_Memory_Route_Invalid:
							{
								int iroutecount = 0;
								do{
									if(err_Success == fnRead())
										break;
									else
										iroutecount++;
								}while(iroutecount < READ_TRY);
							}
								break;
							break;
						case err_MS_Memory_Free_Invalid:
							//Send Error Message to Windows
							break;
						case err_PCI_Read_Memory_Invalid:
							//Send Error Message to Windows
							break;
						default :
							break;
						}
						m_ucReadRetry = REWRITE_ZERO;
					}
					break;
				case UPDATE_CHECK_WORD_IS_INVALID:
					{
						m_ucErrorRetry++;
						m_ucReadRetry = REWRITE_ZERO;
						if(READ_TRY < m_ucErrorRetry) ;
						//Send Error Message to Windows
					}
					break;
				case UPDATE_CMD_IS_INVALID:
					{
						m_ucInvalidRetry++;
						m_ucReadRetry = REWRITE_ZERO;
						if(READ_TRY < m_ucInvalidRetry) ;
						//Send Error Message to Windows
					}
					break;
				default:
					break;
				}
			}
			if(PCIPro.fnPciReadMem(BASE_ADDRESS + UPDATE_WRITE_IDLE_ADDRESS_OFFSET,ulFeedbackWriteStatus))
			{
				if(UPDATE_WRITE_IS_IDLE == ulFeedbackWriteStatus)
				{
					do{
						fnWrite(); 
						m_vBufMaster;
					}while(WRITE == m_bSynLock);
				}
			}
		}
		else
		{//Over Time
			m_ucReadRetry++;
			if(m_ucReadRetry > REWRITE_TIME)
			{           
				m_bSynLock = WRITE;
				m_ucReadRetry = REWRITE_ZERO;
				return err_Success;
			}
		}
	}
	if(WRITE == m_bSynLock)fnWrite(); 
	return err_Success;
}
/*	
	FunctionName：			fnHardProc
	FunctionModifiedTime：	20141019
	FunctionPurpose：		write cmd buffer to memory 
*/
errorCode CSingleton::fnHardProc(sBufData* bd)
{
	int uicount = 0;
	unsigned short icheckword = 0;
	for (uicount = 0;uicount < bd->iLength;uicount++)
	{
		icheckword += *(bd->pucData+uicount);
	}
	icheckword &= LOW8_MASK;
	bd->iCheckCode = icheckword;

	DataRemark tmpData;
	tmpData.ulCmdid = bd->iCmd;
	tmpData.usLength = bd->iLength;
	tmpData.usCheckCode = icheckword;

	if(bd->iLength > MAX_DOWNLOAD_LENGTH)
	{	
		int itimes = bd->iLength/MAX_DOWNLOAD_LENGTH;
		int iremainder = bd->iLength%MAX_DOWNLOAD_LENGTH;
		int i;
		for (i = 0;i<itimes;i++)
		{
			if(!PCIPro.fnPciWriteMem(BASE_ADDRESS + DOWNLOAD_DATA_ADDRESS_OFFSET + i*MAX_DOWNLOAD_LENGTH,bd->pucData + i*MAX_DOWNLOAD_LENGTH,MAX_DOWNLOAD_LENGTH))
			{
				return err_PCI_Write_Memory_Invalid;
			}
		}
		if(!PCIPro.fnPciWriteMem(BASE_ADDRESS + DOWNLOAD_DATA_ADDRESS_OFFSET + itimes*MAX_DOWNLOAD_LENGTH + iremainder,bd->pucData + itimes*MAX_DOWNLOAD_LENGTH,iremainder))
		{
			return err_PCI_Write_Memory_Invalid;
		}
		if(!(PCIPro.fnPciWriteMem(BASE_ADDRESS + DOWNLOAD_REMARK_ADDRESS_OFFSET,(unsigned char*)&tmpData,sizeof(tmpData))
			&&PCIPro.fnPciWriteMem(BASE_ADDRESS + DOWNLOAD_INFORM_ADDRESS_OFFSET,(ULONG)DOWNLOAD_IS_READY)))
		{
			return err_PCI_Write_Memory_Invalid;
		}
	}
	else{
		if(!(PCIPro.fnPciWriteMem(BASE_ADDRESS + DOWNLOAD_DATA_ADDRESS_OFFSET,bd->pucData,bd->iLength)
			&&PCIPro.fnPciWriteMem(BASE_ADDRESS + DOWNLOAD_REMARK_ADDRESS_OFFSET,(unsigned char*)&tmpData,sizeof(tmpData))
			&&PCIPro.fnPciWriteMem(BASE_ADDRESS + DOWNLOAD_INFORM_ADDRESS_OFFSET,(ULONG)DOWNLOAD_IS_READY))
			)
			{
				return err_PCI_Write_Memory_Invalid;
			}
	}
	return err_Success; 
}
/*	
	FunctionName：			fnFreeMemory
	FunctionModifiedTime：	20141019
	FunctionPurpose：		free memory
*/
errorCode CSingleton::fnFreeMemory(sBufData* bd)
{
	unsigned char* puiTestData = bd->pucData;
	if (puiTestData==NULL)
	{
		return err_MS_Memory_Free_Invalid;
	}
	DELETE_POINT(puiTestData);
	return err_Success;
}
/*	
	FunctionName：			fnFreeAllMemoryAndData
	FunctionModifiedTime：	20141019
	FunctionPurpose：		free all memory and data
*/
errorCode CSingleton::fnFreeAllMemoryAndData()//Without Too Much Thinking 20141003
{
	std::vector<BufData>::iterator itrs = m_vBufSlave.begin();
	while(itrs != m_vBufSlave.end())
	{
		BufData BD = *itrs;
		if(err_Success!=fnFreeMemory(&BD))return err_MS_Memory_Free_Invalid;
		itrs = m_vBufSlave.erase(itrs);
	}
	std::vector<BufData>::iterator itrm = m_vBufMaster.begin();
	while(itrm != m_vBufMaster.end())
	{
		BufData BD = *itrm;
		if(err_Success!=fnFreeMemory(&BD))return err_MS_Memory_Free_Invalid;
		itrm = m_vBufMaster.erase(itrm);
	}
	std::vector<BufData>::iterator itra = m_vBufAdvance.begin();
	while(itra != m_vBufAdvance.end())
	{
		BufData BD = *itra;
		if(err_Success!=fnFreeMemory(&BD))return err_MS_Memory_Free_Invalid;
		itra = m_vBufAdvance.erase(itra);
	}
	return err_Success;
}
/*	
	FunctionName：			fnBuffPop
	FunctionModifiedTime：	20141019
	FunctionPurpose：		pop memory and data
*/
errorCode CSingleton::fnBuffPop(const UINT uicmdid,std::vector<BufData>* pBD)
{
	std::vector<BufData>::iterator itrm = pBD->begin();;
	while(itrm != pBD->end())
	{
		if(uicmdid == itrm->iCmd)
		{
			unsigned char* pucDataMaster;
			pucDataMaster = itrm->pucData;
			delete[] pucDataMaster;
			pucDataMaster = NULL;

			itrm = pBD->erase(itrm);
			return err_Success;
		}
		itrm++;
	}
	return err_MS_Memory_Free_Invalid;
}

/*	
	FunctionName：			fnDelay
	FunctionModifiedTime：	20141019
	FunctionPurpose：		delay some time
*/
errorCode CSingleton::fnDelay()
{
	int ii,ij,ik;
	for (ii = 0;ii<100;ii++)
	{
		for (ij = 0;ij<100;ij++)
		{
			for (ik = 0;ik<10;ik++)
			{
			}
		}
	}
	return err_Success;
}

void CSingleton::fnEnableIntterupt()
{
	if(!m_bIsInited)PCIPro.fnPciStartThread();
}

volatile static long int x = 0;

void PASCAL CallBackFunc(UINT wTimerID, UINT msg,DWORD dwUser,DWORD dwl,DWORD dw2) 
{
	LINK.fnBuffTrans();
}

bool CSingleton::InitTimerCheckSlave()
{
	fnInit();
	::timeSetEvent (m_iPeriod, m_iPrecision,CallBackFunc,NULL,TIME_PERIODIC); 
	return true;
}
errorCode CSingleton::fnReleasePCI()
{
	return (PCIPro.fnPciClose())?err_Success:err_PCI_Release_Invalid;
}
/*	
	FunctionName：			fnSendToBuffer
	FunctionModifiedTime：	20141019
	FunctionPurpose：		interface to get info
*/
errorCode CSingleton::fnSendToBuffer(BYTE *m_ControlComd,int len,int *ComdID)
{	
	if(err_Success == fnInit()
		&&err_Success == fnBuffRoute(m_ControlComd,len,ComdID))
		return err_Success;
	else 
		return err_MS_IS_ABORT;
}
/*	
	FunctionName：			fnBuffPull
	FunctionModifiedTime：	20141019
	FunctionPurpose：		interface to return info
*/
errorCode CSingleton::fnBuffPull(int ComdID,BYTE *m_FeedBackInfo,int len,int Waittime)
{
	UINT uiCountMS = 0;
	int iSleepTimeMS = 2;
	while(1)
	{
		while(RAMRead == StatusSlave) ;
		if(RAMRead != StatusSlave)
		{
			StatusSlave = InterfaceRead;
			std::vector<BufData>::iterator itr = m_vBufSlave.begin();
			while(itr != m_vBufSlave.end())
			{
				if (ComdID == itr->iCmd)
				{
					BufData BD = *itr;
					itr = m_vBufSlave.erase(itr);
					int i;
					for (i = 0;i < len;i++)
					{
						*(m_FeedBackInfo + i)= *(BD.pucData + i);
					}
					StatusSlave = Idle;
					fnDelay();
					return err_Success;
				}
				itr++;
			}
			StatusSlave = Idle;
			uiCountMS++;
			Sleep(iSleepTimeMS);
			if ((int)(uiCountMS*iSleepTimeMS) > Waittime)
				return err_MS_Pull_Invalid;
		}else
		{
			uiCountMS++;
			Sleep(iSleepTimeMS);
			if ((int)(uiCountMS*iSleepTimeMS) > Waittime)
				return err_MS_Pull_Invalid;
		}
	}
}
errorCode CSingleton::fnFakeAbortTimer()
{
	m_bAsyLock = true;
	return err_Success;
}
errorCode CSingleton::fnFakeRestartTimer()
{
	m_bAsyLock = false;
	return err_Success;
}
errorCode CSingleton::fnManualIntterupt()
{
	m_bGreenPath = UNLOCKED;
	return err_Success;
}
errorCode CSingleton::fnReset()
{
	if(err_Success == fnFreeAllMemoryAndData())
	return err_Success;
	else return err_MS_IS_ABORT;
}