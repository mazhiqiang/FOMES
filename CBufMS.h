#ifndef __CBUFMS_H__
#define __CBUFMS_H__

#include <vector>
#include <iostream>
#include "CPLXPCI.h"

#define LINK CSingleton::GetInstance()
#define WM_MYMESSAGE WM_USER+100

#define BASE_ADDRESS 0
#define UPDATE_BACK_ADDRESS_OFFSET 0x1FFC
#define UPDATE_REMARK_ADDRESS_OFFSET 0x17F8
#define UPDATE_DATA_ADDRESS_OFFSET 0x1000
#define UPDATE_FEEDBACK_INFO_IS_READY 0x1212
#define UPDATE_CHECK_WORD_IS_INVALID 0x2323
#define UPDATE_CMD_IS_INVALID 0x3434
#define UPDATE_ERROR_RETRY 3
#define UPDATE_INVALID_RETRY 3
#define UPDATE_IS_RETURNED ((UINT)0x00005555)
#define UPDATE_IS_UNRETURNED ((UINT)0x0000AAAA)

#define UPDATE_INTERRUPT_FLAG_ADDRESS_OFFSET 0x2000

#define DOWNLOAD_IS_READY 0x54540000
#define DOWNLOAD_INFORM_SIZE 4
#define DOWNLOAD_DATA_ADDRESS_OFFSET 0
//#define DOWNLOAD_BACK_ADDRESS_OFFSET 0x1FFC
#define DOWNLOAD_REMARK_ADDRESS_OFFSET 0x0FF8
#define DOWNLOAD_INFORM_ADDRESS_OFFSET 0x1FFC

#define REMARK_LENGTH sizeof(DataRemark)
#define REMARK_CHECK_WORD_LENGTH sizeof(unsigned short)
#define REMARK_CMD_LENGTH sizeof(unsigned short)
#define REMARK_CMD_ID_LENGTH sizeof(ULONG)

#define UNLOCKED true
#define LOCKED false

#define READ_TRY 3
#define READ true
#define WRITE false

#define MAX_DOWNLOAD_LENGTH 128
#define BUFFER_ZERO 0
#define BUF_SLAVE_SIZE 2048
#define UNADVANCE true
#define ADVANCE false

#define DELETE_POINT(x) do{delete[] x;x = NULL;}while(0)
#define LogComd 1
using namespace std;
enum PCI2DPS_Status
{
	P2D_WRITE,
	P2D_READ,
	P2D_EMERGENCY,
	P2D_BUSY
};
enum errorCode{
	err_Success,
	err_PCI_Init_Failed,
	err_PCI_Intterupt_Invalid,
	err_PCI_Release_Invalid,
	err_PCI_Write_Memory_Invalid,
	err_PCI_Read_Memory_Invalid,
	err_MS_Check_Code_Invalid,
	err_MS_Memory_Route_Invalid,
	err_MS_Hardware_Communication_Invalid,
	err_MS_Memory_Trans_Invalid,
	err_MS_Memory_Free_Invalid,
	err_MS_Data_Get_Invalid,
	err_MS_Data_Pop_Invalid,
	err_MS_Push_Invalid,
	err_MS_Pull_Invalid,
	err_MS_IS_ABORT
};//2014.08.26
enum LinkState
{
	InterfaceWrite,
	InterfaceRead,
	RAMWrite,
	RAMRead,
	Idle
};
//��·���������ڴ����ݵ���Ϣ�Ľṹ��
//uc* puData:	���������ڴ�ĵ�ַ
//i iCmd	:	���ɵ�����ID
//i iLengthe:	���ݳ���(�ֽ�)
//i iCheckCode:	У����(δʵ��)
typedef struct sBufData
{
	unsigned char* pucData;
	int iCmd;
	int iLength;
	int iCheckCode;
	ULONG uiOffset;
}BufData;

typedef struct sDataRemark
{
	ULONG ulCmdid;
	unsigned short usLength;
	unsigned short usCheckCode;
}DataRemark;
//��·��ʵ���࣬����ģʽ
class CSingleton
{
private:
	CSingleton()   //���캯����˽�е�
	{
		m_iPeriod = 10;
		m_iPrecision = 1;
		m_TimerSlaveCount = 0;
		m_uicmdid = 0;
		m_bIsInited = false;
		m_bSynLock = WRITE;
		m_bAsyLock = false;
		m_ucInvalidRetry = 0;
		m_ucErrorRetry = 0;
		m_bInterfaceLock = UNLOCKED;
		m_bGreenPath = LOCKED;
		m_bGreenPathStatus = WRITE;
		Status = Idle;
		m_bAdvance = UNADVANCE;
		P2DStatus = P2D_WRITE;

	}
	CSingleton(const CSingleton &);
	CSingleton & operator = (const CSingleton &);
	//Master Buffer:	���Թ��̿��Ƶ�������Ϣ���ϣ�ʹ��std::Vector�洢
	std::vector<BufData> m_vBufMaster;
	//Slave Buffer:		����Ӳ���豸������������Ϣ���ϣ�ʹ��std::Vector�洢
	std::vector<BufData> m_vBufSlave;
	//Advance Buffer:	���Թ��̿��Ƶ����ȼ���ߵ���Ϣ���ϣ�ʹ��std::Vector�洢
	std::vector<BufData> m_vBufAdvance;

	//�趨��ѯMasterBuffer�����ڼ�����(ms)
	int m_iPeriod;
	int m_iPrecision;
	//��ǰ���������ݿ����Ϣ
	BufData m_sCurMaster;
	BufData m_sCurSlave;
	//����ָ��
	CWnd* m_pCWnd;
	//��ѯ��ʱ�����µĴ���
	int m_TimerSlaveCount;
	//��������ʾ��ѯ���´����Ŀռ�ID
	int m_TextSlaveCount;
	//���ݴ��������ID
	unsigned int m_uicmdid;
	CPciProcess PCIPro;
	bool m_bIsInited;
	PCI2DPS_Status P2DStatus;


	

	unsigned char m_ucErrorRetry;
	unsigned char m_ucInvalidRetry;
	unsigned char m_ucReadRetry;
public:
	
	bool m_bSynLock;	//only debug,clear them later on 
	bool m_bAsyLock;	//only debug,clear them later on 
	bool m_bInterfaceLock;
	bool m_bGreenPath;
	bool m_bGreenPathStatus;
	LinkState Status;
	bool m_bAdvance;
	//static BufData sBufM;
	static CSingleton & GetInstance()
	{
		static CSingleton instance;   
		return instance;
	}
	//hardware
	errorCode fnInit();
	errorCode fnReset();
	errorCode fnReleasePCI();
	//status
	//bool fnGetLockM();
	//bool fnSetLockM(bool tmp);
	//bool fnSetLockS(bool tmp);
	//bool fnGetLockS();
	bool fnSetWindowText(CWnd* cwnd,int iconten,unsigned int iposition);
	bool fnSetWindowText(CWnd* cwnd,CString iconten,unsigned int iposition);
	int fnGetTimerPeriodms();
	void fnSetTimerPeriodms(int itimerperiod);
	int fnGetTimerPrecisionms();
	void fnSetTimerPrecisionms(int itimerprecisionms);
	//winform information
	void fnSetCWnd(CWnd* cwnd);
	CWnd* fnGetCWnd();
	//only debug,clear them later on 
	errorCode fnBuffPull2WinForm(const UINT uicmdid,BufData* BD);
	CString fnExData(BufData* BD);
	int fnGetTimerSlaveCount();
	void fnSetTimerSlaveCount(int icount);
	void fnSetTextTimerSlaveID(int idc);
	int fnGetTextTimerSlaveID();
	//memory
	errorCode fnFreeMemory(sBufData* bd);//20140820
	errorCode fnPopBuffMaster();	//only debug,clear them later on 
	errorCode fnPopBuffAdvance();	//only debug,clear them later on 
	errorCode fnBuffPop(const UINT ,std::vector<BufData>*);
	errorCode fnFreeAllMemoryAndData();
	//procedure 
	errorCode fnBuffRoute(unsigned char* m_ControlComd,int len,int* ComdID,unsigned int uioffset);
	errorCode fnBuffTrans();
	errorCode fnHardProc(sBufData* bd);
	bool InitTimerCheckSlave();
	//remark only debug,clear them later on 
	void fnEnableIntterupt();
	errorCode fnFakeAbortTimer();
	errorCode fnFakeRestartTimer();
	errorCode fnManualIntterupt();	//only debug,clear them later on 
	errorCode fnForce2WriteStatus();
	//main process and interface 
	errorCode fnRead(void);
	errorCode fnWrite(void);
	errorCode fnSendToBuffer(BYTE *,int,int *);
	errorCode fnBuffPull(int,BYTE *,int ,int );
};


#endif