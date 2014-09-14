#include "stdafx.h"
#include "CBufMS.h"


//#include "EX_SDMDlg.h"
#include "mmsystem.h" 
#pragma comment(lib, "winmm.lib")

//设定或获得主从锁定的状态
bool CSingleton::fnGetLockM()
{
	return m_bLockMaster;
}
//初始化
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
bool CSingleton::fnSetLockM(bool tmp)
{
	m_bLockMaster = tmp;
	return m_bLockMaster;
}
bool CSingleton::fnSetLockS(bool tmp)
{
	m_bLockSlave = tmp;
	return m_bLockSlave;
}
bool CSingleton::fnGetLockS()
{
	return m_bLockSlave;
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
//设定或获取轮询定时器的定时周期或精度
int CSingleton::fnGetTimerPeriodms()
{
	return m_iPeriod;
}
void CSingleton::fnSetTimerPeriodms(int itimerperiod)
{
	m_iPeriod = itimerperiod;
}
int CSingleton::fnGetTimerPrecisionms()
{
	return m_iPrecision;
}
void CSingleton::fnSetTimerPrecisionms(int itimerprecisionms)
{
	m_iPrecision = itimerprecisionms;
}
//设定或获得主窗体的指针
void CSingleton::fnSetCWnd(CWnd* cwnd)
{
	m_pCWnd = cwnd;
}
CWnd* CSingleton::fnGetCWnd()
{
	return m_pCWnd;
}
//设定或者获得当前轮询定时器的有效搬移数据的次数
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

//将输入的结构体或结构体特征进行链路管理
errorCode CSingleton::fnBuffRoute(sTestData* sBuf,unsigned int uioffset)
{
	unsigned char* puiTestData = (unsigned char*)sBuf;
	UCHAR uiTestLen = sizeof(*sBuf);

	if (uiTestLen<0||m_bLockMaster)
	{
		//LINK.m_bInterfaceLock = UNLOCKED;
		return err_MS_Memory_Route_Invalid;
	}
	m_bLockMaster = true;
	unsigned int uicount = 0;
	unsigned char* buf = new unsigned char[uiTestLen];
	if(buf == NULL)
	{
		//LINK.m_bInterfaceLock = UNLOCKED;
		return err_MS_Memory_Route_Invalid;
	}
	for (uicount = 0;uicount < uiTestLen;uicount++)
	{
		*(buf+uicount) = *(puiTestData+uicount);
	}


	m_sCurMaster.pucData = buf;
	m_sCurMaster.iCmd = m_uicmdid;
	m_sCurMaster.iLength = uiTestLen;
	m_sCurMaster.uiOffset = uioffset;
	if(RAMWrite != LINK.Status)
	{	//操作MVector，防止过程接口对其进行操作
		LINK.Status = InterfaceWrite;
		m_vBufMaster.push_back(m_sCurMaster);
		m_bLockMaster = false;
		m_uicmdid++;
		LINK.Status = Idle;
		return err_Success;
	}else
	{
		delete[] buf;
		buf = NULL;
		return err_MS_Push_Invalid;
	}	
}
errorCode CSingleton::fnBuffRoute(unsigned char* m_ControlComd,int len,int* ComdID,unsigned int uioffset)
{

	if (m_bLockMaster||len<0)
	{
		//LINK.m_bInterfaceLock = UNLOCKED;
		return err_MS_Memory_Route_Invalid;
	}
	m_bLockMaster = true;
	unsigned int uicount = 0;
	unsigned char* buf = new unsigned char[len];
	if(buf == NULL)return err_MS_Memory_Route_Invalid;
	for (uicount = 0;uicount < len;uicount++)
	{
		*(buf+uicount) = *(m_ControlComd+uicount);
	}

	m_sCurMaster.pucData = buf;
	m_sCurMaster.iCmd = m_uicmdid;
	m_sCurMaster.iLength = len;
	m_sCurMaster.uiOffset = uioffset;
	if(RAMWrite != LINK.Status)//LINK并未操作MVector
	{
		LINK.Status = InterfaceWrite;
		m_vBufMaster.push_back(m_sCurMaster);
		*ComdID = m_uicmdid;//返回CMDID
		m_bLockMaster = false;
		m_uicmdid++;
		//LINK.m_bInterfaceLock = UNLOCKED;
		LINK.Status = Idle;	//状态切换回闲
		return err_Success;
	}else
	{
		delete[] buf;
		buf = NULL;
		return err_MS_Push_Invalid;
	}
	//释放内存时，需要考虑释放的内存大小。2014.08.15
}


errorCode CSingleton::fnBuffTrans()//不在绿色通道情况下，常规情况，正常数据管道起作用
{
	if (LINK.fnGetLockS())
	{
		return err_MS_Memory_Trans_Invalid;
	}
	if (!m_bDualRamIsReady)
	{
		return err_MS_Memory_Trans_Invalid;
	}
	if (m_ucInvalidRetry >= UPDATE_ERROR_RETRY 
		|| m_ucErrorRetry >= UPDATE_INVALID_RETRY||m_ucReadRetry >= UPDATE_ERROR_RETRY)
	{
		//这里捅上去,sendmessage
		//目前出错的解决办法是清空内存，这样的错误来自硬件
		if(err_Success == LINK.fnFreeAllMemoryAndData())
		{
			LINK.m_bSynLock = WRITE;//写状态
			//LINK.m_bAsyClock = false;//解锁
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
	//锁定向硬件写数据的状态
	LINK.fnSetLockS(false);//没启用
	ULONG ulFeedbackData;//获取双口RAM数据
	if(LINK.m_bAsyLock)
	{
		LINK.fnSetLockS(false);
		return err_MS_IS_ABORT;
	}
	else
	{	//查看双口RAM的读写状态，在可读写情况下进行数据传输
		if(m_bDualRamIsReady)
		{//读状态，读同步锁的状态，同步锁为读方可进行读状态

		if (m_bSynLock == READ)
		{
			m_bDualRamIsReady = false;//切换双口RAM的读写状态，防止多次操作
			if(PCIPro.fnPciReadMem(BASE_ADDRESS + UPDATE_BACK_ADDRESS_OFFSET,ulFeedbackData))
			{			
				
 				switch (ulFeedbackData&0x0000FFFF)
				{
				case UPDATE_FEEDBACK_INFO_IS_READY://指定的位置有数据，可以进行取数据；
					{
						unsigned char* pucRemark = new unsigned char[REMARK_LENGTH];
						BufData updateData;
// 						updateData.iCmd = 0;
// 						updateData.iLength = 0;
// 						updateData.iCheckCode = 0;
						int icount;
						if(PCIPro.fnPciReadMem(BASE_ADDRESS + UPDATE_REMARK_ADDRESS_OFFSET,REMARK_LENGTH,pucRemark))
						{
							//读取Remark区信息，获得指令ID，指令长度，校验码
							updateData.iCmd =((DataRemark *)pucRemark)->ulCmdid;
							updateData.iLength = ((DataRemark *)pucRemark)->usLength;
							updateData.iCheckCode =(((DataRemark *)pucRemark)->usCheckCode)&0xFF;
						}else 
						{
							m_ucReadRetry++;
							m_bDualRamIsReady = true;
							delete[] pucRemark;
							pucRemark = NULL;
							return err_PCI_Read_Memory_Invalid;
						}
						unsigned char* pucData = new unsigned char[updateData.iLength];
						//读取数据包体，并核对校验码
						if(PCIPro.fnPciReadMem(BASE_ADDRESS + UPDATE_DATA_ADDRESS_OFFSET,updateData.iLength,pucData))
						{
							int icheckword = 0;
							for (icount = 0;icount <updateData.iLength;icount++)
							{
								icheckword += *(pucData + icount);
							}
							//校验码核对出错，则清空存储数据的内存
							if((icheckword&0xFF) != updateData.iCheckCode)
							{
								delete[] pucRemark;
								pucRemark = NULL;
								delete[] pucData;
								pucData = NULL;
								m_ucReadRetry++;//读取数据的校验码有错，需要重试,错三次往上捅
								m_bDualRamIsReady = true;
								//m_bSynLock = WRITE;
								return err_MS_Check_Code_Invalid;
							}else
							{
								//校验码正确，将数据写入上传队列，并保有内存
								updateData.pucData = pucData;
								if(InterfateRead != LINK.Status)//写入SVector，此时不应该为InterfaceRead状态
								{
									LINK.Status = RAMRead;
									LINK.m_vBufSlave.push_back(updateData);
									LINK.Status = Idle;
								}else return err_PCI_Read_Memory_Invalid;
							}
						}else  
						{
							delete[] pucRemark;
							pucRemark = NULL;
							delete[] pucData;
							pucData = NULL;
							m_bDualRamIsReady = true;
							m_ucReadRetry++;
							return	err_PCI_Read_Memory_Invalid;
						}
						
						delete[] pucRemark;
						pucRemark = NULL;

						//完成读任务，此时需要将保有的信息及内存释放
						if(InterfaceWrite != LINK.Status)
						{
							LINK.Status = RAMWrite;//释放内存，要操作MVector
							if(!(err_Success==LINK.fnFreeMemory(&m_vBufMaster[0])&&err_Success==LINK.fnPopBuffMaster()))
							{
								LINK.Status = Idle;
								m_bDualRamIsReady = true;
								m_ucReadRetry++;//释放内存出现问题
								return err_MS_Memory_Free_Invalid;
							}
							LINK.Status = Idle;
						}
						m_ucInvalidRetry = 0;
						m_ucErrorRetry = 0;
						m_ucReadRetry = 0;//清除读写重试次数
						m_bSynLock = WRITE;
						return err_Success;
					}
					break;
				case UPDATE_CHECK_WORD_IS_INVALID://校验码出错，需要重新发送指令；
					{
						LINK.m_ucErrorRetry++;
						m_bDualRamIsReady = true;
						m_bSynLock = WRITE;
						return err_PCI_Read_Memory_Invalid;
					}
					break;
				case UPDATE_CMD_IS_INVALID://指令无意义，需要重新发送指令；
					{
						LINK.m_ucInvalidRetry++;
						m_bDualRamIsReady = true;
						return err_PCI_Read_Memory_Invalid;
					}
					break;
				default:
					m_bDualRamIsReady = true;
					m_ucInvalidRetry = 0;
					m_ucErrorRetry = 0;
					return err_Success;//此时表示没有写入指定数据，可能是硬件忙状态
					break;
				}
			}else
			{
				if(m_ucReadRetry < READ_TRY)LINK.m_ucReadRetry++;
				else m_bSynLock = READ;//这里是个雷
				m_bDualRamIsReady = true;
				return err_PCI_Read_Memory_Invalid;
			}
		} 
		//写状态
		else
		{
			if(InterfaceWrite != LINK.Status)
			{
				LINK.Status = RAMWrite;//操作MVector
				if (0 != LINK.m_vBufMaster.size())
				{	
					m_bDualRamIsReady = false;//向双口RAM写，并锁定状态
					if(err_Success==fnHardProc(&m_vBufMaster[0])) 
					{
						LINK.Status = Idle;
						LINK.m_bSynLock = READ;
						LINK.fnSetLockS(false);
						m_bDualRamIsReady = true;

						fnSetTimerSlaveCount(fnGetTimerSlaveCount()+1);
						m_ucInvalidRetry = 0;
						m_ucErrorRetry = 0;
						return err_Success;
					}else
					{
						LINK.Status = Idle;
						LINK.fnSetLockS(false);
						m_bDualRamIsReady = true;
						m_ucInvalidRetry++;
						return err_MS_Memory_Trans_Invalid;
					}					
				}else 
				{
					LINK.Status = Idle;
					m_bDualRamIsReady = true;
					//m_ucInvalidRetry++;
					return err_PCI_Write_Memory_Invalid;
				}
			}
		}
	}else return err_MS_IS_ABORT;
}
	return err_Success;
}
//模拟的硬件处理程序
errorCode CSingleton::fnHardProc(sBufData* bd)
{
	unsigned int uicount = 0;
	unsigned short icheckword = 0;
	//计算校验码
	for (uicount = 0;uicount < bd->iLength;uicount++)
	{
		icheckword += *(bd->pucData+uicount);
	}
	icheckword &= 0x00FF;
	//组装写数据的结构体
	DataRemark tmpData;
	tmpData.ulCmdid = bd->iCmd;
	tmpData.usLength = bd->iLength;
	tmpData.usCheckCode = icheckword;
	//依次写入数据包，Remark信息和中断信息
	if(bd->iLength > MAX_DOWNLOAD_LENGTH)
	{	//多包数据的情况，最多一次写入128字节数据
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
			&&PCIPro.fnPciWriteMem(BASE_ADDRESS + DOWNLOAD_INFORM_ADDRESS_OFFSET,(ULONG)DOWNLOAD_IS_READY))
			)
		{
			return err_PCI_Write_Memory_Invalid;
		}
	}
	else{
		//单包数据情况，不多于128字节的数据
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
//释放bd持有信息的内存块
errorCode CSingleton::fnFreeMemory(sBufData* bd)
{
	//增加freeMemory()函数，专门释放输入输出内存
	unsigned char* puiTestData = bd->pucData;
	if (puiTestData==NULL)
	{
		return err_MS_Memory_Free_Invalid;
	}

	delete[] puiTestData;
	puiTestData = NULL;

	return err_Success;
}
//CSingleton::fnHardProc(sBufData* bd)
//bool CSingleton::fnFreeMemory(sBufData* bd)
//bool CSingleton::fnPopBuffMaster()
//三个函数务必连续使用
errorCode CSingleton::fnPopBuffMaster()
{
	if (!m_vBufMaster.size())
	{
		return err_MS_Data_Pop_Invalid;
	}
	vector<BufData>::iterator itr = m_vBufMaster.begin();
	m_vBufMaster.erase(itr);
	m_bDualRamIsReady = true;
	return err_Success;
}
errorCode CSingleton::fnFreeAllMemoryAndData()
{
	//释放从队列及内存
	std::vector<BufData>::iterator itrs = m_vBufSlave.begin();
	while(itrs != m_vBufSlave.end())
	{
		BufData BD = *itrs;
		if(err_Success!=fnFreeMemory(&BD))return err_MS_Memory_Free_Invalid;
		itrs = m_vBufSlave.erase(itrs);
	}
	//释放主队列及内存
	std::vector<BufData>::iterator itrm = m_vBufMaster.begin();
	while(itrm != m_vBufMaster.end())
	{
		BufData BD = *itrm;
		if(err_Success!=fnFreeMemory(&BD))return err_MS_Memory_Free_Invalid;
		itrm = m_vBufMaster.erase(itrm);
	}
	return err_Success;
}
errorCode CSingleton::fnBuffPull(const UINT uicmdid,BufData* BD)
{
	if(RAMRead != LINK.Status)
	{
		LINK.Status = InterfateRead;
		std::vector<BufData>::iterator itr = m_vBufSlave.begin();
		while(itr != m_vBufSlave.end())
		{
			if (uicmdid == itr->iCmd)
			{
				*BD = *itr;
				itr = m_vBufSlave.erase(itr);
				LINK.Status = Idle;
				return err_Success;
			}
			itr++;
		}
		LINK.Status = Idle;
		return err_MS_Pull_Invalid;
	}
	return err_MS_Pull_Invalid;
}
CString CSingleton::fnExData(BufData* BD)
{
	
	//模拟过程
	CString csShow,str,strblank,title,strex;
	strblank =_T(" ");
	title = _T("Data:\n");
	unsigned char* puiData = BD->pucData;
	//将指针指向链路层管理内存中，具有当前信息的数据所在的位置
	unsigned int uicount;
	csShow.Format(_T("%s"),title);
	for (uicount = 0;uicount < BD->iLength;uicount++)
	{
		str.Format(_T("%d"),*(puiData+uicount));
		CString tmp = csShow;
		csShow.Format(_T("%s %s %s"),tmp,str,strblank);
	}
	title = _T("\nCmdID:\n");
	strex = csShow;
	csShow.Format(_T("%s %s"),strex,title);
	strex = csShow;
	csShow.Format(_T("%s %d"),strex,BD->iCmd);
	strex = csShow;
	title = _T("\nLength:\n");
	csShow.Format(_T("%s %s"),strex,title);
	strex = csShow;
	csShow.Format(_T("%s %d"),strex,BD->iLength);
	strex = csShow;
	title = _T("\nAddress:\n");
	csShow.Format(_T("%s %s"),strex,title);
	strex = csShow;
	csShow.Format(_T("%s %d"),strex,BD->pucData);
	//释放从设备返回数据的内存
	delete[] puiData;
	puiData = NULL;
	return csShow;
}
void CSingleton::fnEnableIntterupt()
{
	if(!m_bIsInited)PCIPro.fnPciStartThread();
}
void PASCAL CallBackFunc(UINT wTimerID, UINT msg,DWORD dwUser,DWORD dwl,DWORD dw2) 
{
	LINK.fnSetWindowText(LINK.fnGetCWnd(),LINK.fnGetTimerSlaveCount(),LINK.fnGetTextTimerSlaveID());
	if(LOCKED == LINK.m_bGreenPath)
	{
		//LINK.m_bInterfaceLock = LOCKED;
		if(LINK.m_bAsyLock)
		{
			//if(err_Success == LINK.fnFreeAllMemoryAndData())
			{
				//LINK.m_bSynLock = WRITE;//写状态
				LINK.m_bAsyLock = false;//解锁
			}
			//else SendMessage
		}else
		{
			if(!LINK.fnGetLockS())
			{
				switch(LINK.fnBuffTrans())
				{
				case err_Success:
					break;
				case err_MS_IS_ABORT:
					//向上捅一个消息！
					break;
				case err_PCI_Write_Memory_Invalid:
					break;
				case err_PCI_Read_Memory_Invalid:
					break;
				default:
					break;
				}
			}
		}
		//LINK.m_bInterfaceLock = UNLOCKED;
	}
	else{
		//向指定RAM区域写入数据，通知底层硬件
		//这个地方应该设计一个状态机,代码没设计好
		if (WRITE == LINK.m_bGreenPathStatus)
		{

		}
		else if(READ == LINK.m_bGreenPathStatus)
		{
			LINK.m_bInterfaceLock = UNLOCKED;//绿色通道执行完毕，解锁正常工作
		}
	}
}
bool InitTimerCheckSlave()
{
	::timeSetEvent (LINK.fnGetTimerPeriodms(), LINK.fnGetTimerPrecisionms(),
		CallBackFunc,NULL,TIME_PERIODIC); 
	LINK.fnEnableIntterupt();
	return true;
}
errorCode CSingleton::fnReleasePCI()
{
	return (PCIPro.fnPciClose())?err_Success:err_PCI_Release_Invalid;
}
//20140831下面这段代码 尚未测试，接口测试时，请着重测试。
//接口测试尚未完整测试，原因，过程控制代码无法满足轮询条件
errorCode CSingleton::fnBuffPull(int ComdID,BYTE *m_FeedBackInfo,int len,int Waittime)
{
	if(RAMRead != LINK.Status)
	{
		LINK.Status = InterfateRead;
		std::vector<BufData>::iterator itr = m_vBufSlave.begin();
		while(itr != m_vBufSlave.end())
		{
			if (ComdID == itr->iCmd)
			{
				BufData BD = *itr;
				itr = m_vBufSlave.erase(itr);
				/*				LINK.m_bInterfaceLock = UNLOCKED;*/
				int i;
				for (i = 0;i < len;i++)
				{
					*(m_FeedBackInfo + i)= *(BD.pucData + i);
				}
				LINK.Status = Idle;
				return err_Success;
			}
			itr++;
		}
		/*		LINK.m_bInterfaceLock = UNLOCKED;*/
		LINK.Status = Idle;
		return err_MS_Pull_Invalid;
	}else return err_MS_Pull_Invalid;
	
}
errorCode CSingleton::fnFakeAbortTimer()
{
	LINK.m_bAsyLock = true;
	return err_Success;
}
errorCode CSingleton::fnFakeRestartTimer()
{
	LINK.m_bAsyLock = false;
	return err_Success;
}
errorCode CSingleton::fnManualIntterupt()
{
	//LINK.m_bInterfaceLock = LOCKED;//锁定为忙状态，下一周期开始时为绿色通道
	LINK.m_bGreenPath = UNLOCKED;
	return err_Success;
}
//过程控制接口，具体使用时，需要稍加调整
errorCode CSingleton::fnSendToBuffer(BYTE *m_ControlComd,int len,int *ComdID)
{
// 	if(m_bInterfaceLock == UNLOCKED)
// 	{
// 		m_bInterfaceLock = LOCKED;
		if(err_Success == LINK.fnInit()&&err_Success == LINK.fnBuffRoute(m_ControlComd,len,ComdID,0))
		{
			//m_bInterfaceLock = UNLOCKED;
			return err_Success;
		}else 
		{
			//m_bInterfaceLock = UNLOCKED;
			return err_MS_IS_ABORT;
		}
}
/*
ProcErr ReceiveInfoBuffer(int ComdID,BYTE *m_FeedBackInfo,int len,int Waittime)
{
// 	if(m_bInterfaceLock == UNLOCKED)
// 	{
// 		m_bInterfaceLock = LOCKED;
		if(err_Success == LINK.fnInit()&&err_Success == LINK.fnBuffPull(ComdID,m_FeedBackInfo,len))
		{
			//m_bInterfaceLock = UNLOCKED;
			return eMemMallocErr;
		}else
		{
			//m_bInterfaceLock = UNLOCKED;
			return eMemMallocErr;
		}
//
}
*/