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
	do 
	{
		m_bIsBusy = false;
	} while (m_bIsBusy);
	PCI_SET_BUSY;
	RET = PlxPci_DeviceFind(&pKey,SELECT_1ST_DEVICE);
	if (RET != ApiSuccess)
	{
		PCI_SET_IDLE;
		return false;
	}
	RET = PlxPci_DeviceOpen(&pKey,&pDevice); 
	if (RET != ApiSuccess)
	{
		PCI_SET_IDLE;
		return false;
	}	
	PLX_STATUS pStatusa;
	PLX_STATUS pStatusb;
	ULONG data;
	data = PlxPci_PlxRegisterRead(&pDevice,REGISTER_SPACE1_OFFSET, &pStatusa);
	pStatusb = PlxPci_PlxRegisterWrite(&pDevice,REGISTER_SPACE1_OFFSET, data | REGISTER_SPACE_ENABLE_MASK);
	PCI_SET_IDLE;
	return true;
}
//读内存
bool CPciProcess::fnPciReadMem(ULONG offset,unsigned int len,unsigned char* data)
{
	PLX_STATUS RET;
	PCI_SET_BUSY;
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
		PCI_SET_IDLE;
		return false;
	}
	PCI_SET_IDLE;
	return true;
}
bool CPciProcess::fnPciReadMem(ULONG offset,ULONG& data)
{
	volatile PLX_STATUS RET;
	PCI_SET_BUSY;
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
		PCI_SET_IDLE;
		return false;
	}
	PCI_SET_IDLE;
	return true;
}
//写内存
bool CPciProcess::fnPciWriteMem(ULONG offset, unsigned char* data,unsigned int len)
{
	PLX_STATUS RET;
	PCI_SET_BUSY;
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
		PCI_SET_IDLE;
		return false;
	}
	PCI_SET_IDLE;
	return true;
}
bool CPciProcess::fnPciWriteMem(ULONG offset, ULONG data)
{
	PLX_STATUS RET;
	PCI_SET_BUSY;
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
		PCI_SET_IDLE;
		return false;
	}
	PCI_SET_IDLE;
	return true;
}
bool CPciProcess::fnPciWriteMem(ULONG offset, short data)
{
	PLX_STATUS RET;
	PCI_SET_BUSY;
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
		PCI_SET_IDLE;
		return false;
	}
	PCI_SET_IDLE;
	return true;
}
//读寄存器
ULONG CPciProcess::fnPciReadReg(ULONG offset)
{
	ULONG data = 0;
	PLX_STATUS pStatus;
	PCI_SET_BUSY;
	data = PlxPci_PlxRegisterRead(&pDevice,offset, &pStatus);
	PCI_SET_IDLE;
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
	PCI_SET_BUSY;
	pStatus = PlxPci_InterruptDisable(&pDevice,&pPlxIntr);

	RET = PlxPci_DeviceClose(&pDevice);
	PCI_SET_IDLE;	
	if (RET != ApiSuccess)
	{
		PCI_SET_IDLE;
		return false;
	}
	PCI_SET_IDLE;
	return true;
}
//启动中断线程
bool CPciProcess::fnPciStartThread()
{
	PCI_SET_BUSY;
	CWTThread = AfxBeginThread(fnPciIntThread, this);
	PCI_SET_IDLE;
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
		while(port->m_bIsBusy) ;
		port->m_bIsBusy = true;
		//PCI_SET_BUSY;
		
		pStatus = PlxPci_InterruptEnable(&(port->pDevice),&(port->pPlxIntr));

		data = PlxPci_PlxRegisterRead(&(port->pDevice),0x68,&pStatus);
		pStatus = PlxPci_PlxRegisterWrite(&(port->pDevice),0x68,data | 0x8800);
		port->pPlxIntr.LocalToPci = 0x00000001;
		pStatus = PlxPci_NotificationRegisterFor(&(port->pDevice),&(port->pPlxIntr),&(port->pEvent));
		port->m_bIsBusy = false;

		pStatus = PlxPci_NotificationWait(&(port->pDevice),&(port->pEvent),PLX_TIMEOUT_INFINITE);

		switch(pStatus)
		{
		case ApiSuccess: 
			{
				//LINK.m_bAsyLock = true;
				//MessageBox(LINK.m_pCWnd->m_hWnd,0,0,0);
				//Get Button Value 
				ULONG ulErrorLength;
				ULONG ulErrorVeryCode;
				if(!(port->fnPciReadMem(UPDATE_ERROR_LENGTH_ADDRESS_OFFSET,ulErrorLength)
					&&port->fnPciReadMem(UPDATE_ERROR_VERY_CODE_ADDRESS_OFFSET,ulErrorVeryCode)))
				{
					//Send Fatal Error to Windows
				}
				else
				{
					if(0xFF>ulErrorLength)
					{
						unsigned char* ucpErrorValue = new unsigned char[ulErrorLength];
						if(!(port->fnPciReadMem(UPDATE_ERROR_ADDRESS_OFFSET,(int)ulErrorLength,ucpErrorValue)))
						{
							//Send Fatal Error to Windows
						}
						else
						{
							//Send Error Value to Windows
						}
						DELETE_POINT(ucpErrorValue);
					}
					else
					{
						//Send Error Value to Windows
					}
				}

			}		
			break;
		case ApiInvalidAddress:
			//Send Fatal Error to Windows
		case ApiInvalidDeviceInfo:
			//Send Fatal Error to Windows
		default:
			//Send Fatal Error to Windows
			break;
		}
		pStatus = PlxPci_NotificationCancel(&(port->pDevice),&(port->pEvent));
		//PCI_SET_IDLE;
	} while (1);
	return 0;
}

void CPciProcess::fnDelay()
{
	int ipi,ipj,ipk;
	for (ipi = 0;ipi<300;ipi++)
	{
		for (ipj = 0;ipj<100;ipj++)
		{
			for (ipk = 0;ipk<100;ipk++)
			{
			}
		}
	}
}
