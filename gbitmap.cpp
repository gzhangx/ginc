//////////////////////////////////////////////////////////////////////
//
//	Program:	gbitmap.cpp
//	Author:		Gang Zhang
//	E-mail:		gzhang_ocx@yahoo.com or gazhang@geocities.com
//	Version:	1.0
//	Date:		06/21/1996
//

#include "afxwin.h"
#include "gbitmap.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#define PALVERSION   0x300
#define IS_WIN30_DIB(lpbi)  ((*(LPDWORD)(lpbi)) == sizeof(BITMAPINFOHEADER))

GBitmap::GBitmap(){
	cPal=NULL;
	width=height=0;
}

GBitmap::GBitmap(int w,int h,int mode){
	cPal=NULL;
	width=w;height=h;
	this->CreateBitmap(w,h,mode);
	unsigned char pal[256*4];
	for(int i=0;i<256;i++)pal[i*4]=pal[i*4+1]=pal[i*4+2]=i;
	this->CreateDIBPalette(256,pal);
}

GBitmap::~GBitmap(){
	if(cPal)delete cPal;
	this->DeleteObject();
}

//create an empty bitmap but no palette
BOOL GBitmap::CreateBitmap(int w,int h,int mode){
	CClientDC dc(AfxGetMainWnd());
	if(!CBitmap::CreateBitmap(w,h,dc.GetDeviceCaps(PLANES),
		dc.GetDeviceCaps(BITSPIXEL),NULL))return FALSE;
	this->bmpSelfAction(mode,w,h);
	width=w;height=h;
	return TRUE;
}

//Create bitmap and palette with a pbmi structure
BOOL GBitmap::CreateBitmap(BITMAPINFO *pbmi){
	CClientDC dc(AfxGetMainWnd());
	width=pbmi->bmiHeader.biWidth;height=pbmi->bmiHeader.biHeight;
	if(!CBitmap::CreateBitmap(width,height,dc.GetDeviceCaps(PLANES),
		dc.GetDeviceCaps(BITSPIXEL),FindDIBBits(pbmi)))return FALSE;
	CreateDIBPalette(pbmi);
	return TRUE;
}

void GBitmap::DeleteBitmap(void){
	this->DeleteObject();
}

BOOL GBitmap::CreatePal24(BITMAPINFOHEADER *hbmp){
	unsigned char pal[256*4];
	unsigned char *buffer=(unsigned char*)FindDIBBits(hbmp);
	int widthb=WIDTHBYTES(width*24),h=height;
	int i,j;
	if(hbmp->biBitCount!=24)return FALSE;
	if(hbmp->biCompression!=BI_RGB)return FALSE;
	/*	
	struct colink{
		union{
			int i;
			unsigned char c[4];
		}u;
		int count;
		struct colink*next;
	}base,*pl,*tpl,*cpl;

	base.next=NULL;
	base.u.i=0;
	base.count=1;
	pl=&base;

	cpl=new struct colink;
	cpl->next=NULL;
	cpl->count=0;	
	for(j=0;j<height;j++){
		int at=j*widthb*3;
		for(i=0;i<width*3;i++){
			cpl->u.i=0;
			cpl->u.c[0]=*(buffer+at+i*3);
			cpl->u.c[1]=*(buffer+at+i*3+1);
			cpl->u.c[2]=*(buffer+at+i*3+2);
			tpl=&base;
			while(tpl){
				if(tpl->u.i==cpl->u.i){
					tpl->count++;
					break;
				}
				tpl=tpl->next;
			}
			if(!tpl){
				tpl=&base;
				while(tpl->next){
					tpl=tpl->next;
				}
				cpl->count=1;
				tpl->next=cpl;
				cpl=new struct colink;
				cpl->count=0;
				cpl->u.i=0;
				cpl->next=NULL;
			}
		}
	}

	//all color in colink, now find the mouse important 256 color
	for(i=0;i<256;i++){
		int maxcolor=0;
		tpl=&base;
		while(tpl->next){
			if(maxcolor<tpl->next->count)maxcolor=tpl->next->count;
			tpl=tpl->next;
		}
		if(maxcolor==0)break;
		tpl=&base;
		while(tpl){
			if(tpl->next){
				if(tpl->next->count==maxcolor){
					pal[i*4]=tpl->next->u.c[0];
					pal[i*4+1]=tpl->next->u.c[1];
					pal[i*4+2]=tpl->next->u.c[2];
					pal[i*4+3]=0;
					cpl=tpl->next->next;
					delete tpl->next;
					tpl->next=cpl;
				}
				tpl=tpl->next;
			}else{
				pal[i*4]=tpl->u.c[0];
				pal[i*4+1]=tpl->u.c[1];
				pal[i*4+2]=tpl->u.c[2];
				pal[i*4+3]=0;
				break;
			}
			//if(maxcolor<tpl->count)min=tpl->count;
		}
	}

	tpl=&base;
	while(tpl->next){
		cpl=tpl->next->next;
		delete tpl->next;
		tpl->next=cpl;
	}
	*/
	int color;
	for(j=0;j<height;j++){
		int at=j*widthb;
		for(i=0;i<width;i++){
			color=(int)(.3**(buffer+at+i*3)+.4**(buffer+at+i*3+1)+
						.3**(buffer+at+i*3+2));
			if(color>255)color=255;
			pal[color*4]=*(buffer+at+i*3);
			pal[color*4+1]=*(buffer+at+i*3+1);
			pal[color*4+2]=*(buffer+at+i*3+2);
			pal[color*4+3]=0;
		}
	}
	
	return CreateDIBPalette(256,pal);
}

BOOL GBitmap::gLoadBitmap(LPSTR name){
	HGLOBAL hgbl=::LoadResource(AfxGetInstanceHandle(),
		FindResource(AfxGetInstanceHandle(),name,RT_BITMAP)
    );
	HANDLE hbmp=::LockResource(hgbl);
	if(!hbmp)return FALSE;
	CreateDIBPalette((LPBITMAPINFO)hbmp);
	CreatePal24((LPBITMAPINFOHEADER)hbmp);
	width=DIBWidth((LPBITMAPINFOHEADER)hbmp);
	height=DIBHeight((LPBITMAPINFOHEADER)hbmp);
	if(!this->CreateBitmap(width,height)){
		UnlockResource(hgbl);
		FreeResource(hgbl);
		return FALSE;
	}
	CDC dc;
	dc.CreateCompatibleDC(NULL);
	CBitmap*old=dc.SelectObject(this);
	CPalette*oldPal=dc.SelectPalette(cPal,0);
	dc.RealizePalette();
	PaintDIB(&dc,0,0,width,height,(char*)hbmp);
	dc.SelectPalette(oldPal,0);
	dc.SelectObject(old);
	UnlockResource(hgbl);
	FreeResource(hgbl);
	return TRUE;
}

BOOL GBitmap::gLoadBitmap(BITMAPINFOHEADER *hbmp){
	if(!this->CreateBitmap(DIBWidth(hbmp),DIBHeight(hbmp)))return FALSE;
	CreateDIBPalette((BITMAPINFO*)hbmp);
	//CreatePal24(hbmp);
	CDC dc;
	dc.CreateCompatibleDC(NULL);
	CBitmap*old=dc.SelectObject(this);
	CPalette*oldPal=dc.SelectPalette(cPal,0);
	dc.RealizePalette();
	PaintDIB(&dc,0,0,width,height,(char*)hbmp);
	dc.SelectPalette(oldPal,0);
	dc.SelectObject(old);
	return TRUE;
}

GBitmap* GBitmap::loadwMsk(char*namebmp,char*namemsk){
	this->gLoadBitmap(namebmp);
	GBitmap*msk=new GBitmap;
	msk->gLoadBitmap(namemsk);
	BITMAP bm;
	this->GetObject(sizeof(BITMAP),&bm);
	msk->bmpSelfAction(DSTINVERT,bm.bmWidth,bm.bmHeight);
	this->CopyBitmap(0,0,bm.bmWidth,bm.bmHeight,msk,SRCAND);
	msk->bmpSelfAction(DSTINVERT,bm.bmWidth,bm.bmHeight);
	return msk;
}

void GBitmap::bmpSelfAction(int mode,int x,int y){
	CDC dc;
	if(!(x+y)){
		x=width;
		y=height;
	}
	dc.CreateCompatibleDC(NULL);
	CBitmap*old=dc.SelectObject(this);
	dc.BitBlt(0,0,x,y,NULL,0,0,mode);
	dc.SelectObject(old);
}

void GBitmap::bmpSelfAction(int mode){
	CDC dc;
	dc.CreateCompatibleDC(NULL);
	CBitmap*old=dc.SelectObject(this);
	dc.BitBlt(0,0,width,height,NULL,0,0,mode);
	dc.SelectObject(old);
}

//msk has black forgrounds
void GBitmap::maskBlt(GBitmap*bmp,GBitmap*msk,int dx,int dy,int dw,int dh){
	CDC ddc,sdc;
	if(!dw && !dh){
		BITMAP info;
		bmp->GetObject(sizeof(BITMAP),&info);
		dw=info.bmWidth;
		dh=info.bmHeight;	
	}
	ddc.CreateCompatibleDC(NULL);
	sdc.CreateCompatibleDC(&ddc);
	CBitmap*oldd=ddc.SelectObject(this);
	CBitmap*olds=sdc.SelectObject(msk);
	ddc.BitBlt(dx,dy,dw,dh,&sdc,0,0,SRCAND);
	sdc.SelectObject(bmp);
	ddc.BitBlt(dx,dy,dw,dh,&sdc,0,0,SRCPAINT);
	ddc.SelectObject(oldd);
	sdc.SelectObject(olds);
}

void GBitmap::CopyBitmap(int dx,int dy,int dw,int dh,CBitmap*bmp,int mode){
	CDC ddc,sdc;
	ddc.CreateCompatibleDC(NULL);
	sdc.CreateCompatibleDC(&ddc);

	//CPalette*oldPal=ddc.SelectPalette(cPal,0);
	//ddc.RealizePalette();

	CBitmap*oldd=ddc.SelectObject(this);
	CBitmap*olds=sdc.SelectObject(bmp);
	BITMAP bm;
	if(!(dw+dh)){
		bmp->GetObject(sizeof(BITMAP),&bm);
		dw=bm.bmWidth;
		dh=bm.bmHeight;
	}
	
	ddc.BitBlt(dx,dy,dw,dh,&sdc,0,0,mode);
	//ddc.SelectPalette(oldPal,TRUE);
	ddc.SelectObject(oldd);
	sdc.SelectObject(olds);
}

void GBitmap::CopyBitmap(int dx,int dy,int dw,int dh,
	CBitmap*bmp,int sx,int sy,int sw,int sh,int mode){
	CDC ddc,sdc;
	ddc.CreateCompatibleDC(NULL);
	sdc.CreateCompatibleDC(&ddc);

	//CPalette*oldPal=ddc.SelectPalette(cPal,0);
	//ddc.RealizePalette();

	CBitmap*oldd=ddc.SelectObject(this);
	CBitmap*olds=sdc.SelectObject(bmp);
	if(!(dw+dh)){
		//this->GetObject(sizeof(BITMAP),&bm);
		dw=width;	//bm.bmWidth;
		dh=height;	//bm.bmHeight;
	}
	if(!(sw+sh)){
		BITMAP bm;
		bmp->GetObject(sizeof(BITMAP),&bm);
		sw=bm.bmWidth;
		sh=bm.bmHeight;
	}
	if(dw==sw&&dh==sh)
		ddc.BitBlt(dx,dy,dw,dh,&sdc,sx,sy,mode);
	else{
		ddc.SetStretchBltMode(STRETCH_DELETESCANS);
		ddc.StretchBlt(dx,dy,dw,dh,&sdc,sx,sy,sw,sh,mode);
	}
	//ddc.SelectPalette(oldPal,TRUE);
	ddc.SelectObject(oldd);
	sdc.SelectObject(olds);
}

void GBitmap::PaintBitmap(CDC*hdc,int dx,int dy,int dw,int dh,int sx,int sy,int sw,int sh,int mode){
	CDC sdc;
	sdc.CreateCompatibleDC(hdc);

	CBitmap*olds=sdc.SelectObject(this);
	//BITMAP bm;
	if(!(dw+dh)){
		//this->GetObject(sizeof(BITMAP),&bm);
		dw=width;	//bm.bmWidth;
		dh=height;	//bm.bmHeight;
	}
	if(!(sw+sh)){
		//this->GetObject(sizeof(BITMAP),&bm);
		sw=width;	//bm.bmWidth;
		sh=height;	//bm.bmHeight;
	}
	if(dw==sw&&dh==sh)
		hdc->BitBlt(dx,dy,dw,dh,&sdc,sx,sy,mode);
	else{
		hdc->SetStretchBltMode(STRETCH_DELETESCANS);
		hdc->StretchBlt(dx,dy,dw,dh,&sdc,sx,sy,sw,sh,mode);
	}
	sdc.SelectObject(olds);
}

void GBitmap::PaintBitmap(CDC*hdc,int dx,int dy,int dw,int dh,int sx,int sy,int mode){
		CDC sdc;
	sdc.CreateCompatibleDC(hdc);

	CBitmap*olds=sdc.SelectObject(this);
	//BITMAP bm;
	if(!(dw+dh)){
		//this->GetObject(sizeof(BITMAP),&bm);
		dw=width;	//bm.bmWidth;
		dh=height;	//bm.bmHeight;
	}
	
	hdc->BitBlt(dx,dy,dw,dh,&sdc,sx,sy,mode);
	sdc.SelectObject(olds);
}

void GBitmap::PaintBitmap(CDC*hdc,int dx,int dy,int dw,int dh,int mode){
		CDC sdc;
	sdc.CreateCompatibleDC(hdc);

	CBitmap*olds=sdc.SelectObject(this);
	//BITMAP bm;
	if(!(dw+dh)){
		//this->GetObject(sizeof(BITMAP),&bm);
		dw=width;	//bm.bmWidth;
		dh=height;	//bm.bmHeight;
	}
	
	hdc->BitBlt(dx,dy,dw,dh,&sdc,0,0,mode);
	sdc.SelectObject(olds);
}

//Copy a Bitmap from a DC
void GBitmap::GetBitmap(CDC* hdc,int dx,int dy,int dw,int dh,int sx,int sy,int sw,int sh,int mode){
	CDC sdc;
	sdc.CreateCompatibleDC(hdc);

	CBitmap*olds=sdc.SelectObject(this);
	//BITMAP bm;
	if(!(dw+dh)){
		//this->GetObject(sizeof(BITMAP),&bm);
		dw=width;	//bm.bmWidth;
		dh=height;	//bm.bmHeight;
	}
	if(!(sw+sh)){
		//this->GetObject(sizeof(BITMAP),&bm);
		sw=width;	//bm.bmWidth;
		sh=height;	//bm.bmHeight;
	}
	if(dw==sw&&dh==sh)
		sdc.BitBlt(dx,dy,dw,dh,hdc,sx,sy,mode);
	else{
		sdc.SetStretchBltMode(STRETCH_DELETESCANS);
		sdc.StretchBlt(dx,dy,dw,dh,hdc,sx,sy,sw,sh,mode);
	}
	sdc.SelectObject(olds);
}


UINT GBitmap::RealizePalette(int bk){
	CClientDC dc(AfxGetMainWnd());
	UINT num;
	CPalette*oldPal=NULL;
	oldPal=dc.SelectPalette(cPal,bk);
	num=dc.RealizePalette();
	if(oldPal)dc.SelectPalette(oldPal,TRUE);
	return num;
}

UINT GBitmap::RealizePalette(CWnd *wnd,int bk){
	CClientDC dc(wnd);
	UINT num;
	CPalette*oldPal=NULL;
	oldPal=dc.SelectPalette(cPal,bk);
	num=dc.RealizePalette();
	if(oldPal)dc.SelectPalette(oldPal,TRUE);
	return num;
}

UINT GBitmap::RealizePalette(CDC*hdc,int bk){
	CPalette*oldPal=NULL;
	UINT num;
	oldPal=hdc->SelectPalette(cPal,bk);
	num=hdc->RealizePalette();
	if(oldPal)hdc->SelectPalette(oldPal,TRUE);
	return num;
}

BOOL GBitmap::PaintDIB(CDC*hDC,int dx,int dy,int dw,int dh,LPSTR lpDIBHdr)
{
	BOOL     bSuccess=FALSE;      // Success/fail flag
	//HPALETTE hPal=NULL;           // Our DIB's palette
	//HPALETTE hOldPal=NULL;        // Previous palette

	/* Check for valid DIB handle */
	if (lpDIBHdr == NULL)return FALSE;

	/* Lock down the DIB, and get a pointer to the beginning of the bit
	 *  buffer
	 */

	// Get the DIB's palette, then select it into DC
	CPalette*hOldPal=NULL;
	if (cPal != NULL)
	{
		//hPal = (HPALETTE) pPal->m_hObject;
		// Select as background since we have
		// already realized in forground if needed
		hOldPal = hDC->SelectPalette(cPal,FALSE);//hDC->GetSafeHdc(), hPal, TRUE);
	}
	bSuccess = ::SetDIBitsToDevice(hDC->GetSafeHdc(),                    // hDC
								   dx,             // DestX
								   dy,              // DestY
								   dw,        // nDestWidth
								   dh,       // nDestHeight
								   0,            // SrcX
								   (int)DIBHeight((LPBITMAPINFOHEADER)lpDIBHdr)-dh,
								   0,                          // nStartScan
								   (WORD)DIBHeight((LPBITMAPINFOHEADER)lpDIBHdr),  // nNumScans
								   FindDIBBits((LPBITMAPINFOHEADER)lpDIBHdr),                  // lpBits
								   (LPBITMAPINFO)lpDIBHdr,     // lpBitsInfo
								   DIB_RGB_COLORS);            // wUsage
	if (hOldPal != NULL)
	{
		hDC->SelectPalette(hOldPal, TRUE);
	}

   return bSuccess;
}


BOOL GBitmap::PaintDIB(CDC     *hDC,
					int dx,int dy,int dw,int dh,
					LPSTR   lpDIBHdr,LPSTR lpDIBBits,
					int sx,int sy,int sw,int sh,
					CPalette* pPal)
{
	BOOL     bSuccess=FALSE;      // Success/fail flag
	//HPALETTE hPal=NULL;           // Our DIB's palette
	//HPALETTE hOldPal=NULL;        // Previous palette

	/* Check for valid DIB handle */
	if (lpDIBHdr == NULL)return FALSE;

	/* Lock down the DIB, and get a pointer to the beginning of the bit
	 *  buffer
	 */

	// Get the DIB's palette, then select it into DC
	CPalette*hOldPal=NULL;
	if (pPal != NULL)
	{
		//hPal = (HPALETTE) pPal->m_hObject;
		// Select as background since we have
		// already realized in forground if needed
		hOldPal = hDC->SelectPalette(pPal,FALSE);//hDC->GetSafeHdc(), hPal, TRUE);
	}
	
	/* Make sure to use the stretching mode best for color pictures */
	hDC->SetStretchBltMode(STRETCH_DELETESCANS);

	/* Determine whether to call StretchDIBits() or SetDIBitsToDevice() */
	if ((dw==sw) &&(dh==sh))
		bSuccess = ::SetDIBitsToDevice(hDC->GetSafeHdc(),                    // hDC
								   dx,             // DestX
								   dy,              // DestY
								   dw,        // nDestWidth
								   dh,       // nDestHeight
								   sx,            // SrcX
								   (int)DIBHeight((LPBITMAPINFOHEADER)lpDIBHdr) -
									  sy -
									  sh,   // SrcY
								   0,                          // nStartScan
								   (WORD)DIBHeight((LPBITMAPINFOHEADER)lpDIBHdr),  // nNumScans
								   lpDIBBits,                  // lpBits
								   (LPBITMAPINFO)lpDIBHdr,     // lpBitsInfo
								   DIB_RGB_COLORS);            // wUsage
   else
	  bSuccess = ::StretchDIBits(hDC->GetSafeHdc(),                          // hDC
							   dx,                 // DestX
							   dy,                  // DestY
							   dw,            // nDestWidth
							   dh,           // nDestHeight
							   sx,                // SrcX
							   sy,                 // SrcY
							   sw,           // wSrcWidth
							   sh,          // wSrcHeight
							   lpDIBBits,                      // lpBits
							   (LPBITMAPINFO)lpDIBHdr,         // lpBitsInfo
							   DIB_RGB_COLORS,                 // wUsage
							   SRCCOPY);                       // dwROP

	/* Reselect old palette */
	if (hOldPal != NULL)
	{
		hDC->SelectPalette(hOldPal, TRUE);
	}

   return bSuccess;
}

BOOL GBitmap::PaintDIB(CDC     *hDC,
					int dx,int dy,int dw,int dh,
					LPSTR   lpDIBHdr,
					int sx,int sy,int sw,int sh,
					CPalette* pPal)
{
	LPSTR    lpDIBBits;           // Pointer to DIB bits
	BOOL     bSuccess=FALSE;      // Success/fail flag
	//HPALETTE hPal=NULL;           // Our DIB's palette
	CPalette* hOldPal=NULL;        // Previous palette

	/* Check for valid DIB handle */
	if (lpDIBHdr == NULL)return FALSE;

	/* Lock down the DIB, and get a pointer to the beginning of the bit
	 *  buffer
	 */
	lpDIBBits =FindDIBBits((LPBITMAPINFOHEADER)lpDIBHdr);

	// Get the DIB's palette, then select it into DC
	if (pPal != NULL)
	{
		//hPal = (HPALETTE) pPal->m_hObject;
		// Select as background since we have
		// already realized in forground if needed
		hOldPal =hDC->SelectPalette(pPal,FALSE);
	}

	/* Make sure to use the stretching mode best for color pictures */
	hDC->SetStretchBltMode(STRETCH_DELETESCANS);

	/* Determine whether to call StretchDIBits() or SetDIBitsToDevice() */
	if ((dw==sw)&&(dh==sh))
		bSuccess = ::SetDIBitsToDevice(hDC->GetSafeHdc(),                    // hDC
								   dx,             // DestX
								   dy,              // DestY
								   dw,        // nDestWidth
								   dh,       // nDestHeight
								   sx,            // SrcX
								   (int)DIBHeight((LPBITMAPINFOHEADER)lpDIBHdr) -
									  sy -
									  sh,   // SrcY
								   0,                          // nStartScan
								   (WORD)DIBHeight((LPBITMAPINFOHEADER)lpDIBHdr),  // nNumScans
								   lpDIBBits,                  // lpBits
								   (LPBITMAPINFO)lpDIBHdr,     // lpBitsInfo
								   DIB_RGB_COLORS);            // wUsage
   else
	  bSuccess = ::StretchDIBits(hDC->GetSafeHdc(),                          // hDC
							   dx,                 // DestX
							   dy,                  // DestY
							   dw,            // nDestWidth
							   dh,           // nDestHeight
							   sx,                // SrcX
							   sy,                 // SrcY
							   sw,           // wSrcWidth
							   sh,          // wSrcHeight
							   lpDIBBits,                      // lpBits
							   (LPBITMAPINFO)lpDIBHdr,         // lpBitsInfo
							   DIB_RGB_COLORS,                 // wUsage
							   SRCCOPY);                       // dwROP

	/* Reselect old palette */
	if (hOldPal != NULL)
	{
		hDC->SelectPalette(hOldPal, TRUE);
	}

   return bSuccess;
}

/*************************************************************************
 *
 * CreateDIBPalette()
 *
 * Parameter:
 *
 * HDIB hDIB        - specifies the DIB
 *
 * Return Value:
 *
 * HPALETTE         - specifies the palette
 *
 * Description:
 *
 * This function creates a palette from a DIB by allocating memory for the
 * logical palette, reading and storing the colors from the DIB's color table
 * into the logical palette, creating a palette from this logical palette,
 * and then returning the palette's handle. This allows the DIB to be
 * displayed using the best possible colors (important for DIBs with 256 or
 * more colors).
 *
 ************************************************************************/


BOOL GBitmap::CreateDIBPalette(LPBITMAPINFO lpbi)
{
	LPLOGPALETTE lpPal;      // pointer to a logical palette
	int i;                   // loop index
	WORD wNumColors;         // number of colors in color table
	LPBITMAPINFO lpbmi;      // pointer to BITMAPINFO structure (Win3.0)
	LPBITMAPCOREINFO lpbmc;  // pointer to BITMAPCOREINFO structure (old)
	BOOL bWinStyleDIB;       // flag which signifies whether this is a Win3.0 DIB
	BOOL bResult = FALSE;

	/* if handle to DIB is invalid, return FALSE */

	if (lpbi == NULL)return FALSE;

   /* get pointer to BITMAPINFO (Win 3.0) */
   lpbmi = (LPBITMAPINFO)lpbi;

   /* get pointer to BITMAPCOREINFO (old 1.x) */
   lpbmc = (LPBITMAPCOREINFO)lpbi;

   /* get the number of colors in the DIB */
   wNumColors =DIBNumColors((LPBITMAPINFOHEADER)lpbi);

   if (wNumColors != 0)
   {
		lpPal=(LPLOGPALETTE)new char[sizeof(LOGPALETTE)
									+ sizeof(PALETTEENTRY)
									* wNumColors];

		if (lpPal == 0)return FALSE;

		/* set version and number of palette entries */
		lpPal->palVersion = PALVERSION;
		lpPal->palNumEntries = (WORD)wNumColors;

		/* is this a Win 3.0 DIB? */
		bWinStyleDIB = IS_WIN30_DIB(lpbi);
		for (i = 0; i < (int)wNumColors; i++)
		{
			if (bWinStyleDIB)
			{
				lpPal->palPalEntry[i].peRed = lpbmi->bmiColors[i].rgbRed;
				lpPal->palPalEntry[i].peGreen = lpbmi->bmiColors[i].rgbGreen;
				lpPal->palPalEntry[i].peBlue = lpbmi->bmiColors[i].rgbBlue;
				lpPal->palPalEntry[i].peFlags = 0;
			}
			else
			{
				lpPal->palPalEntry[i].peRed = lpbmc->bmciColors[i].rgbtRed;
				lpPal->palPalEntry[i].peGreen = lpbmc->bmciColors[i].rgbtGreen;
				lpPal->palPalEntry[i].peBlue = lpbmc->bmciColors[i].rgbtBlue;
				lpPal->palPalEntry[i].peFlags = 0;
			}
		}

		/* create the palette and get handle to it */
		if(cPal)delete cPal;
		cPal=new CPalette;
		bResult = cPal->CreatePalette(lpPal);
		delete [] (char*)lpPal;
	}
	return bResult;
}

//pal is of type rgbquad
BOOL GBitmap::CreateDIBPalette(WORD wNumColors,unsigned char *pal)
{
	LPLOGPALETTE lpPal;      // pointer to a logical palette
	int i;                   // loop index
	BOOL bResult = FALSE;

	lpPal=(LPLOGPALETTE)new char[sizeof(LOGPALETTE)
									+ sizeof(PALETTEENTRY)
									* wNumColors];

	if (lpPal == 0)return FALSE;

	/* set version and number of palette entries */
	lpPal->palVersion = PALVERSION;
	lpPal->palNumEntries = (WORD)wNumColors;

	for (i = 0; i < (int)wNumColors; i++){
				lpPal->palPalEntry[i].peRed = pal[i*4];
				lpPal->palPalEntry[i].peGreen = pal[i*4+1];
				lpPal->palPalEntry[i].peBlue = pal[i*4+2];
				lpPal->palPalEntry[i].peFlags = 0;
	}

	/* create the palette and get handle to it */
	int result=0;
	if(cPal)delete cPal;
	cPal=new CPalette;
	cPal->CreatePalette(lpPal);
	delete [](char*)lpPal;
	return result;
}

/*************************************************************************
 *
 * DIBWidth()
 *
 * Parameter:
 *
 * LPSTR lpbi       - pointer to packed-DIB memory block
 *
 * Return Value:
 *
 * DWORD            - width of the DIB
 *
 * Description:
 *
 * This function gets the width of the DIB from the BITMAPINFOHEADER
 * width field if it is a Windows 3.0-style DIB or from the BITMAPCOREHEADER
 * width field if it is an other-style DIB.
 *
 ************************************************************************/


DWORD GBitmap::DIBWidth(LPBITMAPINFOHEADER lpbmi)
{
	LPBITMAPCOREHEADER lpbmc;  // pointer to an other-style DIB

	/* point to the header (whether Win 3.0 and old) */

	lpbmc = (LPBITMAPCOREHEADER)lpbmi;

	/* return the DIB width if it is a Win 3.0 DIB */
	if (IS_WIN30_DIB(lpbmi))
		return lpbmi->biWidth;
	else  /* it is an other-style DIB, so return its width */
		return (DWORD)lpbmc->bcWidth;
}


/*************************************************************************
 *
 * DIBHeight()
 *
 * Parameter:
 *
 * LPSTR lpbi       - pointer to packed-DIB memory block
 *
 * Return Value:
 *
 * DWORD            - height of the DIB
 *
 * Description:
 *
 * This function gets the height of the DIB from the BITMAPINFOHEADER
 * height field if it is a Windows 3.0-style DIB or from the BITMAPCOREHEADER
 * height field if it is an other-style DIB.
 *
 ************************************************************************/

DWORD GBitmap::DIBHeight(LPBITMAPINFOHEADER lpbmi)
{
	LPBITMAPCOREHEADER lpbmc;  // pointer to an other-style DIB

	/* point to the header (whether old or Win 3.0 */
	lpbmc = (LPBITMAPCOREHEADER)lpbmi;

	/* return the DIB height if it is a Win 3.0 DIB */
	if (IS_WIN30_DIB(lpbmi))
		return lpbmi->biHeight;
	else  /* it is an other-style DIB, so return its height */
		return (DWORD)lpbmc->bcHeight;
}


/*************************************************************************
 *
 * PaletteSize()
 *
 * Parameter:
 *
 * LPSTR lpbi       - pointer to packed-DIB memory block
 *
 * Return Value:
 *
 * WORD             - size of the color palette of the DIB
 *
 * Description:
 *
 * This function gets the size required to store the DIB's palette by
 * multiplying the number of colors by the size of an RGBQUAD (for a
 * Windows 3.0-style DIB) or by the size of an RGBTRIPLE (for an other-
 * style DIB).
 *
 ************************************************************************/

WORD GBitmap::PaletteSize(LPBITMAPINFOHEADER lpbi)
{
   /* calculate the size required by the palette */
   if (IS_WIN30_DIB (lpbi))
	  return (WORD)(DIBNumColors(lpbi) * sizeof(RGBQUAD));
   else
	  return (WORD)(DIBNumColors(lpbi) * sizeof(RGBTRIPLE));
}


/*************************************************************************
 *
 * DIBNumColors()
 *
 * Parameter:
 *
 * LPSTR lpbi       - pointer to packed-DIB memory block
 *
 * Return Value:
 *
 * WORD             - number of colors in the color table
 *
 * Description:
 *
 * This function calculates the number of colors in the DIB's color table
 * by finding the bits per pixel for the DIB (whether Win3.0 or other-style
 * DIB). If bits per pixel is 1: colors=2, if 4: colors=16, if 8: colors=256,
 * if 24, no colors in color table.
 *
 ************************************************************************/
WORD GBitmap::DIBNumColors(LPBITMAPINFOHEADER lpbi)
{
	WORD wBitCount;  // DIB bit count

	/*  If this is a Windows-style DIB, the number of colors in the
	 *  color table can be less than the number of bits per pixel
	 *  allows for (i.e. lpbi->biClrUsed can be set to some value).
	 *  If this is the case, return the appropriate value.
	 */

	if (IS_WIN30_DIB(lpbi))
	{
		DWORD dwClrUsed;

		dwClrUsed = ((LPBITMAPINFOHEADER)lpbi)->biClrUsed;
		if (dwClrUsed != 0)
			return (WORD)dwClrUsed;
	}

	/*  Calculate the number of colors in the color table based on
	 *  the number of bits per pixel for the DIB.
	 */
	if (IS_WIN30_DIB(lpbi))
		wBitCount = ((LPBITMAPINFOHEADER)lpbi)->biBitCount;
	else
		wBitCount = ((LPBITMAPCOREHEADER)lpbi)->bcBitCount;

	/* return number of colors based on bits per pixel */
	switch (wBitCount)
	{
		case 1:
			return 2;

		case 4:
			return 16;

		case 8:
			return 256;

		default:
			return 0;
	}
}


BOOL GBitmap::ReadDIBFile(CFile &file)
{
	BITMAPFILEHEADER bmfHeader;
	DWORD dwBitsSize;
	LPSTR hbmp;
	int pos=file.GetPosition();
	/*
	 * get length of DIB in bytes for use when reading
	 */

	dwBitsSize = file.GetLength();

	/*
	 * Go read the DIB file header and check if it's valid.
	 */
	if (file.Read((LPSTR)&bmfHeader, sizeof(bmfHeader)) != sizeof(bmfHeader)){
		file.Seek(pos,CFile::begin);
		return NULL;
	}

	if (bmfHeader.bfType !=((WORD)('M' << 8) | 'B')){
		file.Seek(pos,CFile::begin);
		return NULL;
	}

	/*
	 * Allocate memory for DIB
	 */
	hbmp = new char[dwBitsSize];
	
	/*
	 * Go read the bits.
	 */
	if (file.ReadHuge(hbmp, dwBitsSize - sizeof(BITMAPFILEHEADER)) !=
		dwBitsSize - sizeof(BITMAPFILEHEADER) )
	{
		file.Seek(pos,CFile::begin);
		delete hbmp;
		return FALSE;
	}
	
	width=DIBWidth((LPBITMAPINFOHEADER)hbmp);
	height=DIBHeight((LPBITMAPINFOHEADER)hbmp);
	//this->CreateBitmap(width,height);
	//this->CreateDIBPalette((BITMAPINFO*)hbmp);
	this->CreateBitmap((BITMAPINFO*)hbmp);
	CDC dc;
	dc.CreateCompatibleDC(NULL);
	CPalette*oldPal=dc.SelectPalette(cPal,0);
	dc.RealizePalette();
	CBitmap*old=dc.SelectObject(this);
	PaintDIB(&dc,0,0,width,height,(char*)hbmp);
	dc.SelectObject(old);
	dc.SelectPalette(oldPal,TRUE);
	delete hbmp;
	return TRUE;
}

BOOL GBitmap::ReadDIBFile(CFile &file,LPSTR *hbmp)
{
	BITMAPFILEHEADER bmfHeader;
	DWORD dwBitsSize;
	int pos=file.GetPosition();
	/*
	 * get length of DIB in bytes for use when reading
	 */

	dwBitsSize = file.GetLength();

	/*
	 * Go read the DIB file header and check if it's valid.
	 */
	if (file.Read((LPSTR)&bmfHeader, sizeof(bmfHeader)) != sizeof(bmfHeader)){
		file.Seek(pos,CFile::begin);
		return NULL;
	}

	if (bmfHeader.bfType !=((WORD)('M' << 8) | 'B')){
		file.Seek(pos,CFile::begin);
		return NULL;
	}

	/*
	 * Allocate memory for DIB
	 */
	*hbmp = new char[dwBitsSize];
	
	/*
	 * Go read the bits.
	 */
	if (file.ReadHuge(*hbmp, dwBitsSize - sizeof(BITMAPFILEHEADER)) !=
		dwBitsSize - sizeof(BITMAPFILEHEADER) )
	{
		delete *hbmp;
		*hbmp=NULL;
		file.Seek(pos,CFile::begin);
		return FALSE;
	}
	
	width=DIBWidth((LPBITMAPINFOHEADER)(*hbmp));
	height=DIBHeight((LPBITMAPINFOHEADER)(*hbmp));
	//this->CreateBitmap(width,height);
	//this->CreateDIBPalette((BITMAPINFO*)(*hbmp));
	this->CreateBitmap((BITMAPINFO*)*hbmp);
	CDC dc;
	dc.CreateCompatibleDC(NULL);
	CPalette*oldPal=dc.SelectPalette(cPal,0);
	dc.RealizePalette();
	CBitmap*old=dc.SelectObject(this);
	PaintDIB(&dc,0,0,width,height,(char*)(*hbmp));
	dc.SelectObject(old);
	dc.SelectPalette(oldPal,TRUE);
	return TRUE;
}


BOOL GBitmap::WriteDIBFile(CFile &file,BITMAPINFOHEADER*header,RGBQUAD*palette,void *data)
{
	BITMAPFILEHEADER bmfHeader;
	bmfHeader.bfType =(WORD)('M' << 8) | 'B';
	bmfHeader.bfReserved1=0;
    bmfHeader.bfReserved2=0;
	bmfHeader.bfOffBits=sizeof(BITMAPFILEHEADER)+*(LPDWORD)header;
	bmfHeader.bfSize=bmfHeader.bfOffBits;

	if(!palette)palette=(RGBQUAD*)((LPSTR)header+*(LPDWORD)header);
	if(!data)data=(void*)FindDIBBits(header);
	if(header->biClrUsed==0){
		if(header->biBitCount<16)header->biClrUsed=(int)1<<(header->biBitCount);
	}

	bmfHeader.bfSize+=header->biClrUsed*sizeof(RGBQUAD);
	bmfHeader.bfOffBits=bmfHeader.bfSize;
	if(header->biSizeImage==0){
		header->biSizeImage=
		WIDTHBYTES(header->biWidth*header->biBitCount)*header->biHeight;
	}

	bmfHeader.bfSize+=header->biSizeImage;

	file.Write(&bmfHeader, sizeof(bmfHeader));

	file.Write(header,sizeof(BITMAPINFOHEADER));
	file.Write(palette,sizeof(RGBQUAD)*header->biClrUsed);

	file.WriteHuge(data,header->biSizeImage);

	return TRUE;
}

//color is 0,2,16, 256
void GBitmap::FillInfoHeader(BITMAPINFOHEADER*header,int width,int height,int bits,int color){
	int widthbytes;
	if(!width && !height){
		BITMAP bmp;
		this->GetObject(sizeof(BITMAP),&bmp);
		width=bmp.bmWidth;
		height=bmp.bmHeight;
		bits=bmp.bmBitsPixel;
		widthbytes=bmp.bmWidthBytes;
	}else widthbytes=WIDTHBYTES(width*bits);
	header->biClrUsed=color;
	header->biSize=sizeof(BITMAPINFOHEADER);
	header->biWidth=width;
	header->biHeight=height;
	header->biPlanes=1;
	header->biBitCount=bits;
	header->biCompression=BI_RGB;
	header->biSizeImage=widthbytes*height;
	header->biXPelsPerMeter=0;
	header->biYPelsPerMeter=0;
	if(!header->biClrUsed)
		if(header->biBitCount<16)header->biClrUsed=(int)1<<header->biBitCount;
	header->biClrImportant=header->biClrUsed;	
}

//Create from a common 8 bit ppm style data block
BOOL GBitmap::CreateBitmap8(int w,int h,unsigned char *dat,int colors,RGBQUAD *rgbs){
	int i,j;
	BITMAPINFOHEADER *bmpinfo;
	width=w;height=h;
	int widthb=WIDTHBYTES(width*8);
	bmpinfo=(BITMAPINFOHEADER*)new char[widthb*height+sizeof(BITMAPINFOHEADER)+colors*sizeof(RGBQUAD)];
	FillInfoHeader(bmpinfo,width,height,8,colors);

	if(rgbs){
		for(i=0;i<colors;i++){
			((BITMAPINFO*)bmpinfo)->bmiColors[i].rgbBlue=rgbs[i].rgbBlue;
			((BITMAPINFO*)bmpinfo)->bmiColors[i].rgbGreen=rgbs[i].rgbGreen;
			((BITMAPINFO*)bmpinfo)->bmiColors[i].rgbRed=rgbs[i].rgbRed;
			((BITMAPINFO*)bmpinfo)->bmiColors[i].rgbReserved=0;
		}
	}else
		for(i=0;i<colors;i++){
			((BITMAPINFO*)bmpinfo)->bmiColors[i].rgbBlue=
			((BITMAPINFO*)bmpinfo)->bmiColors[i].rgbGreen=
			((BITMAPINFO*)bmpinfo)->bmiColors[i].rgbRed=i;
			((BITMAPINFO*)bmpinfo)->bmiColors[i].rgbReserved=0;
		}

	unsigned char* buffer=(unsigned char*)FindDIBBits(bmpinfo);
	int at=0,atb=0;
	for(j=0;j<height;j++){
		for(i=0;i<widthb;i++){
			if(i<width)
				*(buffer+atb+i)=*(dat+at+i);
			else 
				*(buffer+atb+i)=0;
		}
		at+=width;
		atb+=widthb;
	}

	gLoadBitmap(bmpinfo);
	delete bmpinfo;
	return TRUE;
}

void GBitmap::bmp8Toppm(BITMAPINFOHEADER*bmpinfo,unsigned char *data,RGBQUAD *rgbs)
{
	int i,j,widthb;
	widthb=WIDTHBYTES(bmpinfo->biWidth*8);
	if(rgbs){
		for(i=0;i<(int)bmpinfo->biClrUsed;i++){
			rgbs[i].rgbBlue=((BITMAPINFO*)bmpinfo)->bmiColors[i].rgbBlue;
			rgbs[i].rgbGreen=((BITMAPINFO*)bmpinfo)->bmiColors[i].rgbGreen;
			rgbs[i].rgbRed=((BITMAPINFO*)bmpinfo)->bmiColors[i].rgbRed;
			rgbs[i].rgbReserved=((BITMAPINFO*)bmpinfo)->bmiColors[i].rgbReserved;
		}
	}

	unsigned char* buffer=(unsigned char*)FindDIBBits(bmpinfo);
	int at=0,atb=0;
	for(j=0;j<bmpinfo->biHeight;j++){
		for(i=0;i<bmpinfo->biWidth;i++){
			*(data+at+i)=*(buffer+atb+i);
		}
		at+=bmpinfo->biWidth;
		atb+=widthb;
	}

}
