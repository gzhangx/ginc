//============================================================================
//
//      fourierf.c  -  Don Cross <dcross@intersrv.com>
//	http://www.intersrv.com/~dcross/fft.html
//	refined by Gang Zhang, gzhang_ocx@yahoo.com or gazhang@geocities.com
//		http://members.xoom.com/gzhang
//
//============================================================================

#ifndef CFAST_FOURIER_TRANSFORM_HEADER
#define CFAST_FOURIER_TRANSFORM_HEADER


#ifndef GANG_DLL_EXPORT
#define GANG_DLL_EXPORT
#endif

//T is usually float or double, S might be int, char, float, double
#include <math.h>
#include <stdio.h>
template <class T>
class cfft{

protected:

static char is_power_of_two(const int num) {
	for(int i=0;i<sizeof(int)*8;i++){
		if(num==(1<<i))return 1;
	}
	return 0;
}

unsigned NumberOfBitsNeeded (const unsigned PowerOfTwo ) const
{
	unsigned i;
	for ( i=0; ; i++ )
	{
		if ( PowerOfTwo & (1 << i) )
			return i;
	}
}



unsigned ReverseBits (unsigned index, const unsigned NumBits ) const
{
	unsigned i, rev;
	for ( i=rev=0; i < NumBits; i++ )
	{
		rev = (rev << 1) | (index & 1);
		index >>= 1;
	}
	return rev;
}


inline void SWAP(T &a,T &b){
	T tmpswap=a;
	a=b;
	b=tmpswap;
}
public:
	cfft(){
	}

cfft( const unsigned NumSamples,
                 T *RealOut,
                 T *ImagOut,
		const int InverseTransform
		 )
{
	unsigned NumBits;    // Number of bits needed to store indices
	unsigned i, j, k, n;

	double angle_numerator = 2.0 * 3.14159265358979323846;
	double delta_angle;
	double alpha, beta;  // used in recurrence relation
	double delta_ar;
	double tr, ti;     // temp real, temp imaginary
	double ar, ai;     // angle vector real, angle vector imaginary

	if(InverseTransform)
	{
		angle_numerator = -angle_numerator;
	}

	NumBits = NumberOfBitsNeeded ( NumSamples );

	//Do simultaneous data copy and bit-reversal ordering into outputs...

	for ( i=1; i < NumSamples-1; i++ )
	{
		j = ReverseBits ( i, NumBits );
		if(j>i){
			SWAP(RealOut[j] , RealOut[i]);
			SWAP(ImagOut[j] , ImagOut[i]);
		}
	}

	//Do the FFT itself...

	unsigned BlockSize, BlockEnd;
	BlockEnd = 1;
	for ( BlockSize = 2; BlockSize <= NumSamples; BlockSize <<= 1 )
	{
		delta_angle = angle_numerator / (double)BlockSize;
		alpha = sin ( 0.5 * delta_angle );
		alpha = 2.0 * alpha * alpha;
		beta = sin ( delta_angle );

		for ( i=0; i < NumSamples; i += BlockSize )
		{
			ar = 1.0;   /* cos(0) */
			ai = 0.0;   /* sin(0) */

			for ( j=i, n=0; n < BlockEnd; j++, n++ )
			{
			k = j + BlockEnd;
			tr = ar*RealOut[k] - ai*ImagOut[k];
			ti = ar*ImagOut[k] + ai*RealOut[k];

			RealOut[k] = (T)(RealOut[j] - tr);
			ImagOut[k] = (T)(ImagOut[j] - ti);

			RealOut[j] += (T)tr;
			ImagOut[j] += (T)ti;

			delta_ar = alpha*ar + beta*ai;
			ai -= (alpha*ai - beta*ar);
			ar -= delta_ar;
			}
		}

		BlockEnd = BlockSize;
	}

	//Need to normalize if inverse transform...

	if ( InverseTransform )
	{
		T denom = (T)NumSamples;

		for ( i=0; i < NumSamples; i++ )
		{
			RealOut[i] /= denom;
			ImagOut[i] /= denom;
		}
	}
}


cfft(T *data,const int nn,const int InverseTransform){
	unsigned long n,mmax,m,j,istep,i;
	double wtemp,wr,wpr,wpi,wi,theta,twopi=6.28318530717959;
	T tempr,tempi;

	if(InverseTransform)twopi=-twopi;
	data--;
	n=nn << 1;
	j=1;
	for (i=1;i<n;i+=2) {
		if (j > i) {
			SWAP(data[j],data[i]);
			SWAP(data[j+1],data[i+1]);
		}
		m=nn;
		while (m >= 2 && j > m) {
			j -= m;
			m >>= 1;
		}
		j += m;
	}
	mmax=2;
	while (n > mmax) {
		istep=mmax << 1;
		theta=twopi/mmax;
		wtemp=sin(0.5*theta);
		wpr = -2.0*wtemp*wtemp;
		wpi=sin(theta);
		wr=1.0;
		wi=0.0;
		for (m=1;m<mmax;m+=2) {
			for (i=m;i<=n;i+=istep) {
				j=i+mmax;
				tempr=(T)(wr*data[j]-wi*data[j+1]);
				tempi=(T)(wr*data[j+1]+wi*data[j]);
				data[j]=data[i]-tempr;
				data[j+1]=data[i+1]-tempi;
				data[i] += tempr;
				data[i+1] += tempi;
			}
			wr=(wtemp=wr)*wpr-wi*wpi+wr;
			wi=wi*wpr+wtemp*wpi+wi;
		}
		mmax=istep;
	}

	if(InverseTransform){
		for(i=1;i<=n;i++){
			data[i]/=nn;
		}
	}
}

};

//throws char *
template<class T>
class cfft2d:public cfft<T>{
	public:
	//return nearst power of 2 of num
	//return 0 if too big
	static unsigned long power_of_two(const int num){
		for(int i=0;i<sizeof(int)*8;i++){
			int val=1<<i;
			if(val>=num) return val;
		}
		return 0;
	}

	cfft2d(T*rbuf,T*ibuf,const int w,const int h,const int InverseTransform){
		/*int w,h;
		w=tw;
		h=th;
		if(!is_power_of_two(w))w=power_of_two(tw);
		if(!is_power_of_two(h))h=power_of_two(th);
		if( (w==0) || (h==0) ) throw "image is too big";

		T *origr=NULL,*origi=NULL;
		if( (w!=tw) || (h!=th) ){
			origr=rbuf;
			origi=ibuf;
			rbuf=new T[w*h];
			if(rbuf==NULL)throw "Out of memory";
			ibuf=new T[w*h];
			if(ibuf==NULL){
				if(rbuf)delete rbuf;
				throw "Out of memory";
			}
			memset(rbuf,0,sizeof(T)*w*h);
			memset(ibuf,0,sizeof(T)*w*h);
			int wat=0,twat=0;
			for(int j=0;j<th;j++){
				for(int i=0;i<tw;i++){
					rbuf[wat+i]=origr[twat+i];
					ibuf[wat+i]=origi[twat+i];
				}
				twat+=tw;
				wat+=w;
			}

		}
		*/
		if(!is_power_of_two(w))throw "width not power of 2";
		if(!is_power_of_two(h))throw "heigh not power of 2";
		T *rline=NULL,*iline=NULL;
		rline=new T[h];
		if(rline==NULL)throw "Out of memory";
		iline=new T[h];
		if(iline==NULL){
			if(rline)delete rline;
			throw "Out of memory";
		}


		int at=0;
		for(int i=0;i<h;i++){
			cfft<T>(w,rbuf+at,ibuf+at,InverseTransform);
			at+=w;
		}
		for(i=0;i<w;i++){
			at=i;
			for(int j=0;j<h;j++){
				rline[j]=rbuf[at];
				iline[j]=ibuf[at];
				at+=w;
			}
			cfft<T>(h,rline,iline,InverseTransform);
			at=i;
			for(j=0;j<h;j++){
				rbuf[at]=rline[j];
				ibuf[at]=iline[j];
				at+=w;
			}
		}
		delete rline;
		delete iline;

		/*
		if( (w!=tw) || (h!=th) ){
			int twat=0,wat=0;
			for(int j=0;j<th;j++){
				for(int i=0;i<tw;i++){
					origr[twat+i]=rbuf[wat+i];
					origi[twat+i]=ibuf[wat+i];
				}
				twat+=tw;
				wat+=w;
			}
			delete rbuf;
			delete ibuf;
			//rbuf=origr;
			//ibuf=origi;
		}
		*/
	}
};


//prepare a 2d image to be fit to be ffted
//after call the constructor, use getw(), geth() and getr() geti()
//to get the new image, after fft, use unprep() to fill the original buffer
//with the new img.
template <class T>
class Cprep_fft2d{
protected:
	T *origr,*origi;
	T *currentr,*currenti;
	int tw,th;
	int neww,newh;
public:
	int gettw() const {return tw;}
	int getth() const {return th;}
	int getw() const {return neww;}
	int geth() const {return newh;}
	T* getr() const {return currentr;}
	T* geti() const {return currenti;}

	//return nearst power of 2 of num
	//return 0 if too big
	static unsigned long power_of_two(const int num){
		for(int i=0;i<sizeof(int)*8;i++){
			int val=1<<i;
			if(val>=num) return val;
		}
		return 0;
	}

	Cprep_fft2d(T*rbuf,T*ibuf,const int ow,const int oh){
		origr=NULL;
		origi=NULL;
		currentr=NULL;
		currenti=NULL;
		tw=ow;
		th=oh;
		int w,h;
		w=tw;
		h=th;
		if(!is_power_of_two(w))w=power_of_two(tw);
		if(!is_power_of_two(h))h=power_of_two(th);
		neww=w;
		newh=h;
		if( (w==0) || (h==0) ) throw "image is too big";

		if( (w!=tw) || (h!=th) ){
			origr=rbuf;
			origi=ibuf;
			currentr=rbuf=new T[w*h];
			if(rbuf==NULL)throw "Out of memory";
			currenti=ibuf=new T[w*h];
			if(ibuf==NULL){
				if(rbuf)delete rbuf;
				currentr=currenti=NULL;
				throw "Out of memory";
			}
			memset(rbuf,0,sizeof(T)*w*h);
			memset(ibuf,0,sizeof(T)*w*h);
			int wat=0,twat=0;
			for(int j=0;j<th;j++){
				for(int i=0;i<tw;i++){
					rbuf[wat+i]=origr[twat+i];
					ibuf[wat+i]=origi[twat+i];
				}
				twat+=tw;
				wat+=w;
			}

			wat=0;
			for(j=0;j<w;j++){
				for(int i=0;i<h;i++){
					if((j+i)%2==1){
						rbuf[wat+i]*=-1;
						ibuf[wat+i]*=-1;
					}
				}
				wat+=w;
			}
		}
	}

	~Cprep_fft2d(){
		if(currentr)delete currentr;
		if(currenti)delete currenti;
		currentr=NULL;
		currenti=NULL;
	}

	void unprep(int del=0){
		if( (currentr==NULL) || (currenti==NULL) )throw "Already unprepared";
		int w,h;
		w=tw;
		h=th;
		if( (w!=tw) || (h!=th) ){
			int twat=0,wat=0;
			for(int j=0;j<th;j++){
				for(int i=0;i<tw;i++){
					origr[twat+i]=currentr[wat+i];
					origi[twat+i]=currenti[wat+i];
				}
				twat+=tw;
				wat+=w;
			}
			if(del){
				if(currentr)delete currentr;
				if(currenti)delete currenti;
				currentr=NULL;
				currenti=NULL;
			}
		}
	}
	static char is_power_of_two(const int num){
	for(int i=0;i<sizeof(int)*8;i++){
		if(num==(1<<i))return 1;
	}
	return 0;
}

};

class GANG_DLL_EXPORT gCHistogram_equ256{
protected:
	int clry[256];
public:
	gCHistogram_equ256(unsigned char *buf,const int w,const int h);
};

#endif
