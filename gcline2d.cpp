//////////////////////////////////////////////////////////////////////////
//
//	Program:	gcline2d.cpp
//	DESC:		line and oval functions
//	Author:		Gang Zhang
//	E-mail:		gzhang_ocx@yahoo.com or gazhang@geocities.com
//	HTTP:		http://members.xoom.com/gzhang
//	Date:		Feb 16, 1998
//	Revisions:
//////////////////////////////////////////////////////////////////////////

#include <gcline2d.h>
#include <gcoval.h>

	void gCline2d::gline(int x0,int y0,int x1,int y1){
		tx=0;
		dx=x1-x0;
		dy=y1-y0;
		x=x0;
		y=y0;
		xsign=1;if(dx<0){xsign=-1;dx=-dx;}
		ysign=1;if(dy<0){ysign=-1;dy=-dy;}

		if(y0!=y1){
			while( (y!=y1) ){
				step();
				drawpoint(x,y);
			}
		}else{
			while( (x!=x1) ){
				step();
				drawpoint(x,y);
			}
		}
		while(y!=y1){y+=ysign;drawpoint(x,y);}
		while(x!=x1){x+=xsign;drawpoint(x,y);}
	}

	void gCline2d::step(void){
		if(dx==0)y+=ysign;
		else if(dy==0)x+=xsign;
		else if(tx>dy){
			tx-=dy;
			x+=xsign;
		}else{
			tx+=dx;
			y+=ysign;
		}
	}


	void gCOval::step(void){
		if(tx>y){
			tx-=y;
			y--;
		}else{
			x++;
			tx+=x;
		}
	}

	void gCOval::round(int r,int rx,int ry){
		tx=x=0;y=r;
		while(y){
			step();
			draw(x,y,rx,ry);
		}
	}
	
	void gCOval::draw(int x,int y,int rx,int ry){
		drawpoint(rx+x,	ry+y);
		drawpoint(rx-x,	ry-y);
		drawpoint(rx+x,	ry-y);
		drawpoint(rx-x,	ry+y);
	}
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

