//	Author:		Gang Zhang
//	E-mail:		gzhang_ocx@yahoo.com or gazhang@geocities.com
//	HTTP:		http://members.xoom.com/gzhang
//	Date:		1995

#include <gcsid.h>
#include <stdlib.h>


/*int main(int argv,char**argc){
  RealImg img;
  if(argv!=3)return 0;
  if(readpgm256(*(argc+1),&img)){
    printf("Read file failed!\n");
    return 0;
  }
  printf("Begin\n");
  printf("Processing\n");
  DrawAutoStereogram(&img);  
  printf("done sirds\n");
  writepgm256R(*(argc+2),&img,"SIRDS Created by Gang");
  printf("Done!\n");
  free(img.pic);
  return 0;
}
*/


void gCSid::DrawStereogram(char *imgptr,int imgw,int imgh) 
{                               
  long x,y,at,aty;               
  long *same;
  long s;
  long left, right ,curleft;
 
  same=new long[imgw];

  for( at=0,aty=0,y = 0; y <imgh ; y++,aty+=imgw ) {
    /* initialise the links */    
    for( x = 0; x <imgw; x++ ) *(same+x) = x;

    /* calculate the links for the Z[][] object */    
    for( x = 0; x <imgw; x++,at++ ) {
      s = separation(*(imgptr+at));
      left = x - (s/2);
      right = left + s;        
      if( 0 <= left && right < imgw ){
        while(1){
          curleft=*(same+right);
          if(curleft<left){
           right=left;
           left=curleft;
          }else break;
        } 
	      *(same+right) =left; 
      }
    }
    /* assign the colors */
    for( x =0; x<imgw; x++) {
      if( *(same+x) == x ) *(imgptr+aty+x) = rand()&0xff;
      else *(imgptr+aty+x) =*(imgptr+aty+*(same+x)); 
      //if(*(same+x) != x )*(img->pic+aty+x) =*(img->pic+aty+*(same+x));
    }
  }
  delete same;
}
