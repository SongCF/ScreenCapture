
// ScreenCaptureDlg.h : 头文件
//

#pragma once
#include "afxcmn.h"
#include "afxwin.h"


// CScreenCaptureDlg 对话框
class CScreenCaptureDlg : public CDialog
{
// 构造
public:
	CScreenCaptureDlg(CWnd* pParent = NULL);	// 标准构造函数

// 对话框数据
	enum { IDD = IDD_SCREENCAPTURE_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 支持


// 实现
protected:
	HICON m_hIcon;

	// 生成的消息映射函数
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedStart();
	afx_msg void OnBnClickedStop();
	afx_msg void OnBnClickedExit();

	afx_msg LRESULT OnTrayIcon(WPARAM wParam, LPARAM lParam);
	virtual void OnCancel();
	CSliderCtrl mSlider;
public:
	CComboBox mCompressor;
	CString mStrPath;
	int mRadio;
public:
	afx_msg void OnBnClickedButton1();
public:
	afx_msg void OnNMCustomdrawSlider1(NMHDR *pNMHDR, LRESULT *pResult);
};
