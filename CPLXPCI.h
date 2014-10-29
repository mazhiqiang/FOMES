#ifndef __CPLXPCI_H__
#define __CPLXPCI_H__
#include <PCI/PlxApi.h>
#include <vector>
#include <iostream>
#pragma comment(lib, "PlxApi.lib")


#define VENDOR_ID 0x10B5
#define DEVICE_ID 0x9054
#define REGISTER_SPACE0_OFFSET 0x04
#define REGISTER_SPACE1_OFFSET 0xF4
#define REGISTER_SPACE_ENABLE_MASK 0x00000001
#define REGISTER_SPACE_DISABLE_MASK 0x00000000
#define BAR_2_SPACE_OFFSET 2
#define BAR_3_SPACE_OFFSET 3
#define SELECT_1ST_DEVICE 0

#define PCI_SET_BUSY while(m_bIsBusy) ;m_bIsBusy = true
#define PCI_SET_IDLE fnDelay();m_bIsBusy = false
using namespace std;


class CPciProcess
{
private:
	PLX_DEVICE_KEY pKey;
	PLX_DEVICE_OBJECT pDevice;
	PLX_INTERRUPT pPlxIntr;
	PLX_NOTIFY_OBJECT pEvent;
	unsigned int m_uiVendorId;
	unsigned int m_uiDeviceId;
	
public:
	CPciProcess();//构造函数，在构造过程中应该实现硬件配置值的设定
	~CPciProcess();
	bool m_bIsBusy;
	bool fnPciInit();//PCI初始化配置
	bool fnPciReadMem(ULONG offset,unsigned int len,unsigned char* data );//读内存
	bool fnPciReadMem(ULONG offset,ULONG& data);
	bool fnPciWriteMem(ULONG offset, unsigned char* data,unsigned int len);//写内存
	bool fnPciWriteMem(ULONG offset, ULONG data);
	bool fnPciWriteMem(ULONG offset, short data);
	ULONG fnPciReadReg(ULONG offset);//读寄存器
	void fnPciWriteReg(ULONG offset, unsigned char* data);//写内存
	bool fnPciClose();//关闭PCI设备
	bool fnPciStartThread();//启动中断线程
	static void fnDelay();
	static UINT fnPciIntThread(LPVOID pParam);//初始化线程
	//UINT fnPciIntThread(LPVOID pParam);//初始化线程
	CWinThread* CWTThread;	
};
#endif