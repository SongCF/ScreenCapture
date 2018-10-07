
// ScreenCaptureDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "ScreenCapture.h"
#include "ScreenCaptureDlg.h"
//#include "afxdialogex.h"

#include <Vfw.h>
#include <windowsx.h>
#include <mmsystem.h>
#include "videodriver.h"

#define NOTIFY_SHOW WM_USER+10   //自定义最小化窗口到右下角消息

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#define BUFSIZE 260

VIDEODRIVER videodriver;
int screenWidth;
int screenHeight;
int bits ;   //initial by videoDriver

int recordstate = 0;
int bThreadEnd = 1;
int tdata = 0;

int timelapse=40;		//帧间隔
int frames_per_second = 25;
DWORD compfccHandler = mmioFOURCC('M', 'S', 'V', 'C');
int compquality = 7000;
int keyFramesEvery = 200;
CString AVIfilepath ;
CString logfilepath;




UINT RecordAVIThread(LPVOID pParam);
int RecordVideo(int top,int left,int width,int height,int fps,const char *szFileName);
LPBITMAPINFOHEADER captureScreenFrame(int left,int top,int width, int height,int tempDisableRect);

// 用于应用程序“关于”菜单项的 CAboutDlg 对话框

class CAboutDlg : public CDialog
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
};

CAboutDlg::CAboutDlg() : CDialog(CAboutDlg::IDD)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
END_MESSAGE_MAP()


// CScreenCaptureDlg 对话框




CScreenCaptureDlg::CScreenCaptureDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CScreenCaptureDlg::IDD, pParent)
	, mStrPath(_T(""))
	, mRadio(0)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CScreenCaptureDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_SLIDER1, mSlider);
	DDX_Control(pDX, IDC_COMBO1, mCompressor);
	DDX_Text(pDX, IDC_EDIT1, mStrPath);
	DDX_Radio(pDX, IDC_RADIO1, mRadio);
}

BEGIN_MESSAGE_MAP(CScreenCaptureDlg, CDialog)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_START, &CScreenCaptureDlg::OnBnClickedStart)
	ON_BN_CLICKED(IDC_STOP, &CScreenCaptureDlg::OnBnClickedStop)
	ON_BN_CLICKED(IDC_EXIT, &CScreenCaptureDlg::OnBnClickedExit)

	ON_MESSAGE(NOTIFY_SHOW, OnTrayIcon)
	ON_BN_CLICKED(IDC_BUTTON1, &CScreenCaptureDlg::OnBnClickedButton1)
	ON_NOTIFY(NM_CUSTOMDRAW, IDC_SLIDER1, &CScreenCaptureDlg::OnNMCustomdrawSlider1)
END_MESSAGE_MAP()


// CScreenCaptureDlg 消息处理程序

BOOL CScreenCaptureDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

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
	//1. 压缩率
	mSlider.SetPos(70);
	mSlider.SetRange(10,100);
	SetDlgItemText(IDC_STATIC_SLIDER,"70%");
	//2. 压缩方式

	//3. 帧率
	mRadio = 2;
	//4. 保存路径
	CString fileName("\\ScreenRecord.avi");
	TCHAR path[255];
	SHGetSpecialFolderPath(0,path,CSIDL_DESKTOPDIRECTORY,0);
	mStrPath = path + fileName;
	//
	logfilepath = path;
	logfilepath += "\\recorderLog.txt";

	UpdateData(FALSE);

	return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
}

void CScreenCaptureDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialog::OnSysCommand(nID, lParam);
	}

	////////////////////////////////////////////////////
	if (SC_MINIMIZE == nID) 
	{ 
		NOTIFYICONDATA nid; 
		nid.cbSize = (DWORD)sizeof(NOTIFYICONDATA);    
		nid.hWnd = this->m_hWnd;    
		nid.uID = IDR_MAINFRAME;    
		nid.uFlags = NIF_ICON|NIF_MESSAGE|NIF_TIP   ;    
		nid.uCallbackMessage = NOTIFY_SHOW;//自定义的消息名称 
		nid.hIcon = LoadIcon(AfxGetInstanceHandle(), MAKEINTRESOURCE(IDR_MAINFRAME));    
		strcpy(nid.szTip, "Mirror Driver 屏幕录像");//信息提示条 

		Shell_NotifyIcon(NIM_ADD,&nid);//在托盘区添加图标 
		ShowWindow(SW_HIDE);//隐藏主窗口 
		return; 
	} 
}

// 如果向对话框添加最小化按钮，则需要下面的代码
//  来绘制该图标。对于使用文档/视图模型的 MFC 应用程序，
//  这将由框架自动完成。

void CScreenCaptureDlg::OnPaint()
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
		CDialog::OnPaint();
	}
}

//当用户拖动最小化窗口时系统调用此函数取得光标
//显示。
HCURSOR CScreenCaptureDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}


void CScreenCaptureDlg::OnBnClickedExit()
{
	// TODO: 在此添加控件通知处理程序代码
	recordstate = 0;
	while(bThreadEnd == 0)	
		Sleep(10);

	CDialog::OnCancel();
}


void CScreenCaptureDlg::OnBnClickedStop()
{
	// TODO: 在此添加控件通知处理程序代码
	recordstate = 0;
	GetDlgItem(IDC_START)->EnableWindow(true);
}


void CScreenCaptureDlg::OnBnClickedStart()
{
	GetDlgItem(IDC_START)->EnableWindow(FALSE);
	// TODO: 在此添加控件通知处理程序代码
	HDC hDisplayDC_Temp;
	hDisplayDC_Temp= CreateDC("DISPLAY",NULL,NULL,NULL);
	screenWidth= GetDeviceCaps(hDisplayDC_Temp,HORZRES) ;
	screenHeight = GetDeviceCaps(hDisplayDC_Temp,VERTRES);
	DeleteDC(hDisplayDC_Temp);

	videodriver.VIDEODRIVER_start(0,0,screenWidth,screenHeight,0);	
	videodriver.HardwareCursor();
	if (!videodriver.mypVideoMemory)
	{
		MessageBox("the shared memory ( memory mapped file) is not found","error",0);
		return ;
	}
	UpdateData(TRUE);

	//1. 压缩率
	compquality = 100 * mSlider.GetPos();
	//2. 压缩方式

	//3. 保存路径
	AVIfilepath = mStrPath;
	//4. 帧率
	int fps = 0;
	switch(this->mRadio)
	{
	case 0:
		fps =  5;break;
	case 1:
		fps = 15;break;
	case 2:
		fps = 25;break;
	case 3:
		fps = 30;break;
	default:
		fps = 200;break;
	}
	frames_per_second = fps;
	timelapse = 1000/fps;		//帧间隔

				
	SendMessage(WM_SYSCOMMAND, SC_MINIMIZE, 0);

	::InvalidateRect(NULL,NULL,true);
	recordstate = 1;
	CWinThread * pThread = AfxBeginThread(RecordAVIThread, &tdata);	

}

UINT RecordAVIThread(LPVOID pParam) 
{
#ifdef   _DEBUG
	//Test the validity of writing to the file
	//Make sure the file to be created is currently not used by another application
	int fileverified = 0;
	while (!fileverified) 
	{
		OFSTRUCT ofstruct;	
		HFILE fhandle = OpenFile( AVIfilepath, &ofstruct, OF_SHARE_EXCLUSIVE | OF_WRITE  | OF_CREATE );  
		if (fhandle != HFILE_ERROR) 
		{
			fileverified = 1;
			CloseHandle( (HANDLE) fhandle );
			DeleteFile(AVIfilepath);
		}	 
		else 
		{
			srand( (unsigned)time( NULL ) );
			int randnum = rand();
			char numstr[50];
			sprintf(numstr,"%d",randnum);

			CString cnumstr(numstr);
			CString fxstr("\\ScreenRecord");
			CString exstr(".avi");
			TCHAR  cha[50];
			GetTempPath(50,cha);
			AVIfilepath = cha + fxstr + cnumstr + exstr;
		}
	} 	
#endif

	bThreadEnd = 0;
	RecordVideo(0, 0, screenWidth, screenHeight, frames_per_second, AVIfilepath);
	bThreadEnd = 1;

	return 0;
}

int RecordVideo(int top,int left,int width,int height,int fps,const char *szFileName) 
{

	LPBITMAPINFOHEADER alpbi;

	AVISTREAMINFO strhdr;
	PAVIFILE pfile = NULL;
	PAVISTREAM ps = NULL, psCompressed = NULL;
	AVICOMPRESSOPTIONS opts;
	AVICOMPRESSOPTIONS FAR * aopts[1] = {&opts};

#ifdef   _DEBUG
	CFile file;
	file.Open(logfilepath,CFile::modeCreate|CFile::modeWrite);
	if(file)
		file.Write("帧编号    帧间隔    帧率\r\n", strlen("帧编号    帧间隔    帧率\r\n")*sizeof(TCHAR));
#endif


	WORD wVer = HIWORD(VideoForWindowsVersion());
	if (wVer < 0x010a)
	{			 		
		MessageBox(NULL, "Error! Video for Windows version too old!", "Error" , MB_OK|MB_ICONSTOP);
		return FALSE;
	}

	////////////////////////////////////////////////
	// CAPTURE FIRST FRAME
	////////////////////////////////////////////////	
	alpbi=captureScreenFrame(left,top,width, height,1);


	HRESULT hr;

	// 1. INIT AVI USING FIRST FRAME
	AVIFileInit();

	// 2. Open the movie file for writing....
	hr = AVIFileOpen(&pfile, szFileName, OF_WRITE | OF_CREATE, NULL);	
	if (hr != AVIERR_OK) 
		goto error;

	// Fill in the header for the video stream....
	// The video stream will run in 15ths of a second....
	memset(&strhdr, 0, sizeof(strhdr));
	strhdr.fccType                = streamtypeVIDEO;// stream type	
	//strhdr.fccHandler             = compfccHandler;
	strhdr.fccHandler             = 0;	
	strhdr.dwScale                = 1;
	strhdr.dwRate                 = fps;		    
	strhdr.dwSuggestedBufferSize  = alpbi->biSizeImage;
	SetRect(&strhdr.rcFrame, 0, 0,		    // rectangle for stream
		(int) alpbi->biWidth,
		(int) alpbi->biHeight);

	// 3. And create the stream;
	hr = AVIFileCreateStream(pfile,	&ps, &strhdr);
	if (hr != AVIERR_OK) 	
		goto error; 


	memset(&opts, 0, sizeof(opts)); 
	aopts[0]->fccType			 = streamtypeVIDEO;
	//	aopts[0]->fccHandler	 = mmioFOURCC('M', 'S', 'V', 'C');
	aopts[0]->fccHandler		 = compfccHandler;
	aopts[0]->dwKeyFrameEvery	 = keyFramesEvery;		// keyframe rate 
	aopts[0]->dwQuality		     = compquality;        // compress quality 0-10,000 
	aopts[0]->dwBytesPerSecond	 = 0;		// bytes per second 
	aopts[0]->dwFlags			 = AVICOMPRESSF_VALID | AVICOMPRESSF_KEYFRAMES;    // flags 		
	aopts[0]->lpFormat			 = 0x0;                         // save format 
	aopts[0]->cbFormat			 = 0;
	aopts[0]->dwInterleaveEvery  = 0;			// for non-video streams only 

	// 4. make the AVI compress
	hr = AVIMakeCompressedStream(&psCompressed, ps, &opts, NULL);
	if (hr != AVIERR_OK)  	
		goto error; 

	// 5. set AVI stream format
	hr = AVIStreamSetFormat(psCompressed, 0, 
		alpbi,	    // stream format
		alpbi->biSize +   // format size
		alpbi->biClrUsed * sizeof(RGBQUAD));
	if (hr != AVIERR_OK) 
		goto error;	


	if(alpbi)	
		GlobalFreePtr(alpbi);
	alpbi = NULL;


	DWORD timeexpended, frametime, oldframetime;	
	DWORD currtime, oldtime;

	DWORD initialtime = timeGetTime();		
	int initcapture = 1;	
	oldframetime = 0;
	int nCurrFrame = 0;        // =frametime理论当前帧数； 
	int nActualFrame = 0;      //实际帧数


	//////////////////////////////////////////////
	// WRITING FRAMES
	//////////////////////////////////////////////	
	while (recordstate) 
	{  //repeatedly loop

		if (initcapture==0) 
		{			
			currtime = timeGetTime();
			timeexpended = currtime - initialtime;			
		}
		else 
		{			
			frametime = 0;
			timeexpended = 0;						
		}				

		alpbi=captureScreenFrame(left,top,width, height,0);						

		if (initcapture==0) 
		{
			if (timelapse>1000)
				frametime++;
			else
				frametime = (DWORD) (((double) timeexpended /1000.0 ) * (double) (1000.0/timelapse));
		}
		else  
		{
			initcapture = 0;			
		}

		float fTimeLength = ((float) timeexpended) /((float) 1000.0);

		if ((frametime==0) || (frametime>oldframetime)) 
		{ 			
			//if frametime repeats (frametime == oldframetime) ...the avistreamwrite will cause an error
			hr = AVIStreamWrite(psCompressed,	// stream pointer
				frametime,				// time of this frame
				1,				// number to write
				(LPBYTE) alpbi +		// pointer to data
				alpbi->biSize +
				alpbi->biClrUsed * sizeof(RGBQUAD),
				alpbi->biSizeImage,	// size of this frame
				//AVIIF_KEYFRAME,			 // flags....
				0,    //Dependent n previous frame, not key frame
				NULL,
				NULL);

			if (hr != AVIERR_OK)
				break;		

			nActualFrame ++ ;
			nCurrFrame = frametime;
			float fRate = ((float) nCurrFrame)/fTimeLength;			//理论帧率				
			float fActualRate = ((float) nActualFrame)/fTimeLength; //实际帧率
			
			//Log
		#ifdef   _DEBUG
			if(file && nActualFrame>30)
			{
				TCHAR log[50] = {0};
				sprintf(log, "%6d    %04dms    %07.2f \r\n", nActualFrame, currtime-oldtime, fActualRate);
				file.Write(log, strlen(log)*sizeof(TCHAR));
			}
		#endif
			oldtime = currtime;


			//free memory
			if(alpbi)	
				GlobalFreePtr(alpbi);
			alpbi = NULL;

			oldframetime = frametime;

		} // if frametime is different


		//introduce time lapse
		//maximum lapse when recordstate changes will be less than 100 milliseconds
		int no_iteration = timelapse/50;
		int remainlapse = timelapse - no_iteration*50;		 
		for (int j=0;j<no_iteration;j++)		
		{
			::Sleep(50); //Sleep for 50 milliseconds many times
			if (recordstate==0) break;
		}		
		if (recordstate==1) 
			Sleep(remainlapse);


	} //for loop


error:	
#ifdef   _DEBUG
	if(file)
		file.Close();
#endif
	
	AVISaveOptionsFree(1,(LPAVICOMPRESSOPTIONS FAR *) &aopts);	


	if (pfile)
		AVIFileClose(pfile);


	if (ps)
		AVIStreamClose(ps);


	if (psCompressed)
		AVIStreamClose(psCompressed);

	AVIFileExit();	


	if (hr != NOERROR) 	
	{		
		AfxMessageBox("error");		
		return 0;
	}

	return 0;
}


LPBITMAPINFOHEADER captureScreenFrame(int left,int top,int width, int height,int tempDisableRect)
{

	HANDLE               hdib ;
	HDC                 hdc ;
	BITMAP              bitmap ;
	UINT                wLineLen ;
	DWORD               dwSize ;
	DWORD               wColSize ;
	LPBITMAPINFOHEADER  lpbi ;
	LPBYTE              lpBits ;


	wLineLen = (width*bits+31)/32 * 4;
	wColSize = sizeof(RGBQUAD)*((bits <= 8) ? 1<<bits : 0);
	dwSize = sizeof(BITMAPINFOHEADER) + wColSize +
		(DWORD)(UINT)wLineLen*(DWORD)(UINT)height;

	//
	// Allocate room for a DIB and set the LPBI fields
	//
	hdib = GlobalAlloc(GHND,dwSize);
	if (!hdib)
		exit(1) ;

	lpbi = (LPBITMAPINFOHEADER)GlobalLock(hdib) ;

	lpbi->biSize = sizeof(BITMAPINFOHEADER) ;
	lpbi->biWidth = width ;
	lpbi->biHeight = height ;
	lpbi->biPlanes = 1 ;
	lpbi->biBitCount = (WORD) bits ;
	lpbi->biCompression = BI_RGB ;
	lpbi->biSizeImage = dwSize - sizeof(BITMAPINFOHEADER) - wColSize ;
	lpbi->biXPelsPerMeter = 0 ;
	lpbi->biYPelsPerMeter = 0 ;
	lpbi->biClrUsed = (bits <= 8) ? 1<<bits : 0;
	lpbi->biClrImportant = 0 ;

	//
	// Get the bits from the bitmap and stuff them after the LPBI
	//
	lpBits = (LPBYTE)(lpbi+1)+wColSize ;

	if (videodriver.myframebuffer) 
	{
		int bytes_per_pixel=bits/8;														
		int bytesPerOutputRow = screenWidth * bytes_per_pixel;									
		int bytesPerInputRow = screenWidth * bytes_per_pixel;									
		BYTE *sourcepos,*destpos;														
		destpos = (BYTE *)lpBits;													
		sourcepos=(BYTE*)videodriver.myframebuffer + (bytesPerInputRow * (screenHeight-1));		
		int y2;																			
		int w = screenWidth*bytes_per_pixel;																
		for (y2=0; y2<screenHeight; y2++) {														
			memcpy(destpos, sourcepos, w);												
			sourcepos = (BYTE*)sourcepos - bytesPerInputRow;							
			destpos = (BYTE*)destpos + bytesPerOutputRow;								
		}
	}
	GlobalUnlock(hdib);

	LPBITMAPINFOHEADER pBM_HEADER = (LPBITMAPINFOHEADER)GlobalLock(hdib);	
	//LPBITMAPINFOHEADER pBM_HEADER = (LPBITMAPINFOHEADER)GlobalLock(Bitmap2Dib(hbm, 24));	
	if (pBM_HEADER == NULL) 
	{ 		
		MessageBox(NULL,"Error reading a frame!","Error",MB_OK | MB_ICONEXCLAMATION);					
		exit(1);
	}    

	return pBM_HEADER;   
}

LRESULT CScreenCaptureDlg::OnTrayIcon(WPARAM wParam, LPARAM lParam) 
{ 
	if (wParam != IDR_MAINFRAME) 
	{ 
		return 1; 
	} 

	switch(lParam) 
	{ 
	case WM_RBUTTONUP: 
		{ 
			//右键起来时弹出快捷菜单，这里只有一个“结束录像” 

			//声明一个弹出式菜单 
			//增加菜单项“结束录像”，点击则发送消息WM_CLOSE给主窗口（已    
			//隐藏），将程序结束。  
			CMenu menu; 
			menu.CreatePopupMenu(); 
			menu.AppendMenu(MF_STRING, ID_APP_EXIT, _T("结束录像")); 

			//得到鼠标位置 
			LPPOINT lpoint= new tagPOINT; 
			::GetCursorPos(lpoint); 

			//确定弹出式菜单的位置    
			menu.TrackPopupMenu(TPM_LEFTALIGN, lpoint->x, lpoint->y,this); 

			//资源回收 
			HMENU   hmenu=menu.Detach(); 
			menu.DestroyMenu(); 

			delete lpoint; 
			lpoint = NULL; 

			break; 
		} 
	case WM_LBUTTONDBLCLK: 
		{ 
			//双击左键的处理  
			//界面置顶 
			SetForegroundWindow();
			//显示界面   
			ShowWindow(SW_SHOW); 
			ShowWindow(SW_RESTORE );

			break; 
		} 
	}  
	return 0; 
} 

void CScreenCaptureDlg::OnCancel() 
{ 
	recordstate = 0;

	//删除任务栏图标 
	NOTIFYICONDATA nid; 
	nid.hWnd=this->m_hWnd; 
	nid.uID=IDR_MAINFRAME; 
	Shell_NotifyIcon(NIM_DELETE, &nid); 

	while(bThreadEnd == 0)	
		Sleep(10);

	CDialog::OnCancel(); 
} 
void CScreenCaptureDlg::OnBnClickedButton1()
{
	// TODO: 在此添加控件通知处理程序代码
	CFileDialog dlg(FALSE);
	dlg.m_ofn.lpstrFilter="Text Files(*.avi)\0*.avi\0\0";
	dlg.m_ofn.lpstrDefExt="avi";
	if ( dlg.DoModal() == IDOK )
	{
		// 取得文件路径全名
		mStrPath = dlg.GetPathName() ;
		// 更新界面显示
		UpdateData( FALSE ) ;
	}
}

void CScreenCaptureDlg::OnNMCustomdrawSlider1(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMCUSTOMDRAW pNMCD = reinterpret_cast<LPNMCUSTOMDRAW>(pNMHDR);
	// TODO: 在此添加控件通知处理程序代码
	
	CString str;
	str.Format("%2d%%", mSlider.GetPos());
	SetDlgItemText(IDC_STATIC_SLIDER, str);

	*pResult = 0;
}
