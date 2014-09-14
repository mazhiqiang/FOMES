#include "stdafx.h"
#include "CBufMS.h"


//#include "EX_SDMDlg.h"
#include "mmsystem.h" 
#pragma comment(lib, "winmm.lib")

//�趨��������������״̬
bool CSingleton::fnGetLockM()
{
	return m_bLockMaster;
}
//��ʼ��
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
//��ָ�������ָ��λ��д������
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
//�趨���ȡ��ѯ��ʱ���Ķ�ʱ���ڻ򾫶�
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
//�趨�����������ָ��
void CSingleton::fnSetCWnd(CWnd* cwnd)
{
	m_pCWnd = cwnd;
}
CWnd* CSingleton::fnGetCWnd()
{
	return m_pCWnd;
}
//�趨���߻�õ�ǰ��ѯ��ʱ������Ч�������ݵĴ���
int CSingleton::fnGetTimerSlaveCount()
{
	return m_TimerSlaveCount;
}
void CSingleton::fnSetTimerSlaveCount(int icount)
{
	m_TimerSlaveCount = icount;
}
//�趨���߻�ȡ��ʾ��ѯ��ʱ���Ĵ���ؼ���ID
void CSingleton::fnSetTextTimerSlaveID(int idc)
{
	m_TextSlaveCount = idc;
}
int CSingleton::fnGetTextTimerSlaveID()
{
	return m_TextSlaveCount;
}

//������Ľṹ���ṹ������������·����
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
	{	//����MVector����ֹ���̽ӿڶ�����в���
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
	if(RAMWrite != LINK.Status)//LINK��δ����MVector
	{
		LINK.Status = InterfaceWrite;
		m_vBufMaster.push_back(m_sCurMaster);
		*ComdID = m_uicmdid;//����CMDID
		m_bLockMaster = false;
		m_uicmdid++;
		//LINK.m_bInterfaceLock = UNLOCKED;
		LINK.Status = Idle;	//״̬�л�����
		return err_Success;
	}else
	{
		delete[] buf;
		buf = NULL;
		return err_MS_Push_Invalid;
	}
	//�ͷ��ڴ�ʱ����Ҫ�����ͷŵ��ڴ��С��2014.08.15
}


errorCode CSingleton::fnBuffTrans()//������ɫͨ������£�����������������ݹܵ�������
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
		//����ͱ��ȥ,sendmessage
		//Ŀǰ����Ľ���취������ڴ棬�����Ĵ�������Ӳ��
		if(err_Success == LINK.fnFreeAllMemoryAndData())
		{
			LINK.m_bSynLock = WRITE;//д״̬
			//LINK.m_bAsyClock = false;//����
			m_ucInvalidRetry = 0;
			m_ucErrorRetry = 0;
			m_ucReadRetry = 0;
		}
		if (m_ucInvalidRetry >= UPDATE_ERROR_RETRY )//У�������
		{
		}else if (m_ucErrorRetry >= UPDATE_INVALID_RETRY)//ָ��������
		{
		}else if (m_ucReadRetry >= UPDATE_ERROR_RETRY)//PCIͨ���д�
		{
		}

		return err_MS_IS_ABORT;
	}
	//������Ӳ��д���ݵ�״̬
	LINK.fnSetLockS(false);//û����
	ULONG ulFeedbackData;//��ȡ˫��RAM����
	if(LINK.m_bAsyLock)
	{
		LINK.fnSetLockS(false);
		return err_MS_IS_ABORT;
	}
	else
	{	//�鿴˫��RAM�Ķ�д״̬���ڿɶ�д����½������ݴ���
		if(m_bDualRamIsReady)
		{//��״̬����ͬ������״̬��ͬ����Ϊ�����ɽ��ж�״̬

		if (m_bSynLock == READ)
		{
			m_bDualRamIsReady = false;//�л�˫��RAM�Ķ�д״̬����ֹ��β���
			if(PCIPro.fnPciReadMem(BASE_ADDRESS + UPDATE_BACK_ADDRESS_OFFSET,ulFeedbackData))
			{			
				
 				switch (ulFeedbackData&0x0000FFFF)
				{
				case UPDATE_FEEDBACK_INFO_IS_READY://ָ����λ�������ݣ����Խ���ȡ���ݣ�
					{
						unsigned char* pucRemark = new unsigned char[REMARK_LENGTH];
						BufData updateData;
// 						updateData.iCmd = 0;
// 						updateData.iLength = 0;
// 						updateData.iCheckCode = 0;
						int icount;
						if(PCIPro.fnPciReadMem(BASE_ADDRESS + UPDATE_REMARK_ADDRESS_OFFSET,REMARK_LENGTH,pucRemark))
						{
							//��ȡRemark����Ϣ�����ָ��ID��ָ��ȣ�У����
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
						//��ȡ���ݰ��壬���˶�У����
						if(PCIPro.fnPciReadMem(BASE_ADDRESS + UPDATE_DATA_ADDRESS_OFFSET,updateData.iLength,pucData))
						{
							int icheckword = 0;
							for (icount = 0;icount <updateData.iLength;icount++)
							{
								icheckword += *(pucData + icount);
							}
							//У����˶Գ�������մ洢���ݵ��ڴ�
							if((icheckword&0xFF) != updateData.iCheckCode)
							{
								delete[] pucRemark;
								pucRemark = NULL;
								delete[] pucData;
								pucData = NULL;
								m_ucReadRetry++;//��ȡ���ݵ�У�����д���Ҫ����,����������ͱ
								m_bDualRamIsReady = true;
								//m_bSynLock = WRITE;
								return err_MS_Check_Code_Invalid;
							}else
							{
								//У������ȷ��������д���ϴ����У��������ڴ�
								updateData.pucData = pucData;
								if(InterfateRead != LINK.Status)//д��SVector����ʱ��Ӧ��ΪInterfaceRead״̬
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

						//��ɶ����񣬴�ʱ��Ҫ�����е���Ϣ���ڴ��ͷ�
						if(InterfaceWrite != LINK.Status)
						{
							LINK.Status = RAMWrite;//�ͷ��ڴ棬Ҫ����MVector
							if(!(err_Success==LINK.fnFreeMemory(&m_vBufMaster[0])&&err_Success==LINK.fnPopBuffMaster()))
							{
								LINK.Status = Idle;
								m_bDualRamIsReady = true;
								m_ucReadRetry++;//�ͷ��ڴ��������
								return err_MS_Memory_Free_Invalid;
							}
							LINK.Status = Idle;
						}
						m_ucInvalidRetry = 0;
						m_ucErrorRetry = 0;
						m_ucReadRetry = 0;//�����д���Դ���
						m_bSynLock = WRITE;
						return err_Success;
					}
					break;
				case UPDATE_CHECK_WORD_IS_INVALID://У���������Ҫ���·���ָ�
					{
						LINK.m_ucErrorRetry++;
						m_bDualRamIsReady = true;
						m_bSynLock = WRITE;
						return err_PCI_Read_Memory_Invalid;
					}
					break;
				case UPDATE_CMD_IS_INVALID://ָ�������壬��Ҫ���·���ָ�
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
					return err_Success;//��ʱ��ʾû��д��ָ�����ݣ�������Ӳ��æ״̬
					break;
				}
			}else
			{
				if(m_ucReadRetry < READ_TRY)LINK.m_ucReadRetry++;
				else m_bSynLock = READ;//�����Ǹ���
				m_bDualRamIsReady = true;
				return err_PCI_Read_Memory_Invalid;
			}
		} 
		//д״̬
		else
		{
			if(InterfaceWrite != LINK.Status)
			{
				LINK.Status = RAMWrite;//����MVector
				if (0 != LINK.m_vBufMaster.size())
				{	
					m_bDualRamIsReady = false;//��˫��RAMд��������״̬
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
//ģ���Ӳ���������
errorCode CSingleton::fnHardProc(sBufData* bd)
{
	unsigned int uicount = 0;
	unsigned short icheckword = 0;
	//����У����
	for (uicount = 0;uicount < bd->iLength;uicount++)
	{
		icheckword += *(bd->pucData+uicount);
	}
	icheckword &= 0x00FF;
	//��װд���ݵĽṹ��
	DataRemark tmpData;
	tmpData.ulCmdid = bd->iCmd;
	tmpData.usLength = bd->iLength;
	tmpData.usCheckCode = icheckword;
	//����д�����ݰ���Remark��Ϣ���ж���Ϣ
	if(bd->iLength > MAX_DOWNLOAD_LENGTH)
	{	//������ݵ���������һ��д��128�ֽ�����
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
		//�������������������128�ֽڵ�����
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
//�ͷ�bd������Ϣ���ڴ��
errorCode CSingleton::fnFreeMemory(sBufData* bd)
{
	//����freeMemory()������ר���ͷ���������ڴ�
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
//���������������ʹ��
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
	//�ͷŴӶ��м��ڴ�
	std::vector<BufData>::iterator itrs = m_vBufSlave.begin();
	while(itrs != m_vBufSlave.end())
	{
		BufData BD = *itrs;
		if(err_Success!=fnFreeMemory(&BD))return err_MS_Memory_Free_Invalid;
		itrs = m_vBufSlave.erase(itrs);
	}
	//�ͷ������м��ڴ�
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
	
	//ģ�����
	CString csShow,str,strblank,title,strex;
	strblank =_T(" ");
	title = _T("Data:\n");
	unsigned char* puiData = BD->pucData;
	//��ָ��ָ����·������ڴ��У����е�ǰ��Ϣ���������ڵ�λ��
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
	//�ͷŴ��豸�������ݵ��ڴ�
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
				//LINK.m_bSynLock = WRITE;//д״̬
				LINK.m_bAsyLock = false;//����
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
					//����ͱһ����Ϣ��
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
		//��ָ��RAM����д�����ݣ�֪ͨ�ײ�Ӳ��
		//����ط�Ӧ�����һ��״̬��,����û��ƺ�
		if (WRITE == LINK.m_bGreenPathStatus)
		{

		}
		else if(READ == LINK.m_bGreenPathStatus)
		{
			LINK.m_bInterfaceLock = UNLOCKED;//��ɫͨ��ִ����ϣ�������������
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
//20140831������δ��� ��δ���ԣ��ӿڲ���ʱ�������ز��ԡ�
//�ӿڲ�����δ�������ԣ�ԭ�򣬹��̿��ƴ����޷�������ѯ����
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
	//LINK.m_bInterfaceLock = LOCKED;//����Ϊæ״̬����һ���ڿ�ʼʱΪ��ɫͨ��
	LINK.m_bGreenPath = UNLOCKED;
	return err_Success;
}
//���̿��ƽӿڣ�����ʹ��ʱ����Ҫ�Լӵ���
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