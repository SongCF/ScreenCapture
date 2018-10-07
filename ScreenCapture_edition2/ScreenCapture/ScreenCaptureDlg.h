
// ScreenCaptureDlg.h : ͷ�ļ�
//

#pragma once
#include "afxcmn.h"
#include "afxwin.h"


// CScreenCaptureDlg �Ի���
class CScreenCaptureDlg : public CDialog
{
// ����
public:
	CScreenCaptureDlg(CWnd* pParent = NULL);	// ��׼���캯��

// �Ի�������
	enum { IDD = IDD_SCREENCAPTURE_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV ֧��


// ʵ��
protected:
	HICON m_hIcon;

	// ���ɵ���Ϣӳ�亯��
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
