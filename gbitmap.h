//////////////////////////////////////////////////////////////////////
//
//	Program:	gbitmap.h
//	Author:		Gang Zhang
//	E-mail:		gzhang_ocx@yahoo.com or gazhang@geocities.com
//	Version:	1.0
//	Date:		06/21/1996
//


#ifndef GANGDIBAPIHEADER
#define GANGDIBAPIHEADER

//use to realize palette
#define WM_DOREALIZE   (WM_USER + 5)
/*	relative functions in view class
	afx_msg LRESULT OnDoRealize(WPARAM wParam, LPARAM);
		ON_MESSAGE(WM_DOREALIZE, OnDoRealize)

	LRESULT CShowView::OnDoRealize(WPARAM wParam, LPARAM)
	{
		ASSERT(wParam != NULL);
		if(!GetDocument()->Getbitmap())return 0L;  // must be a new document

		GetDocument()->Getbitmap()->RealizePalette(((HWND)wParam) != m_hWnd);
		return 0L;
	}

	and in mainfram
	void CMainFrame::OnPaletteChanged(CWnd* pFocusWnd)
{
	CMDIFrameWnd::OnPaletteChanged(pFocusWnd);

	// always realize the palette for the active view
	CMDIChildWnd* pMDIChildWnd = MDIGetActive();
	if (pMDIChildWnd == NULL)
		return; // no active MDI child frame
	CView* pView = pMDIChildWnd->GetActiveView();
	ASSERT(pView != NULL);

	// notify all child windows that the palette has changed
	SendMessageToDescendants(WM_DOREALIZE, (WPARAM)pView->m_hWnd);
}



	BOOL CMainFrame::OnQueryNewPalette()
	{
	// always realize the palette for the active view
	CMDIChildWnd* pMDIChildWnd = MDIGetActive();
	if (pMDIChildWnd == NULL)
		return FALSE; // no active MDI child frame (no new palette)
	CView* pView = pMDIChildWnd->GetActiveView();
	ASSERT(pView != NULL);

	// just notify the target view
	pView->SendMessage(WM_DOREALIZE, (WPARAM)pView->m_hWnd);
	return TRUE;
	}

*/


class GBitmap:public CBitmap{
	CPalette*cPal;
	int width,height;

public:
	GBitmap();
	~GBitmap();
	GBitmap(int w,int h,int mode);

	static int WIDTHBYTES(int bits)  {return ((((bits) + 31)>>5)<<2);}	
	void bmp8Toppm(BITMAPINFOHEADER*bmpinfo,unsigned char *data,RGBQUAD *rgbs);

	BOOL CreateBitmap(BITMAPINFO *pbmi);
	BOOL CreateBitmap(int w,int h,int mode=WHITENESS);
	BOOL CreateBitmap8(int w,int h,unsigned char *dat,int colors=256,RGBQUAD *rgb=NULL);
	BOOL gLoadBitmap(int name){return gLoadBitmap((LPSTR)name);}
	BOOL gLoadBitmap(LPSTR name);
	BOOL gLoadBitmap(BITMAPINFOHEADER *hbmp);	//actually create one with the info

	BOOL ReadDIBFile(CFile &file);
	BOOL ReadDIBFile(CFile &file,LPSTR *lpbm);	//read in to a bitmapinfoheader as well
	BOOL WriteDIBFile(CFile &file,BITMAPINFOHEADER *header,RGBQUAD *palette,void * data);

	void FillInfoHeader(BITMAPINFOHEADER*header,int width,int height,int bits,int color);
	void DeleteBitmap(void);

	void PaintBitmap(CDC*hdc,int dx,int dy,int dw,int dh,int sx,int sy,int sw,int sh,int mode=SRCCOPY);
	void PaintBitmap(CDC*hdc,int dx,int dy,int dw,int dh,int sx,int sy,int mode=SRCCOPY);
	void PaintBitmap(CDC*hdc,int dx,int dy,int dw,int dh,int mode=SRCCOPY);
	void CopyBitmap(int dx,int dy,int dw,int dh,CBitmap*bmp,int mode=SRCCOPY);
	void CopyBitmap(int dx,int dy,int dw,int dh,
		CBitmap*bmp,int sx,int sy,int sw,int sh,int mode=SRCCOPY);

	void GetBitmap(CDC*hdc,int dx,int dy,int dw,int dh,int sx,int sy,int sw,int sh,int mode=SRCCOPY);

	void bmpSelfAction(int mode,int x,int y);
	void bmpSelfAction(int mode);
	GBitmap* loadwMsk(char*namebmp,char*namemsk);
	void maskBlt(GBitmap*bmp,GBitmap*msk,int dx,int dy,int dw,int dh);

	UINT  RealizePalette(int bk);
	UINT  RealizePalette(CDC*hdc,int bk);
	UINT  RealizePalette(CWnd*wnd,int bk);

	inline DWORD DIBWidth(LPBITMAPINFOHEADER lpbmi);
	inline DWORD DIBHeight(LPBITMAPINFOHEADER lpbmi);
	DWORD DIBWidth(void){return width;}
	DWORD DIBHeight(void){return height;}
	inline WORD	 PaletteSize(LPBITMAPINFOHEADER lpbi);
	inline WORD	 DIBNumColors(LPBITMAPINFOHEADER lpbi);
	LPSTR FindDIBBits(LPBITMAPINFOHEADER lpbi){return ((LPSTR)lpbi + *(LPDWORD)lpbi +PaletteSize(lpbi));};
	LPSTR FindDIBBits(BITMAPINFO* lpbi){return FindDIBBits((LPBITMAPINFOHEADER)lpbi);}

	BOOL CreatePal24(BITMAPINFOHEADER *hbmp);
	BOOL CreateDIBPalette(LPBITMAPINFO lpbi);
	BOOL CreateDIBPalette(WORD colorentrys,unsigned char*pal);

	//paint to dc with no stretch and our palette
	BOOL PaintDIB(CDC*hDC,int dx,int dy,int dw,int dh,LPSTR lpDIBHdr);
	//seperate dib header and bits
	BOOL WINAPI PaintDIB(CDC     *hDC,
					int dx,int dy,int dw,int dh,
					LPSTR   lpDIBHdr,LPSTR lpDIBBits, 
					int sx,int sy,int sw,int sh,
					CPalette* pPal);
	//use when dib bits follow dib header
	BOOL PaintDIB(CDC*hDC,
					int dx,int dy,int dw,int dh,
					LPSTR   lpDIBHdr,
					int sx,int sy,int sw,int sh,
					CPalette* pPal);
};

//a sample version
class cbitmap{
	public:
		static int get_bwidth(int width){
			return ((width+3)&(~0x3));
		}
	cbitmap(){
	}

	//256 color by default
	void fillBitmapInfo(BITMAPINFO *pbmpi,int width,int height,int bitcount=8){
		pbmpi->bmiHeader.biSize=sizeof(BITMAPINFOHEADER); 
		pbmpi->bmiHeader.biWidth=width; 
		pbmpi->bmiHeader.biHeight=-height; 
		pbmpi->bmiHeader.biPlanes=1; 
		pbmpi->bmiHeader.biBitCount=bitcount;
		pbmpi->bmiHeader.biCompression=BI_RGB; 
		pbmpi->bmiHeader.biSizeImage=0; 
		pbmpi->bmiHeader.biXPelsPerMeter=0; 
		pbmpi->bmiHeader.biYPelsPerMeter=0; 
		pbmpi->bmiHeader.biClrUsed=256; 
		pbmpi->bmiHeader.biClrImportant=256; 

		for(int i=0;i<256;i++){
			pbmpi->bmiColors[i].rgbBlue=
			pbmpi->bmiColors[i].rgbGreen= 
			pbmpi->bmiColors[i].rgbRed=i;
			pbmpi->bmiColors[i].rgbReserved=0;
		}
	}

	void draw_row_256(CDC *pDC,unsigned char *buff,int width,int heigh){
			typedef struct tagBITMAPINFO 
			{    
				BITMAPINFOHEADER    bmiHeader;
				RGBQUAD             bmiColors[256];
			} BITMAPINFO256;

			BITMAPINFO256 bpmi;

		fillBitmapInfo((BITMAPINFO*)&bpmi,width,heigh);
		StretchDIBits(
			pDC->GetSafeHdc(),	// handle of device context 
			0,	// x-coordinate of upper-left corner of dest. rect. 
			0,	// y-coordinate of upper-left corner of dest. rect. 
			width,	// width of destination rectangle 
			heigh,	// height of destination rectangle 
			0,	// x-coordinate of upper-left corner of source rect. 
			0,	// y-coordinate of upper-left corner of source rect. 
			width,	// width of source rectangle 
			heigh,	// height of source rectangle 
			buff,	// address of bitmap bits 
			(BITMAPINFO*)&bpmi,	// address of bitmap data 
			DIB_RGB_COLORS,	// usage 
			SRCCOPY 	// raster operation code 
		);
	}
};


#endif
