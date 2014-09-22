
// EX_SDMDlg.cpp : ʵ���ļ�
//

#include "stdafx.h"
#include "EX_SDM.h"
#include "EX_SDMDlg.h"
#include "CBufMS.h"
#include "afxdialogex.h"
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include "mmsystem.h" 
#pragma comment(lib, "winmm.lib")
#ifdef _DEBUG
#define new DEBUG_NEW
#endif
//#define WM_MYMESSAGE WM_USER+100

// ����Ӧ�ó��򡰹��ڡ��˵���� CAboutDlg �Ի���

#include "mmsystem.h" 
#pragma comment(lib, "winmm.lib")
errorCode statusCode;
class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// �Ի�������
	enum { IDD = IDD_ABOUTBOX };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV ֧��

// ʵ��
protected:
	DECLARE_MESSAGE_MAP()
//	afx_msg LRESULT OnMymessage(WPARAM wParam, LPARAM lParam);
//	afx_msg LRESULT OnMymessage(WPARAM wParam, LPARAM lParam);
public:
//	afx_msg void OnTimer(UINT_PTR nIDEvent);
//	afx_msg void OnClose();
};

CAboutDlg::CAboutDlg() : CDialogEx(CAboutDlg::IDD)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)

//	ON_MESSAGE(MYMESSAGE, &CAboutDlg::OnMymessage)
//	ON_REGISTERED_MESSAGE(WM_MYMESSAGE, &CAboutDlg::OnMymessage)
//	ON_WM_TIMER()
//	ON_WM_CLOSE()
END_MESSAGE_MAP()


// CEX_SDMDlg �Ի���




CEX_SDMDlg::CEX_SDMDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CEX_SDMDlg::IDD, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
	m_Data1 = 0;
	m_Data2 = 0;
	//  m_Data = 0;
	m_Data3 = 0;
	m_CmdID = 0;
	m_Data4 = 0;
	m_Offset = 0;
	m_str0xCMD = _T("");
	m_CMD_Len = 0;
}

void CEX_SDMDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_DATA1, m_Data1);
	//  DDV_MinMaxUInt(pDX, m_Data1, 0, 65535);
	DDX_Text(pDX, IDC_DATA2, m_Data2);
	DDV_MinMaxUInt(pDX, m_Data2, 0, 65535);
	//  DDX_Text(pDX, IDC_DATA3, m_Data);
	//	DDV_MinMaxUInt(pDX, m_Data, 0, 65535);
	DDX_Text(pDX, IDC_DATA3, m_Data3);
	DDV_MinMaxUInt(pDX, m_Data3, 0, 65535);
	DDX_Text(pDX, IDC_CMDID, m_CmdID);
	DDV_MinMaxUInt(pDX, m_CmdID, 0, 65535555);
	DDX_Text(pDX, IDC_DATA4, m_Data4);
	DDV_MinMaxUInt(pDX, m_Data4, 0, 65535);
	DDX_Text(pDX, IDC_OFFSET, m_Offset);
	DDV_MinMaxUInt(pDX, m_Offset, 0, 65535);
	DDX_Text(pDX, IDC_EDIT_0xCMD, m_str0xCMD);
	DDX_Text(pDX, IDC_EDIT_Len, m_CMD_Len);
	DDV_MinMaxInt(pDX, m_CMD_Len, 0, 65535);
}

BEGIN_MESSAGE_MAP(CEX_SDMDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_BTNTEST, &CEX_SDMDlg::OnClickedBtntest)
	ON_BN_CLICKED(IDC_BTNENTER, &CEX_SDMDlg::OnClickedBtnenter)
	ON_MESSAGE(WM_MYMESSAGE, &CEX_SDMDlg::OnMymessage)
//ON_REGISTERED_MESSAGE(WM_MYMESSAGE, &CEX_SDMDlg::OnMymessage)
ON_BN_CLICKED(IDC_BTNPULL, &CEX_SDMDlg::OnClickedBtnpull)
ON_WM_CLOSE()
ON_BN_CLICKED(IDC_BTNCMD, &CEX_SDMDlg::OnClickedBtncmd)
END_MESSAGE_MAP()


// CEX_SDMDlg ��Ϣ��������

BOOL CEX_SDMDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// ��������...���˵������ӵ�ϵͳ�˵��С�

	// IDM_ABOUTBOX ������ϵͳ���Χ�ڡ�
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		BOOL bNameValid;
		CString strAboutMenu;
		bNameValid = strAboutMenu.LoadString(IDS_ABOUTBOX);
		ASSERT(bNameValid);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// ���ô˶Ի����ͼ�ꡣ��Ӧ�ó��������ڲ��ǶԻ���ʱ����ܽ��Զ�
	//  ִ�д˲���
	SetIcon(m_hIcon, TRUE);			// ���ô�ͼ��
	SetIcon(m_hIcon, FALSE);		// ����Сͼ��

	// TODO: �ڴ����Ӷ���ĳ�ʼ������
	
	if(err_Success!=LINK.fnInit())
	{
		//PCI��ʼ��ʧ�ܣ��رմ���
		MessageBox("PCI��ʼ��ʧ�ܣ�");
		//exit(NULL);
	}
	return TRUE;  // ���ǽ��������õ��ؼ������򷵻� TRUE
}

void CEX_SDMDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialogEx::OnSysCommand(nID, lParam);
	}
}

// �����Ի���������С����ť������Ҫ����Ĵ���
//  �����Ƹ�ͼ�ꡣ����ʹ���ĵ�/��ͼģ�͵� MFC Ӧ�ó���
//  �⽫�ɿ���Զ���ɡ�

void CEX_SDMDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // ���ڻ��Ƶ��豸������

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// ʹͼ���ڹ����������о���
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// ����ͼ��
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

//���û��϶���С������ʱϵͳ���ô˺���ȡ�ù��
//��ʾ��
HCURSOR CEX_SDMDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

int i_countNum = 0;

bool InitTimerCheckSlave();
void CEX_SDMDlg::OnClickedBtntest()
{
	// TODO: �ڴ����ӿؼ�֪ͨ�����������
	//LINK.fnSetLockM(false);
	//LINK.fnSetLockS(false);
	LINK.fnSetCWnd(this);
	LINK.fnSetTextTimerSlaveID(IDC_STATIC);
	InitTimerCheckSlave();
}
sTestData fnCreatTestData(const UINT uiD1,const UINT uiD2,const UINT uiD3,const UINT uiD4)
{
	sTestData TD;

	TD.ucData = uiD1;
	TD.ucataNO = uiD2;
	//TD.ucFlag = uiD3;
	//TD.ucDefault = uiD4;

	//unsigned char* puiTestData = (unsigned char*)&TD;
	//UCHAR uiTestLen = sizeof(TD);
	//LINK.fnBuffRoute(puiTestData,uiTestLen,NULL);
	return TD;
}

//sTestData TD;
void CEX_SDMDlg::OnClickedBtnenter()
{
	// TODO: �ڴ����ӿؼ�֪ͨ�����������  
	LINK.fnSetLockS(false);
	LINK.fnSetLockM(false);
	UpdateData(TRUE);
	if(LINK.m_bAsyLock == false)
		LINK.fnBuffRoute(&fnCreatTestData(m_Data1,m_Data2,m_Data3,m_Data4),m_Offset);
	
	//return 0;
}

afx_msg LRESULT CEX_SDMDlg::OnMymessage(WPARAM wParam, LPARAM lParam)
{
	CStatic* pWnd = (CStatic*)GetDlgItem(wParam);
	CString str;
	str.Format(_T("%d"),lParam);
	pWnd->SetWindowText(str);
	
	return 0;
}


void CEX_SDMDlg::OnClickedBtnpull()
{
	// TODO: �ڴ����ӿؼ�֪ͨ�����������
	UpdateData();
	BufData bd;
	if(LINK.m_bAsyLock == false && err_Success==LINK.fnBuffPull(m_CmdID,&bd))
	{ 
		CStatic* pWnd = (CStatic*)GetDlgItem(IDC_STATICDATA);
		pWnd->SetWindowText(LINK.fnExData(&bd));
	}else MessageBox("�б�δ��������ݣ�\n����������Ѿ����ڻ����б����ˣ�");

}


//void CAboutDlg::OnClose()
//{
//	// TODO: �ڴ�������Ϣ������������/�����Ĭ��ֵ
//
//	CDialogEx::OnClose();
//}


void CEX_SDMDlg::OnClose()
{
	// TODO: �ڴ�������Ϣ������������/�����Ĭ��ֵ
	if(err_Success!=LINK.fnReleasePCI())
	{
		MessageBox("PCI�ر���������");
	}else 
	CDialogEx::OnClose();
}

bool fnChangeInfo2Data(CString strCmd,int iLen,unsigned char* pucI);
void CEX_SDMDlg::OnClickedBtncmd()
{
	// TODO: �ڴ����ӿؼ�֪ͨ�����������
	UpdateData();
	unsigned char* pucInfo = new unsigned char[m_CMD_Len];
	int* piCmdID = new int[1];
	if(fnChangeInfo2Data(m_str0xCMD,m_CMD_Len,pucInfo))
	{
		if(err_Success == LINK.fnSendToBuffer(pucInfo,m_CMD_Len,piCmdID))
		{
			delete[] pucInfo;
			pucInfo = NULL;
			delete piCmdID;
			piCmdID = NULL;
		}else
		{
			MessageBox("Connection Is Unvalid!");
		}
	}
}

bool fnChangeInfo2Data(CString strCmd,int iLen,unsigned char* pucI)
{
	CString Cmd = strCmd;
	int CmdLen = iLen;
	int i = 0;

	unsigned char* puc = (unsigned char*)(Cmd.GetBuffer(Cmd.GetLength()));
	//��strת����Ϊunsigned char* �浽�����У���õ�Ϊascii��
	//��ascii��ת��Ϊ������ת�浽pucIָ�����ڴ���
	for (i = 0;i < iLen*2;i++)
	{
		if(0 == i%2)
		{
			if(*(puc+i) >= '0'&&*(puc+i) <= '9')
				*(pucI + i/2) = (*(puc+i) - 48)*0x10;
			else if(*(puc+i) >= 'a'&&*(puc+i) <= 'z')
				*(pucI + i/2) = (*(puc+i) - 87)*0x10;
			else if(*(puc+i) >= 'A'&&*(puc+i) <= 'Z')
				*(pucI + i/2) = (*(puc+i) - 55)*0x10;
		}
		else
		{
			if(*(puc+i) >= '0'&&*(puc+i) <= '9')
				*(pucI + i/2) += (*(puc+i) - 48)*0x1;
			else if(*(puc+i) >= 'a'&&*(puc+i) <= 'z')
				*(pucI + i/2) += (*(puc+i) - 87)*0x1;
			else if(*(puc+i) >= 'A'&&*(puc+i) <= 'Z')
				*(pucI + i/2) += (*(puc+i) - 55)*0x1;
		}
	}
	//����ΪLen���ڴ�������д�������
//	delete[] puc;
	//puc = NULL;
	//�ͷ��ַ�����������ʵû�����õ�����
	Cmd.ReleaseBuffer();
	return true;
}