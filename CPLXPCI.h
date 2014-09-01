#ifndef __CPLXPCI_H__
#define __CPLXPCI_H__
#include "./Plx/PlxSdk/Include/PlxApi.h"
#include <vector>
#include <iostream>


#define VENDOR_ID 0x10B5
#define DEVICE_ID 0x9054
#define REGISTER_SPACE0_OFFSET 0x04
#define REGISTER_SPACE1_OFFSET 0xF4
#define REGISTER_SPACE_ENABLE_MASK 0x00000001
#define REGISTER_SPACE_DISABLE_MASK 0x00000000
#define BAR_2_SPACE_OFFSET 2
#define BAR_3_SPACE_OFFSET 3
#define SELECT_1ST_DEVICE 0
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
	CPciProcess();//���캯�����ڹ��������Ӧ��ʵ��Ӳ������ֵ���趨
	~CPciProcess();
	
	bool fnPciInit();//PCI��ʼ������
	bool fnPciReadMem(ULONG offset,unsigned int len,unsigned char* data );//���ڴ�
	bool fnPciReadMem(ULONG offset,ULONG& data);
	bool fnPciWriteMem(ULONG offset, unsigned char* data,unsigned int len);//д�ڴ�
	bool fnPciWriteMem(ULONG offset, ULONG data);
	bool fnPciWriteMem(ULONG offset, short data);
	ULONG fnPciReadReg(ULONG offset);//���Ĵ���
	void fnPciWriteReg(ULONG offset, unsigned char* data);//д�ڴ�
	bool fnPciClose();//�ر�PCI�豸
	bool fnPciStartThread();//�����ж��߳�
	static UINT fnPciIntThread(LPVOID pParam);//��ʼ���߳�
	CWinThread* CWTThread;	
};
#endif