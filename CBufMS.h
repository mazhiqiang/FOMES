#ifndef __CBUFMS_H__
#define __CBUFMS_H__

#include <vector>
#include <iostream>
#include "CPLXPCI.h"

#define LINK CSingleton::GetInstance()

#define WM_MYMESSAGE WM_USER+100

#define BASE_ADDRESS 0
#define UPDATE_INTERACTIVE_ADDRESS_OFFSET 0x1C00
#define UPDATE_WRITE_IDLE_ADDRESS_OFFSET (UPDATE_INTERACTIVE_ADDRESS_OFFSET + 0x00)
#define UPDATE_BACK_ADDRESS_OFFSET 0x1FFC
#define UPDATE_REMARK_ADDRESS_OFFSET 0x17F8
#define UPDATE_DATA_ADDRESS_OFFSET 0x1000

#define UPDATE_ERROR_ADDRESS_OFFSET 0x1800
#define UPDATE_ERROR_REMARK_ADDRESS_OFFSET 0x1BF8 
#define UPDATE_ERROR_LENGTH_ADDRESS_OFFSET 0x1BFC
#define UPDATE_ERROR_VERY_CODE_ADDRESS_OFFSET 0x1BFE

#define UPDATE_ERROR_RETURN_VALUE 0x0101
#define UPDATE_ERROR_RETURN_STATUS 0x0202

#define UPDATE_FEEDBACK_INFO_IS_READY 0x1212
#define UPDATE_CHECK_WORD_IS_INVALID 0x2323
#define UPDATE_WRITE_IS_IDLE 0xABCD
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

#define REMARK_LENGTH (sizeof(DataRemark))
#define REMARK_CHECK_WORD_LENGTH (sizeof(unsigned short))
#define REMARK_CMD_LENGTH (sizeof(unsigned short))
#define REMARK_CMD_ID_LENGTH (sizeof(ULONG))

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
//#define LogComd 1
#define DELETE_POINT(x) do{delete[] x;x = NULL;}while(0)
#define FATAL_ERROR
#define REWRITE_TIME ((ULONG)300000)
#define REWRITE_ZERO 0
#define LOW16_MASK ((ULONG)(0x0000FFFF))
#define LOW8_MASK ((unsigned char)(0x00FF))

#define ADVANCE_CMD 1
#define COMMON_CMD 0

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
//链路层所管理内存数据的信息的结构体
//uc* puData:	数据所在内存的地址
//i iCmd	:	生成的命令ID
//i iLengthe:	数据长度(字节)
//i iCheckCode:	校验码(未实现)
typedef struct sBufData
{
	unsigned char* pucData;
	int iCmd;
	int iLength;
	int iCheckCode;
}BufData;

typedef struct sDataRemark
{
	ULONG ulCmdid;
	unsigned short usLength;
	unsigned short usCheckCode;
}DataRemark;
//链路层实现类，单例模式
class CSingleton
{
private:
	CSingleton()   //构造函数是私有的
	{
		m_iPeriod = 2;
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
		StatusMaster = Idle;
		StatusSlave = Idle;
		m_bAdvance = UNADVANCE;
		P2DStatus = P2D_WRITE;
		m_bMainAbort = true;
		m_bTreadIsRunning = false;
	}
	~CSingleton();
	CSingleton(const CSingleton &);
	CSingleton & operator = (const CSingleton &);
	//Master Buffer:	来自过程控制的数据信息集合，使用std::Vector存储
	std::vector<BufData> m_vBufMaster;
	//Slave Buffer:		来自硬件设备反馈的数据信息集合，使用std::Vector存储
	std::vector<BufData> m_vBufSlave;
	//Advance Buffer:	来自过程控制的优先级最高的信息集合，使用std::Vector存储
	std::vector<BufData> m_vBufAdvance;

	//设定轮询MasterBuffer的周期及精度(ms)
	int m_iPeriod;
	int m_iPrecision;
	//当前操作的数据块的信息
	BufData m_sCurMaster;
	BufData m_sCurSlave;
	
	//轮询定时器更新的次数
	int m_TimerSlaveCount;
	//窗体上显示轮询更新次数的空间ID
	int m_TextSlaveCount;
	//数据传输的命令ID
	unsigned int m_uicmdid;
	CPciProcess PCIPro;
	bool m_bIsInited;
	PCI2DPS_Status P2DStatus;


	

	unsigned char m_ucErrorRetry;
	unsigned char m_ucInvalidRetry;
	ULONG m_ulReadRetry;
	bool m_bMainAbort;
	bool m_bTreadIsRunning;
	errorCode fnDelay();
	bool fnThreadDestroy();
	bool fnThreadAbort();
	static UINT fnMain(LPVOID);
	bool fnThreadStart();
	bool fnIsAdvanceCMD(unsigned char*);

	int fnGetVeryCode(unsigned char*,int);
public:
	//窗体指针
	CWnd* m_pCWnd;
	bool m_bSynLock;	//only debug,clear them later on 
	bool m_bAsyLock;	//only debug,clear them later on 
	bool m_bInterfaceLock;
	bool m_bGreenPath;
	bool m_bGreenPathStatus;
	LinkState StatusMaster;
	LinkState StatusSlave;
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

	//winform information
	void fnSetCWnd(CWnd* cwnd);
	CWnd* fnGetCWnd();

	//memory
	errorCode fnFreeMemory(sBufData* bd);//20140820
	//errorCode fnPopBuffMaster();	//only debug,clear them later on 
	//errorCode fnPopBuffAdvance();	//only debug,clear them later on 
	errorCode fnBuffPop(const UINT ,std::vector<BufData>*);
	errorCode fnFreeAllMemoryAndData();
	//procedure 
	errorCode fnBuffRoute(unsigned char* m_ControlComd,int len,int* ComdID);
	errorCode fnBuffTrans();
	errorCode fnHardProc(sBufData* bd, unsigned char);
	errorCode fnHardProcAdvance(sBufData* bd);
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
	errorCode fnWriteAdvance(void);
	errorCode fnSendToBuffer(BYTE *,int,int *);
	errorCode fnBuffPull(int,BYTE *,int ,int );

	errorCode fnSendMessageToWinform();
};


#endif