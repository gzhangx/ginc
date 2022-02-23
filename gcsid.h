//////////////////////////////////////////////////////////////////////////
//
//	Program:	gcsid.h
//	DESC:		implement SIDRS function
//	Author:		Gang Zhang
//	E-mail:		gzhang_ocx@yahoo.com or gazhang@geocities.com
//	HTTP:		http://members.xoom.com/gzhang
//	Date:		1995
//	Revisions:
//////////////////////////////////////////////////////////////////////////

#ifndef GCSIDRS_HEADERFILE
#define GCSIDRS_HEADERFILE

#include <stdio.h>
#include <math.h>

#ifndef GANG_DLL_EXPORT
#define GANG_DLL_EXPORT
#endif


/*int main(int argv,char**argc){
  RealImg img;
  if(argv!=3)return 0;
  if(readpgm256(*(argc+1),&img)){
    printf("Read file failed!\n");
    return 0;
  }
  printf("Begin\n");
  printf("Processing\n");
  gCSid cs;
  cs.DrawStereogram(img.pic,img.x,img.y);  
  printf("done sirds\n");
  writepgm256R(*(argc+2),&img,"SIRDS Created by Gang");
  printf("Done!\n");
  free(img.pic);
  return 0;
}
*/

typedef float Rationalfloat;

class GANG_DLL_EXPORT gCSid{
	Rationalfloat m_VD;	//(300.0*pmm)  Viewing distance from screen (in pixels) 
	Rationalfloat m_ES;	//(65.0*pmm)   Eye separation 
	Rationalfloat m_OS;	//(150.0*pmm)  Offset into screen 

	long round(float X) {return (long)((X)+0.5); } 
	long separation(float Z) {return round((m_OS-Z)*m_ES/(m_OS+m_VD-Z));}

public:
	gCSid(Rationalfloat pmm=.656){
		pmm = (Rationalfloat)0.656;
		m_VD =(Rationalfloat)(300.0*pmm);  //Viewing distance from screen (in pixels) 
		m_ES =(Rationalfloat)(65.0*pmm);   //Eye separation 
		m_OS =(Rationalfloat)(150.0*pmm);  //Offset into screen 
	}

	void set_dist(Rationalfloat VD,Rationalfloat ES,Rationalfloat OS,Rationalfloat ppm=.656){
		m_VD=VD*ppm;
		m_ES=ES*ppm;
		m_OS=OS*ppm;
	}

	void DrawStereogram(char *imgptr,int imgw,int imgh);
};


#endif