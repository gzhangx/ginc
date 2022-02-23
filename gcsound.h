//////////////////////////////////////////////////////////////
//
//File name:	gsnd.h
//Author:		Gang Zhang (gzhang@icx.net)
//	E-mail:		gzhang_ocx@yahoo.com or gazhang@geocities.com
//	HTTP:		http://members.xoom.com/gzhang
//Date:			Jun 27,1998
//Version:		1.0
//Note:			the 3 classes here wrap around windows sound
//				layers.  gWaveHeader class is the class for 
//				wave header to be prepared.
//				Must queueu up to more then one buffer in the
//				wave device to play or record smoothly
//Libraries:	winmm.lib
//////////////////////////////////////////////////////////////
#ifndef GSND_HEADER_FILE
#define GSND_HEADER_FILE
#include <gerror.h>
#include <mmsystem.h>

/*************************************************************
**************************************************************
#include <gcsound.h>
#include <stdio.h>

#define BUFS 16
#define BUFLEN 2048
void example_func(){
	int datalen=BUFLEN;
	gWaveHeader* wh[BUFS];
	char *data;
	MMRESULT mres;

	for(int i=0;i<BUFS;i++){
		data=new char[datalen];
		if(data==NULL)break;
		wh[i]=new gWaveHeader(data,datalen);
	}
	try{
		gCSoundIn *Gi=new gCSoundIn(0);
		Gi->open();
		for(i=0;i<BUFS;i++){
			Gi->add_buf(wh[i]);
			//Gi->wait();
		}
		printf("begin recording\n");
		Gi->start();
		printf("waiting\n");

		for(i=0;i<BUFS;i++){
			Gi->wait();
			printf("done wait\n");
			//wh[i]->get_pwh()->reserved=0;
			if(wh[i]->get_pwh()->dwFlags & WHDR_DONE ){
				printf("done\n");
			}
			
			if(wh[i]->get_pwh()->dwFlags & WHDR_PREPARED ){
				printf("prepared\n");
			}
			if(wh[i]->get_pwh()->dwFlags & WHDR_ENDLOOP ){
				printf("endloop\n");
			}

			//mres=waveInUnprepareHeader(Gi->hwi,&wh[i]->mpwh, sizeof(WAVEHDR));
			mres=Gi->remove_buf(wh[i]);
			if(mres!=MMSYSERR_NOERROR){
				char errtxt[512];
				errtxt[0]=0;
				waveInGetErrorText(mres,errtxt,512);
				printf("remove buffer error: %s\n",errtxt);
			}
		}
		printf("done recording\n");
		Gi->pause();

		delete Gi;
	}catch(gSound_error_str e){
		printf(e.get_str());
	}

	//FILE *fp=fopen("out.bin","wb");
	//if(fp){
	//	for(i=0;i<BUFS;i++){
	//		unsigned char *data=(unsigned char*)wh[i]->get_data();
	//		fwrite(data,1,datalen,fp);
	//	}
	//	fclose(fp);
	//}

	for(i=0;i<BUFS;i++){
		unsigned char *data=(unsigned char*)wh[i]->get_data();
		for(int j=0;j<datalen;j++){
			data[j]&=0xf0;
			if(data[j]>0x80){
				data[j]-=0x80;
				data[j]>>2;
				data[j]+=0x80;
			}else{
				data[j]=0x80-data[j];
				data[j]>>2;
				data[j]=0x80-data[j];
			}
		}
	}

	for(i=0;i<BUFS;i++){
		unsigned char *data=(unsigned char*)wh[i]->get_data();
		for(int j=1;j<datalen;j++){
		}
	}
////////////////////////////////////////
printf("playing \n");
	try{
		gCSoundOut *Go=new gCSoundOut(0);
		Go->open();


		for(i=0;i<BUFS;i++){
			Go->add_buf(wh[i]);
			Go->play(wh[i]);
		}

		Go->wait();

		while(! (wh[BUFS-1]->get_pwh()->dwFlags & WHDR_DONE)){
			Sleep(1000);
		}
		for(i=0;i<BUFS;i++){
			mres=Go->remove_buf(wh[i]);
			if(mres!=MMSYSERR_NOERROR){
				char errtxt[512];
				errtxt[0]=0;
				waveOutGetErrorText(mres,errtxt,512);
				printf("remove buffer error: %s\n",errtxt);
			}
		}
		printf("done playinging\n");
		Go->pause();
		delete Go;
	}catch(gSound_error_str e){
		printf(e.get_str());
	}

	for(i=0;i<BUFS;i++){
		delete wh[i];
	}
}
**************************************************************
**************************************************************/
/////////////////////////////////////////////


#ifndef GANG_DLL_EXPORT
#define GANG_DLL_EXPORT
#endif

class GANG_DLL_EXPORT gSound_error_str:public gerror_str{
protected:
	virtual void init(){
		set_class("gSound_error_str");
	}
public:
	gSound_error_str(const char *str1=NULL,const GERROR_CODES err_num=ERR_OK):gerror_str(str1,err_num){
		init();
	}
	gSound_error_str(const gerror_str &err):gerror_str(err){
		init();
	}
};


class GANG_DLL_EXPORT gWaveHeader{
protected:
		WAVEHDR mpwh;
public:
	void init_buf(int loops=1);
	void add_buf(char *data,int datalen,int loops=1);
	gWaveHeader();
	gWaveHeader(char *data,int len,int loop=1);
	~gWaveHeader();

	WAVEHDR *get_pwh() {return &mpwh;}
	char *get_data() const {return mpwh.lpData;}
	int get_len() const {return mpwh.dwBufferLength;}
};


//absact sound class
class GANG_DLL_EXPORT gCSound{
protected:
	HANDLE m_finish_event;
	MMRESULT m_mres;
	UINT m_device_num;
	BOOL m_opened;
	char m_errtxt[512];
	WAVEFORMATEX m_pwfx;
public:
	void set_sound_format(const int nchannel=1,const int khz=8000,const int bitspresample=8);
	UINT get_dev() const {
		return m_device_num;
	}
	BOOL is_open() const {
		return m_opened;
	}
	virtual int numDevs() const =0;
	virtual int get_channels() const =0;
	virtual int get_formats() const =0;
	virtual void pause()=0;
	virtual void close()=0;
	virtual void reset()=0;
};

class GANG_DLL_EXPORT gCSoundIn:public gCSound{
protected:
	WAVEINCAPS m_pwic;
	HWAVEIN m_hwi;
public:
	int numDevs() const {
		return waveInGetNumDevs();
	}

	gCSoundIn(int dev=WAVE_MAPPER);
	char *get_dev_name() const {
		return (char*)m_pwic.szPname;
	}
	int get_channels() const {
		return m_pwic.wChannels;
	}
	int get_formats() const {
		return m_pwic.dwFormats;
	}

	void open(int nchannel=1,int khz=8000,int bits=8);

	
	void reset();

	void add_buf(gWaveHeader *hdr);

	MMRESULT remove_buf(gWaveHeader *hdr);

	DWORD wait(DWORD msec=INFINITE);

	void start();
	void pause();
	void close();

	~gCSoundIn();
};


class GANG_DLL_EXPORT gCSoundOut : public gCSound{
protected:
	WAVEOUTCAPS m_pwoc;
	HWAVEOUT m_hwo;
public:
	int numDevs() const {
		return waveOutGetNumDevs();
	}

	gCSoundOut(UINT dev=WAVE_MAPPER );
	char *get_dev_name() const {
		return (char*)m_pwoc.szPname;
	}
	int get_channels() const {
		return m_pwoc.wChannels;
	}
	int get_formats() const {
		return m_pwoc.dwFormats;
	}

	void open(int nchannel=1,int khz=8000,int bits=8);

	void add_buf(gWaveHeader *hdr);
	
	//waveOutGetPosition()
	void play(gWaveHeader *hdr);

	void reset();

	MMRESULT remove_buf(gWaveHeader *hdr);

	DWORD wait(DWORD msec=INFINITE);

	void pause();

	void close();

	void get_vol(DWORD *vol);

	void set_vol(DWORD vol);

	~gCSoundOut();

};


#endif
