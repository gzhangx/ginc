//============================================================================
//
//      gcfft.cpp  -  Don Cross <dcross@intersrv.com>
//	http://www.intersrv.com/~dcross/fft.html
//	refined by Gang Zhang
//	E-mail:		gzhang_ocx@yahoo.com or gazhang@geocities.com
//	HTTP:		http://members.xoom.com/gzhang
//
//============================================================================

#include <gcfft.h>
#include <memory.h>
	gCHistogram_equ256::gCHistogram_equ256(unsigned char *buf,const int w,const int h){
		memset(clry,0,sizeof(clry));
		int total=w*h;
		int big=0;
		for(int i=0;i<total;i++){
			clry[buf[i]]++;
		}
		for(i=1;i<256;i++){
			clry[i]+=clry[i-1];
		}
		//int fac=255*total/big;
		for(i=0;i<total;i++){
			buf[i]=255*clry[buf[i]]/total;
		}
	}


extern "C"{

void GANG_DLL_EXPORT gfft_d( unsigned NumSamples,
                 double *RealOut,
                 double *ImagOut,
		int InverseTransform
		 ){
	cfft<double> fft(NumSamples,RealOut,ImagOut,InverseTransform);
}

void GANG_DLL_EXPORT gfft_f( unsigned NumSamples,
                 float *RealOut,
                 float *ImagOut,
		int InverseTransform
		 ){
	cfft<float> fft(NumSamples,RealOut,ImagOut,InverseTransform);
}

void GANG_DLL_EXPORT cfft_d(double *data,int nn, int InverseTransform){
	cfft<double> fft(data,nn,InverseTransform);
}

void GANG_DLL_EXPORT cfft_f(float *data,int nn, int InverseTransform){
	cfft<float> fft(data,nn,InverseTransform);
}


void GANG_DLL_EXPORT cfft2d_f(float *rbuf,float *ibuf,int tw,int th,int InverseTransform){
	cfft2d<float> fft2df(rbuf,ibuf,tw,th,InverseTransform);
}

void GANG_DLL_EXPORT cfft2d_d(double *rbuf,double *ibuf,int tw,int th,int InverseTransform){
	cfft2d<double> fft2df(rbuf,ibuf,tw,th,InverseTransform);
}

}

