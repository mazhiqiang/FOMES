
// EX_SDMDlg.cpp : 实现文件
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

// 用于应用程序“关于”菜单项的 CAboutDlg 对话框

#include "mmsystem.h" 
#pragma comment(lib, "winmm.lib")
errorCode statusCode;
#define ASCII_SPACE 32
#define SPACE_EXIST 1
#define SPACE_NOT_EXIST 0
#define CMD_LEN_MAX 1024
class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// 对话框数据
	enum { IDD = IDD_ABOUTBOX };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

// 实现
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


// CEX_SDMDlg 对话框




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
	m_cmd_data = _T("");
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
	DDX_Text(pDX, IDC_EDIT_DATA, m_cmd_data);
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
ON_BN_CLICKED(IDC_BTN_DATA, &CEX_SDMDlg::OnBnClickedBtnData)
ON_BN_CLICKED(IDC_BUTTON1, &CEX_SDMDlg::OnBnClickedButton1)
END_MESSAGE_MAP()


// CEX_SDMDlg 消息处理程序

BOOL CEX_SDMDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// 将“关于...”菜单项添加到系统菜单中。

	// IDM_ABOUTBOX 必须在系统命令范围内。
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

	// 设置此对话框的图标。当应用程序主窗口不是对话框时，框架将自动
	//  执行此操作
	SetIcon(m_hIcon, TRUE);			// 设置大图标
	SetIcon(m_hIcon, FALSE);		// 设置小图标

	// TODO: 在此添加额外的初始化代码
	
	if(err_Success!=LINK.fnInit())
	{
		//PCI初始化失败，关闭窗口
		MessageBox("PCI初始化失败！");
		//exit(NULL);
	}
	return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
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

// 如果向对话框添加最小化按钮，则需要下面的代码
//  来绘制该图标。对于使用文档/视图模型的 MFC 应用程序，
//  这将由框架自动完成。

void CEX_SDMDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // 用于绘制的设备上下文

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// 使图标在工作区矩形中居中
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// 绘制图标
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

//当用户拖动最小化窗口时系统调用此函数取得光标
//显示。
HCURSOR CEX_SDMDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

int i_countNum = 0;


void CEX_SDMDlg::OnClickedBtntest()
{
	// TODO: 在此添加控件通知处理程序代码
	//LINK.fnSetLockM(false);
	//LINK.fnSetLockS(false);
	LINK.fnSetCWnd(this);
	LINK.fnSetTextTimerSlaveID(IDC_STATIC);
	LINK.InitTimerCheckSlave();
}



void CEX_SDMDlg::OnClickedBtnenter()
{
	// TODO: 在此添加控件通知处理程序代码  
	//LINK.fnSetLockS(false);
	//LINK.fnSetLockM(false);
	UpdateData(TRUE);
	//if(LINK.m_bAsyLock == false)
		//LINK.fnBuffRoute(&fnCreatTestData(m_Data1,m_Data2,m_Data3,m_Data4),m_Offset);
	
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
	// TODO: 在此添加控件通知处理程序代码
	UpdateData();
	BufData bd;
	if(LINK.m_bAsyLock == false && err_Success==LINK.fnBuffPull2WinForm(m_CmdID,&bd))
	{ 
		CStatic* pWnd = (CStatic*)GetDlgItem(IDC_STATICDATA);
		pWnd->SetWindowText(LINK.fnExData(&bd));
	}else MessageBox("列表未存入此数据，\n或这个数据已经不在缓存列表中了！");

}


//void CAboutDlg::OnClose()
//{
//	// TODO: 在此添加消息处理程序代码和/或调用默认值
//
//	CDialogEx::OnClose();
//}


void CEX_SDMDlg::OnClose()
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值
	if(err_Success!=LINK.fnReleasePCI())
	{
		MessageBox("PCI关闭遇到问题");
	}else 
	CDialogEx::OnClose();
}

bool fnChangeInfo2Data(CString strCmd,int iLen,unsigned char* pucI);
void CEX_SDMDlg::OnClickedBtncmd()
{
	// TODO: 在此添加控件通知处理程序代码
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
bool fnGetData(CString strCmd,unsigned char* pucI,short * pucDataLen)
{
	CString Cmd = strCmd;

	unsigned char* pucTotal = (unsigned char*)(Cmd.GetBuffer(Cmd.GetLength()));
	short ucCmdLen = Cmd.GetLength();
	unsigned char* pucLocationSpace = new unsigned char[ucCmdLen];
	short ucCount = ucCmdLen;
	short ucCountSpace = 0;
	//第一次循环找到空格的位置，同时确定有效数据的长度
	while(ucCount)
	{
		if(ASCII_SPACE == *(pucTotal + ucCmdLen - ucCount))
		{
			*(pucLocationSpace + ucCmdLen - ucCount) = 1;
			ucCountSpace++;
		}
		else *(pucLocationSpace + ucCmdLen- ucCount) = 0;
		ucCount--;
	}
	ucCount = ucCmdLen;
	*pucDataLen = (ucCmdLen - ucCountSpace)/2;
	//unsigned char* pucFilter = unsigned char[*pucDataLen];
	//第二次循环拆卸数据并进行拼接
	unsigned char ucDataLen = 0;
	short sCurrentPoint = 0;
	short sPointTmp = 0;
	while(ucCount)
	{
		
		if(*(pucLocationSpace + ucCmdLen- ucCount) == SPACE_NOT_EXIST)
		{
			ucDataLen++;
			sCurrentPoint++;
		}
		else
		{
			sPointTmp = sCurrentPoint/2 - 1;
			unsigned char i = 0;
			unsigned char LH = 0;
			unsigned char ucOffset = (ucCmdLen- ucCount - ucDataLen);
			for (i = ucOffset;i < ucOffset + ucDataLen;i++)
			{
				if(0 == LH%2)//H
				{
					if(*(pucTotal+i) >= '0'&&*(pucTotal+i) <= '9')
						*(pucI + sPointTmp) = (*(pucTotal+i) - 48)*0x10;
					else if(*(pucTotal+i) >= 'a'&&*(pucTotal+i) <= 'z')
						*(pucI + sPointTmp) = (*(pucTotal+i) - 87)*0x10;
					else if(*(pucTotal+i) >= 'A'&&*(pucTotal+i) <= 'Z')
						*(pucI + sPointTmp) = (*(pucTotal+i) - 55)*0x10;					
				}
				else
				{
					if(*(pucTotal+i) >= '0'&&*(pucTotal+i) <= '9')
						*(pucI + sPointTmp) += (*(pucTotal+i) - 48)*0x1;
					else if(*(pucTotal+i) >= 'a'&&*(pucTotal+i) <= 'z')
						*(pucI + sPointTmp) += (*(pucTotal+i) - 87)*0x1;
					else if(*(pucTotal+i) >= 'A'&&*(pucTotal+i) <= 'Z')
						*(pucI + sPointTmp) += (*(pucTotal+i) - 55)*0x1;
					sPointTmp--;
				}
				LH++;
			}
			LH = 0;
			ucDataLen = 0;
		}
		ucCount--;
	}
	//delet pucFilter[];
	//pucFilter = NULL;
	delete[] pucLocationSpace;
	pucLocationSpace = NULL;
	return true;
}
bool fnChangeInfo2Data(CString strCmd,int iLen,unsigned char* pucI)
{
	CString Cmd = strCmd;
	int CmdLen = iLen;
	int i = 0;

	unsigned char* puc = (unsigned char*)(Cmd.GetBuffer(Cmd.GetLength()));
	//将str转换到为unsigned char* 存到数据中，获得的为ascii码
	//将ascii码转换为数字再转存到pucI指定的内存中
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
	//长度为Len的内存中填入写入的数据
//	delete[] puc;
	//puc = NULL;
	//释放字符串，不过其实没有作用的喵！
	Cmd.ReleaseBuffer();
	return true;
}


void CEX_SDMDlg::OnBnClickedBtnData()
{
	// TODO: 在此添加控件通知处理程序代码
	UpdateData();
	unsigned char* pucInfo = new unsigned char[CMD_LEN_MAX];
	int* piCmdID = new int[1];
	short* psCmdLen = new short[1];
	
	if(fnGetData(m_cmd_data,pucInfo,psCmdLen))
	{
		if(err_Success == LINK.fnSendToBuffer(pucInfo,*psCmdLen,piCmdID))
		{
			delete[] pucInfo;
			pucInfo = NULL;
			delete piCmdID;
			piCmdID = NULL;
			delete psCmdLen;
			psCmdLen = NULL;
		}else
		{
			MessageBox("Connection Is Unvalid!");
		}
	}
}


void CEX_SDMDlg::OnBnClickedButton1()
{
	// TODO: 在此添加控件通知处理程序代码
	LINK.fnForce2WriteStatus();
}
