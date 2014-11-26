/*Modified by mzq in 20141019*/
#include "stdafx.h"
#include "CBufMS.h"
//#include "..\SMTLogManage\SMTLogManage\SMTLogManage.h" 
//#include "TypeConvert.h"

#include "mmsystem.h" 
#pragma comment(lib, "winmm.lib")
#include <afxwin.h>

#define START_THREAD cspLink->GetInstance().m_bTreadIsRunning = true
#define END_THREAD cspLink->GetInstance().m_bTreadIsRunning = false
#define MASTER_WAIT while(RAMWrite == StatusMaster || InterfaceWrite == StatusMaster) 
#define MASTER_IS_READY InterfaceWrite != StatusMaster && RAMWrite != StatusMaster
#define SLAVE_WAIT while(InterfaceRead == StatusSlave || RAMRead == StatusSlave)
#define SLAVE_IS_READY InterfaceRead != StatusSlave && RAMRead != StatusSlave
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
	if(buf == NULL)
		return err_MS_Memory_Route_Invalid;
	for (uicount = 0;uicount < len;uicount++)
	{
		*(buf+uicount) = *(m_ControlComd+uicount);
	}
	m_sCurMaster.pucData = buf;
	m_sCurMaster.iCmd = m_uicmdid;
	m_sCurMaster.iLength = len;
	MASTER_WAIT ;
	if(MASTER_IS_READY)
	{
		StatusMaster = InterfaceWrite;
		if(!fnIsAdvanceCMD(m_ControlComd))
			m_vBufMaster.push_back(m_sCurMaster);
		else 
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
	unsigned char ucVeryTimes = 0;
	BufData updateData;
	do{
		unsigned char* pucRemark = new unsigned char[REMARK_LENGTH];		
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
		
		if(PCIPro.fnPciReadMem(BASE_ADDRESS + UPDATE_DATA_ADDRESS_OFFSET,updateData.iLength,pucData))
		{	
			if(true&&((fnGetVeryCode(pucData,updateData.iLength)) != updateData.iCheckCode))
			{
				ucVeryTimes++;
				DELETE_POINT(pucRemark);
				DELETE_POINT(pucData);
				if(ucVeryTimes >= READ_TRY)
				{			
					m_bSynLock = WRITE;
					return err_MS_Check_Code_Invalid;
				}			
			}
			else
			{
				ucVeryTimes = READ_TRY;
				updateData.pucData = pucData;
				SLAVE_WAIT ;
				if(SLAVE_IS_READY)
				{
					StatusSlave = RAMRead;
					if (m_vBufSlave.size()>BUF_SLAVE_SIZE)
					{
						fnBuffPop(m_vBufSlave[0].iCmd,&m_vBufSlave);
					}
					m_vBufSlave.push_back(updateData);
					StatusSlave = Idle;
					DELETE_POINT(pucRemark);
#ifdef WINTEST
					fnSetWindowText(fnGetCWnd(),updateData.iCmd,fnGetTextTimerSlaveID());
#endif
					//DELETE_POINT(pucData);
				}else 
				{
					DELETE_POINT(pucRemark);
					DELETE_POINT(pucData);
					return err_PCI_Read_Memory_Invalid;
				}
			}
		}else  
		{
			DELETE_POINT(pucRemark);
			DELETE_POINT(pucData);
			m_ulReadRetry++;
			return	err_PCI_Read_Memory_Invalid;
		}
	}while(ucVeryTimes < READ_TRY);
	

	if(!fnMasterPop(updateData.iCmd))
		return err_MS_Memory_Trans_Invalid;
	
	m_ucInvalidRetry = 0;
	m_ucErrorRetry = 0;
	m_ulReadRetry = 0;
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
	MASTER_WAIT ;
	if(MASTER_IS_READY)
	{

		StatusMaster = RAMWrite;
		if (0 != m_vBufMaster.size())
		{	
			if(0 != m_vBufMaster.size()&&err_Success==fnHardProc(&m_vBufMaster[BUFFER_ZERO],COMMON_CMD)) 
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
errorCode CSingleton::fnWriteAdvance(void)
{
	MASTER_WAIT ;
	if(MASTER_IS_READY)
	{
		StatusMaster = RAMWrite;
		if (0 != m_vBufAdvance.size())
		{	
			if(0 != m_vBufAdvance.size()&&err_Success == fnHardProc(&m_vBufAdvance[BUFFER_ZERO],ADVANCE_CMD)) 
			{
				StatusMaster = Idle;
				m_bSynLock = READ;
				m_ucInvalidRetry = 0;
				m_ucErrorRetry = 0;
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

/*	
	FunctionName：			fnIsAdvanceCMD
	FunctionModifiedTime：	20141102
	FunctionPurpose：		Is Advanced CMD? 
*/
bool CSingleton::fnIsAdvanceCMD(unsigned char* ucpCMD)
{
	int* CmdNo;
	CmdNo = ((int*) ucpCMD);
	if(*CmdNo > 0)
		return false;
	else
		return true;
}
/*	
	FunctionName：			fnBuffTrans
	FunctionModifiedTime：	20141019
	FunctionPurpose：		send or get cmd in turn 
*/
errorCode CSingleton::fnBuffTrans()
{
	ULONG ulFeedbackData;
	ULONG ulFeedbackInterruptFlag;
	ULONG ulFeedbackWriteStatus;
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
							FATAL_ERROR;
							break;
						case err_PCI_Read_Memory_Invalid:
							FATAL_ERROR;
							break;
						default :
							break;
						}
						m_ulReadRetry = REWRITE_ZERO;
					}
					break;
				case UPDATE_CHECK_WORD_IS_INVALID:
					{
						m_ucErrorRetry++;
						m_ulReadRetry = REWRITE_ZERO;
						if(READ_TRY < m_ucErrorRetry)
							FATAL_ERROR;
						
					}
					break;
				case UPDATE_CMD_IS_INVALID:
					{
						m_ucInvalidRetry++;
						m_ulReadRetry = REWRITE_ZERO;
						if(READ_TRY < m_ucInvalidRetry)
							FATAL_ERROR;
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
			m_ulReadRetry++;
			if(m_ulReadRetry > REWRITE_TIME)
			{           
				m_bSynLock = WRITE;
				m_ulReadRetry = REWRITE_ZERO;
				return err_Success;
			}
		}
	}
	if(WRITE == m_bSynLock)
		fnWrite(); 

	return err_Success;
}
/*	
	FunctionName：			fnHardProc
	FunctionModifiedTime：	20141019
	FunctionPurpose：		write cmd buffer to memory 
*/
errorCode CSingleton::fnHardProc(sBufData* bd, unsigned char ucCharacter)
{
	
	bd->iCheckCode = fnGetVeryCode(bd->pucData,bd->iLength);
	DataRemark tmpData;
	tmpData.ulCmdid = bd->iCmd;
	tmpData.usLength = bd->iLength;
	tmpData.usCheckCode = bd->iCheckCode;

	if(bd->iLength > MAX_DOWNLOAD_LENGTH)
	{	
		int itimes = bd->iLength/MAX_DOWNLOAD_LENGTH;
		int iremainder = bd->iLength%MAX_DOWNLOAD_LENGTH;
		int i;
		for (i = 0;i<itimes;i++)
		{
			if(!(COMMON_CMD == ucCharacter&&PCIPro.fnPciWriteMem(BASE_ADDRESS + DOWNLOAD_DATA_ADDRESS_OFFSET + i*MAX_DOWNLOAD_LENGTH,bd->pucData + i*MAX_DOWNLOAD_LENGTH,MAX_DOWNLOAD_LENGTH)))
			{
				return err_PCI_Write_Memory_Invalid;
			}
			if(!(ADVANCE_CMD == ucCharacter)) ;
			//////////////////////////////////
		}
		if(!(COMMON_CMD == ucCharacter&&PCIPro.fnPciWriteMem(BASE_ADDRESS + DOWNLOAD_DATA_ADDRESS_OFFSET + itimes*MAX_DOWNLOAD_LENGTH + iremainder,bd->pucData + itimes*MAX_DOWNLOAD_LENGTH,iremainder)))
		{
			return err_PCI_Write_Memory_Invalid;
		}
		if(!(ADVANCE_CMD == ucCharacter)) ;
		///////////////////////////////////
		if(!(COMMON_CMD == ucCharacter&&PCIPro.fnPciWriteMem(BASE_ADDRESS + DOWNLOAD_REMARK_ADDRESS_OFFSET,(unsigned char*)&tmpData,sizeof(tmpData))
			&&PCIPro.fnPciWriteMem(BASE_ADDRESS + DOWNLOAD_INFORM_ADDRESS_OFFSET,(ULONG)DOWNLOAD_IS_READY)))
		{
			return err_PCI_Write_Memory_Invalid;
		}
		if(!(ADVANCE_CMD == ucCharacter)) ;
		///////////////////////////////////
	}
	else{
		if(!(COMMON_CMD == ucCharacter&&PCIPro.fnPciWriteMem(BASE_ADDRESS + DOWNLOAD_DATA_ADDRESS_OFFSET,bd->pucData,bd->iLength)
			&&PCIPro.fnPciWriteMem(BASE_ADDRESS + DOWNLOAD_REMARK_ADDRESS_OFFSET,(unsigned char*)&tmpData,sizeof(tmpData))
			&&PCIPro.fnPciWriteMem(BASE_ADDRESS + DOWNLOAD_INFORM_ADDRESS_OFFSET,(ULONG)DOWNLOAD_IS_READY))
			)
			{
				return err_PCI_Write_Memory_Invalid;
			}
		if(!(ADVANCE_CMD == ucCharacter)) ;
		///////////////////////////////////
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
		if(err_Success!=fnFreeMemory(&BD))
			return err_MS_Memory_Free_Invalid;
		itrs = m_vBufSlave.erase(itrs);
	}
	std::vector<BufData>::iterator itrm = m_vBufMaster.begin();
	while(itrm != m_vBufMaster.end())
	{
		BufData BD = *itrm;
		if(err_Success!=fnFreeMemory(&BD))
			return err_MS_Memory_Free_Invalid;
		itrm = m_vBufMaster.erase(itrm);
	}
	std::vector<BufData>::iterator itra = m_vBufAdvance.begin();
	while(itra != m_vBufAdvance.end())
	{
		BufData BD = *itra;
		if(err_Success!=fnFreeMemory(&BD))
			return err_MS_Memory_Free_Invalid;
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
	//LINK.fnBuffTrans();
}
/*	
	FunctionName：			fnMain
	FunctionModifiedTime：	20141102
	FunctionPurpose：		Main Thread
*/
UINT CSingleton::fnMain(LPVOID pParam)
{
	CSingleton* cspLink = (CSingleton*)pParam;
	cspLink->GetInstance().m_bMainAbort = false;
	do{
		START_THREAD;
		cspLink->GetInstance().fnBuffTrans();
		END_THREAD;
		Sleep(cspLink->GetInstance().m_iPeriod);
	}while(!cspLink->GetInstance().m_bMainAbort);
	return 0;
}
CSingleton::~CSingleton()
{
	fnThreadDestroy();
}
bool CSingleton::fnThreadStart()
{
	AfxBeginThread(fnMain,this);
	return true;
}
bool CSingleton::fnThreadAbort()
{
	while(m_bTreadIsRunning) ;
	m_bMainAbort = false;
	return true;
}
bool CSingleton::fnThreadDestroy()
{
	while(m_bTreadIsRunning) ;
	m_bMainAbort = false;
	while(Idle != StatusMaster || Idle != StatusSlave) ;
	fnFreeAllMemoryAndData();
	PCIPro.fnPciClose();
	return true;
}
bool CSingleton::InitTimerCheckSlave()
{
	if(!(err_Success==fnInit()&&fnThreadStart()))
		return false;
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
		SLAVE_WAIT ;
		if(SLAVE_IS_READY)
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
errorCode CSingleton::fnReset()
{
	if(err_Success == fnFreeAllMemoryAndData())
		return err_Success;
	else 
		return err_MS_IS_ABORT;
}

bool CSingleton::fnMasterPop(int ivar)
{
	MASTER_WAIT ;
	if(MASTER_IS_READY)
	{
		StatusMaster = RAMWrite;
		if(!(err_Success == fnBuffPop(ivar,&m_vBufMaster)
			||err_Success == fnBuffPop(ivar,&m_vBufAdvance)))
		{
			StatusMaster = Idle;
			m_ulReadRetry++;
			return false;
		}
		StatusMaster = Idle;
		return true;
	}
	else
	{
		return false;
	}
}

errorCode CSingleton::fnSendMessageToWinform()
{
	BufData updateData;
	unsigned char ucVeryTimes = 0;
	ULONG ulAdvanceIsBusy;
	ULONG ulAdvanceIsCMD;
	ULONG ulAdvanceIsERR;
	ULONG ulAdvanceIsBTN;
	PCIPro.fnPciReadMem(BASE_ADDRESS + UPDATE_HIGH_BUSY_ADDRESS_OFFSET,ulAdvanceIsBusy);

	if(PCIPro.fnPciReadMem(BASE_ADDRESS + UPDATE_HIGH_CMD_ADDRESS_OFFSET,ulAdvanceIsCMD)
		&&PCIPro.fnPciReadMem(BASE_ADDRESS + UPDATE_HIGH_ERR_ADDRESS_OFFSET,ulAdvanceIsERR)
		&&PCIPro.fnPciReadMem(BASE_ADDRESS + UPDATE_HIGH_BTN_ADDRESS_OFFSET,ulAdvanceIsBTN))
	{
		PCIPro.fnPciWriteMem(BASE_ADDRESS + UPDATE_HIGH_BTN_ADDRESS_OFFSET,ULONG(0));
		PCIPro.fnPciWriteMem(BASE_ADDRESS + UPDATE_HIGH_ERR_ADDRESS_OFFSET,ULONG(0));
		PCIPro.fnPciWriteMem(BASE_ADDRESS + UPDATE_HIGH_CMD_ADDRESS_OFFSET,ULONG(0));

		if(BUSY == (ulAdvanceIsCMD&LOW16_MASK))
		{
			do{
				unsigned char* pucRemark = new unsigned char[REMARK_LENGTH];
				if(PCIPro.fnPciReadMem(BASE_ADDRESS + UPDATE_HIGH_REMARK_ADDRESS_OFFSET,REMARK_LENGTH,pucRemark))
				{
					updateData.iCmd =((DataRemark *)pucRemark)->ulCmdid;
					updateData.iLength = ((DataRemark *)pucRemark)->usLength;
					updateData.iCheckCode =(((DataRemark *)pucRemark)->usCheckCode)&0xFF;
				}else 
				{
					DELETE_POINT(pucRemark);
					//send fatal message
				}
				unsigned char* pucData = new unsigned char[updateData.iLength];
				if(PCIPro.fnPciReadMem(BASE_ADDRESS + UPDATE_HIGH_DATA_ADDRESS_OFFSET,updateData.iLength,pucData))
				{	
					if(true&&((fnGetVeryCode(pucData,updateData.iLength)) != updateData.iCheckCode))
					{
						ucVeryTimes++;
						DELETE_POINT(pucRemark);
						DELETE_POINT(pucData);
						if(ucVeryTimes >= READ_TRY)
						{
							m_bSynLock = WRITE;
							//send fatal message
						}			
					}
					else
					{
						ucVeryTimes = READ_TRY;
						updateData.pucData = pucData;
						SLAVE_WAIT ;
						if(SLAVE_IS_READY)
						{
							StatusSlave = RAMRead;
							if (m_vBufSlave.size()>BUF_SLAVE_SIZE)
							{
								fnBuffPop(m_vBufSlave[0].iCmd,&m_vBufSlave);
							}
							m_vBufSlave.push_back(updateData);
							StatusSlave = Idle;
							DELETE_POINT(pucRemark);
						}
						else 
						{
							DELETE_POINT(pucRemark);
							DELETE_POINT(pucData);
							//send fatal message
						}
						
					}
				}else  
				{
					DELETE_POINT(pucRemark);
					DELETE_POINT(pucData);
					m_ulReadRetry++;
					//send fatal message
				}
			}while(ucVeryTimes < READ_TRY);

			if(!fnMasterPop(updateData.iCmd))
				return err_MS_Memory_Trans_Invalid;

			m_ucInvalidRetry = 0;
			m_ucErrorRetry = 0;
			m_ulReadRetry = 0;
			m_bSynLock = WRITE;
		}

		ucVeryTimes = 0;
		if(BUSY == (ulAdvanceIsBTN&LOW16_MASK))
		{
			do{
				ULONG ulBTNLength;
				ULONG ulBTNVeryCode;
				if(PCIPro.fnPciReadMem(BASE_ADDRESS + UPDATE_BTN_LENGHT_ADDRESS_OFFSET,ulBTNLength)
					&&PCIPro.fnPciReadMem(BASE_ADDRESS + UPDATE_BTN_VERY_CODE_ADDRESS_OFFSET,ulBTNVeryCode))
				{
					unsigned char* pucData = new unsigned char[ulBTNLength&LOW8_MASK];
					if(PCIPro.fnPciReadMem(BASE_ADDRESS + UPDATE_BTN_ADDRESS_OFFSET,(ulBTNLength&LOW8_MASK),pucData))
					{	
						if(true&&((fnGetVeryCode(pucData,ulBTNLength&LOW8_MASK)) != (ulBTNVeryCode&LOW8_MASK)))
						{
							ucVeryTimes++;
							DELETE_POINT(pucData);
							if(ucVeryTimes >= READ_TRY)
							{
								m_bSynLock = WRITE;
								//send fatal message
							}			
						}
						else
						{
							ucVeryTimes = READ_TRY;
							//send message 
							DELETE_POINT(pucData);
						}
					}else  
					{
						DELETE_POINT(pucData);
						m_ulReadRetry++;
						//send fatal message
					}
				}else 
				{
					//send fatal message
				}	
			}while(ucVeryTimes < READ_TRY);
			m_ucInvalidRetry = 0;
			m_ucErrorRetry = 0;
			m_ulReadRetry = 0;
			m_bSynLock = WRITE;
		}

		ucVeryTimes = 0;
		if(BUSY == (ulAdvanceIsERR&LOW16_MASK))
		{

			do{
				unsigned char* pucRemark = new unsigned char[REMARK_LENGTH];
				if(PCIPro.fnPciReadMem(BASE_ADDRESS + UPDATE_ERROR_REMARK_ADDRESS_OFFSET,REMARK_LENGTH,pucRemark))
				{
					updateData.iCmd =((DataRemark *)pucRemark)->ulCmdid;
					updateData.iLength = ((DataRemark *)pucRemark)->usLength;
					updateData.iCheckCode =(((DataRemark *)pucRemark)->usCheckCode)&0xFF;
				}else 
				{
					DELETE_POINT(pucRemark);
					//send fatal message
				}
				unsigned char* pucData = new unsigned char[updateData.iLength];
				if(PCIPro.fnPciReadMem(BASE_ADDRESS + UPDATE_ERROR_ADDRESS_OFFSET,updateData.iLength,pucData))
				{	
					if(true&&((fnGetVeryCode(pucData,updateData.iLength)) != updateData.iCheckCode))
					{
						ucVeryTimes++;
						DELETE_POINT(pucRemark);
						DELETE_POINT(pucData);
						if(ucVeryTimes >= READ_TRY)
						{
							m_bSynLock = WRITE;
							//send fatal message
						}			
					}
					else
					{
						ucVeryTimes = READ_TRY;
						//send message
						updateData.pucData = pucData;
					}
				}else  
				{
					DELETE_POINT(pucRemark);
					DELETE_POINT(pucData);
					m_ulReadRetry++;
					//send fatal message
				}
			}while(ucVeryTimes < READ_TRY);

			m_ucInvalidRetry = 0;
			m_ucErrorRetry = 0;
			m_ulReadRetry = 0;
			m_bSynLock = WRITE;
		}
	}else
	{
		//send fatal message
	}
	//Read EXE
	
	return err_Success;
}
int CSingleton::fnGetVeryCode(unsigned char* ucp,int ilen)
{
	int uicount = 0;
	unsigned int icheckword = 0;
	for (uicount = 0;uicount < ilen;uicount++)
	{
		icheckword += *(ucp+uicount);
	}
	icheckword &= LOW8_MASK;
	return (int)icheckword;
}

#ifdef WINTEST
	int CSingleton::fnGetTimerSlaveCount()
	{
		return m_TimerSlaveCount;
	}
	void CSingleton::fnSetTimerSlaveCount(int icount)
	{
		m_TimerSlaveCount = icount;
	}
	//设定或者获取显示轮询定时器的窗体控件的ID
	void CSingleton::fnSetTextTimerSlaveID(int idc)
	{
		m_TextSlaveCount = idc;
	}
	int CSingleton::fnGetTextTimerSlaveID()
	{
		return m_TextSlaveCount;
	}
	//在指定窗体的指定位置写入数据
	bool CSingleton::fnSetWindowText(CWnd* cwnd,int iconten,unsigned int iposition)
	{
		CString cstr;
		cstr.Format(_T("%d"),iconten);
		CStatic* pWnd = (CStatic*)GetDlgItem(cwnd->m_hWnd,iposition);
		SendMessage(cwnd->m_hWnd,WM_MYMESSAGE,iposition,iconten);
		return true;
	}
	bool CSingleton::fnSetWindowText(CWnd* cwnd,CString iconten,unsigned int iposition)
	{
		CStatic* pWnd = (CStatic*)GetDlgItem(cwnd->m_hWnd,iposition);
		unsigned int i_var = _ttoi(iconten);
		SendMessage(cwnd->m_hWnd,WM_MYMESSAGE,iposition,i_var);
		return true;
	}
#endif