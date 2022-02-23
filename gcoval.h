//	Author:		Gang Zhang
//	E-mail:		gzhang_ocx@yahoo.com or gazhang@geocities.com
//	HTTP:		http://members.xoom.com/gzhang
//	Date:		04/12/1996

#ifndef GDRAW_OVAL_HEADER_FILE
#define GDRAW_OVAL_HEADER_FILE

#ifndef GANG_DLL_EXPORT
#define GANG_DLL_EXPORT
#endif


class GANG_DLL_EXPORT gCOval{
protected:
	int tx;
	int x,y;
	virtual void drawpoint(int x,int y)=0;

	void gCOval::step(void);

	void gCOval::round(int r,int rx,int ry);
	
	void gCOval::draw(int x,int y,int rx,int ry);
};

#endif