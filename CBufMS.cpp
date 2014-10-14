#include "stdafx.h"
#include "CBufMS.h"
#include "..\SMTLogManage\SMTLogManage\SMTLogManage.h" //by mzq 2014.10.13
#include "TypeConvert.h"//by mzq 2014.10.13

//#include "EX_SDMDlg.h"
#include "mmsystem.h" 
#pragma comment(lib, "winmm.lib")
#define  ModifiedByMzq

//�趨��������������״̬

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

errorCode CSingleton::fnBuffRoute(unsigned char* m_ControlComd,int len,int* ComdID,unsigned int uioffset)
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
	m_sCurMaster.uiOffset = uioffset;
	if(RAMWrite != Status)//LINK��δ����MVector
	{
		Status = InterfaceWrite;
		if(UNADVANCE == m_bAdvance)
			m_vBufMaster.push_back(m_sCurMaster);
		else if(ADVANCE == m_bAdvance)
			m_vBufAdvance.push_back(m_sCurMaster);
		*ComdID = m_uicmdid;//����CMDID
		m_uicmdid++;
		Status = Idle;	//״̬�л�����
		return err_Success;
	}else
	{
		delete[] buf;
		buf = NULL;
		return err_MS_Push_Invalid;
	}
	//�ͷ��ڴ�ʱ����Ҫ�����ͷŵ��ڴ��С��2014.08.15
}

errorCode CSingleton::fnRead(void)
{
	unsigned char* pucRemark = new unsigned char[REMARK_LENGTH];
	BufData updateData;
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
		DELETE_POINT(pucRemark);
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
			DELETE_POINT(pucRemark);
			DELETE_POINT(pucData);
			m_ucReadRetry++;//��ȡ���ݵ�У�����д���Ҫ����,����������ͱ
			m_bSynLock = WRITE;
			return err_MS_Check_Code_Invalid;
		}else
		{
			//У������ȷ��������д���ϴ����У��������ڴ�
			updateData.pucData = pucData;
			if(InterfaceRead != Status)//д��SVector����ʱ��Ӧ��ΪInterfaceRead״̬
			{
				Status = RAMRead;
#ifdef ModifiedByMzq
				//����ѭ������m_vBufSlave
				if (m_vBufSlave.size()>BUF_SLAVE_SIZE)
				{
					fnBuffPop(m_vBufSlave[0].iCmd,&m_vBufSlave);
				}
#endif
				m_vBufSlave.push_back(updateData);
				Status = Idle;
			}else return err_PCI_Read_Memory_Invalid;
		}
	}else  
	{
		DELETE_POINT(pucRemark);
		DELETE_POINT(pucData);
		m_ucReadRetry++;
		return	err_PCI_Read_Memory_Invalid;
	}
						
	DELETE_POINT(pucRemark);
	//��ɶ����񣬴�ʱ��Ҫ�����е���Ϣ���ڴ��ͷ�
	if(InterfaceWrite != Status)
	{
		Status = RAMWrite;//�ͷ��ڴ棬Ҫ����MVector
		if(!(err_Success==fnBuffPop(updateData.iCmd,&m_vBufMaster)))
		{
			Status = Idle;
			m_ucReadRetry++;//�ͷ��ڴ��������
			return err_MS_Memory_Free_Invalid;
		}
		Status = Idle;
	}
	m_ucInvalidRetry = 0;
	m_ucErrorRetry = 0;
	m_ucReadRetry = 0;//�����д���Դ���
	m_bSynLock = WRITE;
	return err_Success;
}

errorCode CSingleton::fnWrite(void)
{
	if(InterfaceWrite != Status)
	{
		Status = RAMWrite;//����MVector
		if (0 != m_vBufMaster.size()||0 != m_vBufAdvance.size())
		{	
			if(0 != m_vBufMaster.size()&&err_Success==fnHardProc(&m_vBufMaster[BUFFER_ZERO])) 
			{
				Status = Idle;
				m_bSynLock = READ;
				m_ucInvalidRetry = 0;
				m_ucErrorRetry = 0;
#if LogComd
				string Logcontent = "PCI Route {ComdID =" ;
				Logcontent += TypeConvert <short, string>(m_vBufMaster[BUFFER_ZERO].iCmd);
				Logcontent += "}";
				LogManage& CurLog=LogManage::GetInit();//��ȡ��־��ʵ����ֱ���������ǰ�ֶ�������
				WriteLogMessage(Logcontent ,SMT_WARN,LOG_FDINFO);
#endif
				return err_Success;
			}else
			{					
				Status = Idle;
				m_ucInvalidRetry++;
				return err_MS_Memory_Trans_Invalid;
			}					
		}else 
		{
			Status = Idle;
			return err_PCI_Write_Memory_Invalid;
		}
	}
	return err_Success;
}
errorCode CSingleton::fnBuffTrans()//������ɫͨ������£�����������������ݹܵ�������
{
#if 0	
	if (0)
	{
		return err_MS_Memory_Trans_Invalid;
	}
	if (m_ucInvalidRetry >= UPDATE_ERROR_RETRY 
		|| m_ucErrorRetry >= UPDATE_INVALID_RETRY||m_ucReadRetry >= UPDATE_ERROR_RETRY)
	{

		//����ͱ��ȥ,sendmessage
		//Ŀǰ����Ľ���취������ڴ棬�����Ĵ�������Ӳ��
		if(err_Success == fnFreeAllMemoryAndData())
		{
			m_bSynLock = WRITE;//д״̬
			//m_bAsyClock = false;//����
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
#endif
	//������Ӳ��д���ݵ�״̬
	ULONG ulFeedbackData;//��ȡ˫��RAM����
	ULONG ulFeedbackInterruptFlag;
	if(m_bSynLock == WRITE)
	{
		fnWrite();  // yu
		m_vBufMaster;
	}
	else{
		
		//if(PCIPro.fnPciReadMem(BASE_ADDRESS + UPDATE_INTERRUPT_FLAG_ADDRESS_OFFSET,ulFeedbackInterruptFlag))
		//{UPDATE_IS_RETURNED == (ulFeedbackInterruptFlag&0xFFFF)&&
			if(PCIPro.fnPciReadMem(BASE_ADDRESS + UPDATE_BACK_ADDRESS_OFFSET,ulFeedbackData))
			{		
				switch (ulFeedbackData&0x0000FFFF)
				{
				case UPDATE_FEEDBACK_INFO_IS_READY://ָ����λ�������ݣ����Խ���ȡ���ݣ�
					fnRead();  
					m_ucReadRetry = 0;
					break;
				case UPDATE_CHECK_WORD_IS_INVALID://У���������Ҫ���·���ָ�
					{
						m_ucErrorRetry++;
						m_ucReadRetry = 0;
						return err_PCI_Read_Memory_Invalid;
					}
					break;
				case UPDATE_CMD_IS_INVALID://ָ�������壬��Ҫ���·���ָ�
					{
						m_ucInvalidRetry++;
						m_ucReadRetry = 0;
						return err_PCI_Read_Memory_Invalid;
					}
					break;
				default:
					m_ucInvalidRetry = 0;
					m_ucErrorRetry = 0;
					m_ucReadRetry++;
					if(m_ucReadRetry > 100)
					{
						m_bSynLock = WRITE;
						m_ucReadRetry = 0;
					}	
					return err_Success;//��ʱ��ʾû��д��ָ�����ݣ�������Ӳ��æ״̬
					break;
				}
			}
		//}
		else 
		{
			return err_MS_Memory_Route_Invalid;
		}
	}
	return err_Success;
}
//ģ���Ӳ���������
errorCode CSingleton::fnHardProc(sBufData* bd)
{
	int uicount = 0;
	unsigned short icheckword = 0;
	//����У����
	for (uicount = 0;uicount < bd->iLength;uicount++)
	{
		icheckword += *(bd->pucData+uicount);
	}
	icheckword &= 0x00FF;
	bd->iCheckCode = icheckword;
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
			&&PCIPro.fnPciWriteMem(BASE_ADDRESS + DOWNLOAD_INFORM_ADDRESS_OFFSET,(ULONG)DOWNLOAD_IS_READY)))
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
/*				(ULONG)((ULONG)((bd->iCmd<<16)+bd->iCmd))*/
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
	DELETE_POINT(puiTestData);

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
	return err_Success;
}

errorCode CSingleton::fnPopBuffAdvance()
{
	if (!m_vBufAdvance.size())
	{
		return err_MS_Data_Pop_Invalid;
	}
	vector<BufData>::iterator itr = m_vBufAdvance.begin();
	m_vBufAdvance.erase(itr);
	return err_Success;
}
errorCode CSingleton::fnFreeAllMemoryAndData()//Without Too Much Thinking 20141003
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
	//�ͷŸ����ȼ����м��ڴ�
	std::vector<BufData>::iterator itra = m_vBufAdvance.begin();
	while(itra != m_vBufAdvance.end())
	{
		BufData BD = *itra;
		if(err_Success!=fnFreeMemory(&BD))return err_MS_Memory_Free_Invalid;
		itra = m_vBufAdvance.erase(itra);
	}
	return err_Success;
}
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
			Status = Idle;
			return err_Success;
		}
		itrm++;
		//return err_MS_Memory_Free_Invalid;
	}

	return err_MS_Memory_Free_Invalid;
}
errorCode CSingleton::fnBuffPull2WinForm(const UINT uicmdid,BufData* BD)
{
	if(RAMRead != Status)
	{
		Status = InterfaceRead;
		std::vector<BufData>::iterator itr = m_vBufSlave.begin();
		while(itr != m_vBufSlave.end())
		{
			if (uicmdid == itr->iCmd)
			{
				*BD = *itr;
				itr = m_vBufSlave.erase(itr);
				Status = Idle;
				return err_Success;
			}
			itr++;
		}
		Status = Idle;
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
	int uicount;
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

volatile static long int x = 0;

void PASCAL CallBackFunc(UINT wTimerID, UINT msg,DWORD dwUser,DWORD dwl,DWORD dw2) 
{
	//LINK.fnSetWindowText(LINK.fnGetCWnd(),LINK.fnGetTimerSlaveCount(),LINK.fnGetTextTimerSlaveID());


	if(true)
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
	else{
	}
}
bool CSingleton::InitTimerCheckSlave()
{
	::timeSetEvent (fnGetTimerPeriodms(), fnGetTimerPrecisionms(),
		CallBackFunc,NULL,TIME_PERIODIC); 
	fnEnableIntterupt();
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
	UINT uiCountMS = 0;
	int iSleepTimeMS = 50;
	while(1)
	{
	if(RAMRead != Status)
	{
		Status = InterfaceRead;
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
				Status = Idle;
				return err_Success;
			}
			itr++;
		}
		Status = Idle;
		uiCountMS++;
		Sleep(iSleepTimeMS);
		if ((int)(uiCountMS*iSleepTimeMS) > Waittime)
			return err_MS_Pull_Invalid;
	}else return err_MS_Pull_Invalid;
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
	//m_bInterfaceLock = LOCKED;//����Ϊæ״̬����һ���ڿ�ʼʱΪ��ɫͨ��
	m_bGreenPath = UNLOCKED;
	return err_Success;
}
//���̿��ƽӿڣ�����ʹ��ʱ����Ҫ�Լӵ���
errorCode CSingleton::fnSendToBuffer(BYTE *m_ControlComd,int len,int *ComdID)
{
// 	if(m_bInterfaceLock == UNLOCKED)
// 	{
// 		m_bInterfaceLock = LOCKED;
	
	if(false)
	{
		//�����ȼ������Σ�Ŀǰδ����
	}
	else
	{
		//�����ȼ������Σ�Ŀǰ������
		if(err_Success == fnInit()&&err_Success == fnBuffRoute(m_ControlComd,len,ComdID,0))
		{
			//m_bInterfaceLock = UNLOCKED;
			return err_Success;
		}else 
		{
			//m_bInterfaceLock = UNLOCKED;
			return err_MS_IS_ABORT;
		}
	}
}
errorCode CSingleton::fnForce2WriteStatus()
{
	if((0 != m_vBufMaster.size()&&err_Success==fnFreeMemory(&m_vBufMaster[0])&&err_Success==fnPopBuffMaster()))m_bSynLock = WRITE;//CAUTION: EXECUTE WITHOUT TOO MUCH THINGKING
	return err_Success;
}
errorCode CSingleton::fnReset()
{
	if(err_Success == fnFreeAllMemoryAndData())
	return err_Success;
	else return err_MS_IS_ABORT;
}
/*
ProcErr ReceiveInfoBuffer(int ComdID,BYTE *m_FeedBackInfo,int len,int Waittime)
{
// 	if(m_bInterfaceLock == UNLOCKED)
// 	{
// 		m_bInterfaceLock = LOCKED;
		if(err_Success == fnInit()&&err_Success == fnBuffPull(ComdID,m_FeedBackInfo,len))
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