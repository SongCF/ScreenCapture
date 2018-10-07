// changed_screen_parts.cpp : Defines the entry point for the console application.
//

#include "videodriver.h"
#include <stdio.h>

int bits;

FILE *fp;
void captureScreenFrame(int x, int y, int w, int h, VIDEODRIVER& mydriver);
void handle_driver_input(PCHANGES_BUF pchanges_buf, int i);

int _tmain(int argc, _TCHAR* argv[])
{
	VIDEODRIVER mydriver;
	// if depth==0, we take the current depth
	HDC hDisplayDC_Temp;
	hDisplayDC_Temp= CreateDC("DISPLAY",NULL,NULL,NULL);
	int cxWidth= GetDeviceCaps(hDisplayDC_Temp,HORZRES) ;
	int cyHeight = GetDeviceCaps(hDisplayDC_Temp,VERTRES);
	DeleteDC(hDisplayDC_Temp);

	mydriver.VIDEODRIVER_start(0,0,cxWidth,cyHeight,0);
	
	if (!mydriver.mypVideoMemory) return 0; //error

	Sleep(1000);

	captureScreenFrame(0,0,cxWidth,cyHeight, mydriver);
	Sleep(500);


	fp = fopen("D:\\log.txt","w");
	if(fp==NULL)
	{
		printf("open file error");
		return 1;
	}
	PCHANGES_BUF pchanges_buf;
	pchanges_buf=mydriver.mypchangebuf;
	int oldaantal=0;
	int counter=pchanges_buf->counter;
	int runs=0;
	while(runs<1000)
	{
		runs++;
		counter=pchanges_buf->counter;
		if (oldaantal!=counter && (counter>=1 || counter <=1999))
		{
				oldaantal=counter;
				if (oldaantal<counter)
				{
					for (int i =oldaantal+1; i<=counter;i++)
						{
							handle_driver_input(pchanges_buf,i);
						}

				}
				else
				{
					int i = 0;
					for (i =oldaantal+1;i<MAXCHANGES_BUF;i++)
						{
							handle_driver_input(pchanges_buf,i);
						}
					for (i=1;i<=counter;i++)
						{
							handle_driver_input(pchanges_buf,i);
						}
		}	
		}		
	}


	return 0;
}

void captureScreenFrame(int x, int y, int w, int h, VIDEODRIVER& mydriver)
{
	BITMAP              bitmap ;
	UINT                wLineLen ;
	DWORD               dwSize ;
	DWORD               wColSize ;

	LPBITMAPINFOHEADER  lpbi ;
	LPBYTE              lpBits ;
	
//	int bits = 32;
	
	wLineLen = (w*bits+31)/32 * 4;
	wColSize = sizeof(RGBQUAD)*((bits <= 8) ? 1<<bits : 0);//真彩色没有调色版
	dwSize = sizeof(BITMAPINFOHEADER) + wColSize + (DWORD)(UINT)wLineLen*(DWORD)(UINT)h;

	//
	// Allocate room for a DIB and set the LPBI fields
	//
	HANDLE hdib = GlobalAlloc(GHND,dwSize);
	if (!hdib)
		exit(1) ;

	lpbi = (LPBITMAPINFOHEADER)GlobalLock(hdib) ;

	lpbi->biSize = sizeof(BITMAPINFOHEADER) ;
	lpbi->biWidth = w ;
	lpbi->biHeight = h ;
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


	if (mydriver.myframebuffer) 
	{		
		BYTE *bufferIn  = (BYTE *)mydriver.myframebuffer;
		BYTE *bufferOut = lpBits;

		int bytes_per_pixel=bits/8;														
		int bytesPerOutputRow = w * bytes_per_pixel;									
		int bytesPerInputRow = w * bytes_per_pixel;	

		BYTE *sourcepos,*destpos;														
		destpos = (BYTE *)bufferOut;													
		sourcepos=(BYTE*)bufferIn + (bytesPerInputRow * (y+h-1))+(x * bytes_per_pixel);		
//		sourcepos=(BYTE*)bufferIn + (x * bytes_per_pixel);	

		int ww= w*bytes_per_pixel;																
		for (int y2=0; y2<h; y2++) 
		{														
			memcpy(destpos, sourcepos, ww);												
			sourcepos = (BYTE*)sourcepos - bytesPerInputRow;							
			destpos = (BYTE*)destpos + bytesPerOutputRow;								
		}		
	}

	GlobalUnlock(hdib);
    			
	LPBITMAPINFOHEADER pBM_HEADER = (LPBITMAPINFOHEADER)GlobalLock(hdib);	
	//LPBITMAPINFOHEADER pBM_HEADER = (LPBITMAPINFOHEADER)GlobalLock(Bitmap2Dib(hbm, 24));	
	if (pBM_HEADER == NULL) { 
			
		//MessageBox(NULL,"Error reading a frame!","Error",MB_OK | MB_ICONEXCLAMATION);					
		//MessageOut(NULL,IDS_STRING_ERRFRAME ,IDS_STRING_NOTE,MB_OK | MB_ICONEXCLAMATION);
		exit(1);
	}    

	//位图文件头结构
	 BITMAPFILEHEADER   bmfHdr;        
 
	//创建位图文件    
	HANDLE  fh=CreateFile("D:\\screen.bmp", GENERIC_WRITE,0, NULL, CREATE_ALWAYS,FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN, NULL);
	if (fh==INVALID_HANDLE_VALUE)
		return ;
	// 设置位图文件头
	bmfHdr.bfType = 0x4D42;  // "BM"
	DWORD dwDIBSize=sizeof(BITMAPFILEHEADER)+sizeof(BITMAPINFOHEADER)+wColSize+(DWORD)(UINT)wLineLen*(DWORD)(UINT)h;  
	bmfHdr.bfSize = dwDIBSize;
	bmfHdr.bfReserved1 = 0;
	bmfHdr.bfReserved2 = 0;
	bmfHdr.bfOffBits = (DWORD)sizeof(BITMAPFILEHEADER)+(DWORD)sizeof(BITMAPINFOHEADER)+wColSize;

	DWORD dwWritten;
	// 写入位图文件头
	WriteFile(fh, (LPSTR)&bmfHdr, sizeof(BITMAPFILEHEADER), &dwWritten, NULL);
	// 写入位图文件其余内容
	WriteFile(fh, (LPSTR)lpbi, sizeof(BITMAPINFOHEADER)+wColSize+(DWORD)(UINT)wLineLen*(DWORD)(UINT)h , &dwWritten, NULL); 

}

void handle_driver_input(PCHANGES_BUF pchanges_buf, int i)
{
	switch(pchanges_buf->pointrect[i].type)
			{
				case SCREEN_SCREEN:
					{
						int dx=pchanges_buf->pointrect[i].point.x;
						int dy=pchanges_buf->pointrect[i].point.y;
						int left=pchanges_buf->pointrect[i].rect.left;
						int right=pchanges_buf->pointrect[i].rect.right;
						int top=pchanges_buf->pointrect[i].rect.top;
						int bottom=pchanges_buf->pointrect[i].rect.bottom;
						printf("move dx=%i dy=%i rect=%i right=%i top=%i bottom=%i \n",dx,dy,left,right,top,bottom);
						fprintf(fp,"move dx=%i dy=%i rect=%i right=%i top=%i bottom=%i \r\n",dx,dy,left,right,top,bottom);

						break;
					}

				case SOLIDFILL:
				case TEXTOUT:
				case BLEND:
				case TRANS:
				case PLG:
				case BLIT:
					{
					int left=pchanges_buf->pointrect[i].rect.left;
					int right=pchanges_buf->pointrect[i].rect.right;
					int top=pchanges_buf->pointrect[i].rect.top;
					int bottom=pchanges_buf->pointrect[i].rect.bottom;
					printf("blit left=%i right=%i top=%i bottom=%i \n",left,right,top,bottom);
					fprintf(fp,"blit left=%i right=%i top=%i bottom=%i \n",left,right,top,bottom);
					}

					break;
				default:
					break;
			}
}
