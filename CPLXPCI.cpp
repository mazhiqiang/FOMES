#include "stdafx.h"
#include "CPLXPCI.h"
#include "CBufMS.h"

//#include "EX_SDMDlg.h"
#include "mmsystem.h" 
#pragma comment(lib, "winmm.lib")
//构造函数
CPciProcess::CPciProcess()
{
	m_uiVendorId = VENDOR_ID;
	m_uiDeviceId = DEVICE_ID;
	memset(&pKey, PCI_FIELD_IGNORE, sizeof(PLX_DEVICE_KEY));
	memset(&pPlxIntr,0,sizeof(PLX_INTERRUPT));
	memset(&pDevice,0,sizeof(PLX_DEVICE_OBJECT));
	memset(&pEvent,0,sizeof(PLX_NOTIFY_OBJECT));
}

//析构函数
CPciProcess::~CPciProcess()
{

}
//初始化函数
//获得设备，并且配置读写功能
bool CPciProcess::fnPciInit()
{
	PLX_STATUS RET;
	pKey.VendorId = m_uiVendorId;
	pKey.DeviceId = m_uiDeviceId;
	RET = PlxPci_DeviceFind(&pKey,SELECT_1ST_DEVICE);
	if (RET != ApiSuccess)
	{
		//MessageBox("请检查板卡是否异常。 ");
		return false;
	}
	RET = PlxPci_DeviceOpen(&pKey,&pDevice); 
	if (RET != ApiSuccess)
	{
		//MessageBox("出错了！请检查板卡是否异常。 ");
		return false;
	}
	PLX_STATUS pStatusa;
	PLX_STATUS pStatusb;
	ULONG data;
	data = PlxPci_PlxRegisterRead(&pDevice,REGISTER_SPACE1_OFFSET, &pStatusa);
	pStatusb = PlxPci_PlxRegisterWrite(&pDevice,REGISTER_SPACE1_OFFSET, data | REGISTER_SPACE_ENABLE_MASK);
	//return fnPciStartThread();
	return true;
}
//读内存
bool CPciProcess::fnPciReadMem(ULONG offset,unsigned int len,unsigned char* data)
{
	PLX_STATUS RET;
	RET = PlxPci_PciBarSpaceRead(
		&pDevice,
		BAR_3_SPACE_OFFSET,
		offset,
		data,
		len,
		BitSize16,
		TRUE
		);

	if (RET != ApiSuccess)
	{
		return false;
	}
	return true;
}
bool CPciProcess::fnPciReadMem(ULONG offset,ULONG& data)
{
	PLX_STATUS RET;
	RET = PlxPci_PciBarSpaceRead(
		&pDevice,
		BAR_3_SPACE_OFFSET,
		offset,
		&data,
		sizeof(data),
		BitSize16,
		TRUE
		);

	if (RET != ApiSuccess)
	{
		return false;
	}
	return true;
}
//写内存
bool CPciProcess::fnPciWriteMem(ULONG offset, unsigned char* data,unsigned int len)
{
	PLX_STATUS RET;

	RET = PlxPci_PciBarSpaceWrite(
		&pDevice,
		BAR_3_SPACE_OFFSET,
		offset,
		data,
		len,
		BitSize16,
		TRUE
		);
	if (RET!=ApiSuccess)
	{
		return false;
	}
	return true;
}
bool CPciProcess::fnPciWriteMem(ULONG offset, ULONG data)
{
	PLX_STATUS RET;
	RET = PlxPci_PciBarSpaceWrite(
		&pDevice,
		BAR_3_SPACE_OFFSET,
		offset,
		&data,
		sizeof(data),
		BitSize16,
		TRUE
		);
	if (RET!=ApiSuccess)
	{
		return false;
	}
	return true;
}
bool CPciProcess::fnPciWriteMem(ULONG offset, short data)
{
	PLX_STATUS RET;
	RET = PlxPci_PciBarSpaceWrite(
		&pDevice,
		BAR_3_SPACE_OFFSET,
		offset,
		&data,
		sizeof(data),
		BitSize16,
		TRUE
		);
	if (RET!=ApiSuccess)
	{
		return false;
	}
	return true;
}
//读寄存器
ULONG CPciProcess::fnPciReadReg(ULONG offset)
{
	ULONG data = 0;
	PLX_STATUS pStatus;

	data = PlxPci_PlxRegisterRead(&pDevice,offset, &pStatus);
	return data;
}
//写寄存器
void CPciProcess::fnPciWriteReg(ULONG offset, unsigned char* data)
{
	
}
//关闭PCI设备
bool CPciProcess::fnPciClose()
{
	PLX_STATUS RET;

	PLX_STATUS pStatus;
	
	pStatus = PlxPci_InterruptDisable(&pDevice,&pPlxIntr);

	RET = PlxPci_DeviceClose(&pDevice);
		
	if (RET != ApiSuccess)
	{
		//AfxMessageBox("设备关闭遇到问题！");
		return false;
	}
	return true;
}
//启动中断线程
bool CPciProcess::fnPciStartThread()
{
	CWTThread = AfxBeginThread(fnPciIntThread, this);
	return true;
}
//初始化线程
UINT CPciProcess::fnPciIntThread(LPVOID pParam)
{
	PLX_STATUS pStatus;
	ULONG data;

//	CCommUtl *port = (CCommUtl*)pParam;
	do 
	{
		CPciProcess *port = (CPciProcess*)pParam;


		pStatus = PlxPci_InterruptEnable(&(port->pDevice),&(port->pPlxIntr));

		data = PlxPci_PlxRegisterRead(&(port->pDevice),0x68,&pStatus);
		pStatus = PlxPci_PlxRegisterWrite(&(port->pDevice),0x68,data | 0x8800);
		port->pPlxIntr.LocalToPci = 0x00000001;
		pStatus = PlxPci_NotificationRegisterFor(&(port->pDevice),&(port->pPlxIntr),&(port->pEvent));
		pStatus = PlxPci_NotificationWait(&(port->pDevice),&(port->pEvent),PLX_TIMEOUT_INFINITE);

		switch(pStatus) {
		case ApiSuccess: 
			LINK.m_bAsyClock = true;
			AfxMessageBox("Trigger!");
			break;
		case ApiInvalidAddress:
			AfxMessageBox("InvalidAddress!");
		case ApiInvalidDeviceInfo:
			AfxMessageBox("ApiInvalidDeviceInfo!");
		default:
			//AfxMessageBox("Wrong!");
			break;
		}
		pStatus = PlxPci_NotificationCancel(&(port->pDevice),&(port->pEvent));
	} while (1);
	
	
	return 0;
}

