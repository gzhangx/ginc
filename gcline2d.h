//////////////////////////////////////////////////////////////////////////
//
//	Program:	gcline2d.h
//	DESC:		line and oval functions
//	Author:		Gang Zhang
//	E-mail:		gzhang_ocx@yahoo.com or gazhang@geocities.com
//	HTTP:		http://members.xoom.com/gzhang
//	Date:		Feb 16, 1998
//	Revisions:
//////////////////////////////////////////////////////////////////////////


#ifndef GLINE2D_HEADER_FILE
#define GLINE2D_HEADER_FILE

#ifndef GANG_DLL_EXPORT
#define GANG_DLL_EXPORT
#endif


class GANG_DLL_EXPORT gCline2d{
protected:
	int tx;
	int dx,dy;
	int xsign,ysign;
	int x,y;
public:
	virtual void drawpoint(int x,int y)=0;
	void gline(int x0,int y0,int x1,int y1);

	void step(void);
};

/*
class line2dwin:public gline2d{
	CDC *pDC;
	void dawpoint(int x,int y){
		pDC->MoveTo(x,y);
		pDC->LineTo(x+1,y+1);
	}
public:
	line2dwin(int x0,int y0,int x1,int y1,CDC *pdc){
		pDC=pdc;
		gline(x0,y0,x1,y1);
	}
};
*/

template <class T>
class line2cbuf:public gCline2d{
protected:
	T *buf;
	int w,h,truew;
	void drawpoint(int x,int y){
		if( (x>=w) || (x<0) )return;
		if( (y>=h) || (y<0) )return;
		*(buf+y*truew+x)=(T)255;
	}
public:
	line2cbuf(){};
	line2cbuf(int x0,int y0,int x1,int y1,T *b,int wi,int hi){
		setup(b,wi,hi);
		gline(x0,y0,x1,y1);
	}
	line2cbuf(int x0,int y0,int x1,int y1,T *b,int wi,int hi,int truewi){
		setup(b,wi,hi,truewi);
		gline(x0,y0,x1,y1);
	}
	void setup(T *b,int wi,int hi){
		setup(b,wi,hi,wi);
	}
	void setup(T *b,int wi,int hi,int truewi){
		buf=b;
		truew=truewi;
		w=wi;
		h=hi;
	}
	void line(int x0,int y0,int x1,int y1){
		gline(x0,y0,x1,y1);
	}
};

#endif
