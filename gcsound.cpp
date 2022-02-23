//////////////////////////////////////////////////////////////
//
//	File name:	gcsound.cpp
//	Author:		Gang Zhang
//	E-mail:		gzhang_ocx@yahoo.com or gazhang@geocities.com
//	HTTP:		http://members.xoom.com/gzhang
//	Date:			Jun 27,1998
//	Version:		1.0
//	Note:			the 3 classes here wrap around windows sound
//				layers.  gWaveHeader class is the class for 
//				wave header to be prepared.
//				Must queueu up to more then one buffer in the
//				wave device to play or record smoothly
//Libraries:	winmm.lib
//////////////////////////////////////////////////////////////

#include <gcsound.h>

/*************************************************************
**************************************************************
void example_func(){
#define BUFS 16
	int datalen=2048;
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
		Gi->stop();

		delete Gi;
	}catch(gSound_error_str e){
		printf(e.getstr());
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
		printf(e.getstr());
	}

	for(i=0;i<BUFS;i++){
		delete wh[i];
	}
}
**************************************************************
**************************************************************/
/////////////////////////////////////////////




	void gWaveHeader::init_buf(int loops){
			mpwh.dwBytesRecorded=0;
			mpwh.dwUser=0;
			mpwh.dwFlags=0;
			mpwh.dwLoops=loops;
			mpwh.lpNext=NULL;
			mpwh.reserved=0;
	}
		void gWaveHeader::add_buf(char *data,int datalen,int loops){
			mpwh.lpData=data;
			mpwh.dwBufferLength=datalen; 
			init_buf(loops);
		}
		gWaveHeader::gWaveHeader(){
			add_buf(NULL,0);
		}
		gWaveHeader::gWaveHeader(char *data,int len,int loop){
			add_buf(data,len,loop);
		}
		gWaveHeader::~gWaveHeader(){
			if(mpwh.lpData)delete mpwh.lpData;
		}



	void gCSound::set_sound_format(const int nchannel,const int khz,const int bitspresample)
	{
	m_pwfx.wFormatTag=WAVE_FORMAT_PCM;
	m_pwfx.nChannels=nchannel;	//1
	m_pwfx.nSamplesPerSec=khz;	//8000 (8KHz)
	m_pwfx.wBitsPerSample=bitspresample;	//8
	m_pwfx.nBlockAlign=m_pwfx.nChannels*m_pwfx.wBitsPerSample/8;
	m_pwfx.nAvgBytesPerSec=m_pwfx.nSamplesPerSec*m_pwfx.nBlockAlign;
	m_pwfx.cbSize=sizeof(m_pwfx);
	}

	
	gCSoundIn::gCSoundIn(int dev){
		m_device_num=dev;
		m_opened=FALSE;
		//WAVEINCAPS pwic;
		m_pwic.szPname[0]=0; 
		m_mres=waveInGetDevCaps(dev,&m_pwic,sizeof(m_pwic));
		m_pwic.szPname[MAXPNAMELEN-1]=0;
		if(m_mres!=MMSYSERR_NOERROR){
			m_pwic.szPname[0]=0;
			m_pwic.wChannels=0;
			m_pwic.dwFormats=0;
			switch(m_mres){
			case MMSYSERR_NODRIVER:
				throw gSound_error_str("No driver present",ERR_MMSYS_NODRIVER);
			case MMSYSERR_BADDEVICEID:
				throw gSound_error_str("Bad device id",ERR_MMSYS_BADID);
			default:
				throw gSound_error_str("Out of memory",ERR_NO_MEM);
			}
		}

		set_sound_format(1,8000,8);

		m_finish_event=CreateEvent(NULL,FALSE,FALSE,NULL);
		if(m_finish_event==NULL)throw gSound_error_str("Can't create event");
	}
	

	void gCSoundIn::open(int nchannel,int khz,int bits){
		set_sound_format(nchannel,khz,bits);

		m_mres=waveInOpen( &m_hwi,m_device_num,&m_pwfx, 
				(DWORD_PTR)m_finish_event,
				0,CALLBACK_EVENT );//WAVE_FORMAT_DIRECT);
		
		if(m_mres!=MMSYSERR_NOERROR){
			m_errtxt[0]=0;
			waveInGetErrorText(m_mres,m_errtxt,512);
			throw gSound_error_str("wavinopen err: ",ERR_MMSYS_DEVOPEN)+gSound_error_str(m_errtxt);
		}
		m_opened=TRUE;
	}

	
	void gCSoundIn::reset(){
		if(!m_opened){
			throw gSound_error_str("Wave Out Device not open: ",ERR_MMSYS_DEVNOTOPEN)<<m_device_num;
		}
		m_mres=waveInReset(m_hwi); 
		if(m_mres!=MMSYSERR_NOERROR){
			m_errtxt[0]=0;
			waveInGetErrorText(m_mres,m_errtxt,512);
			throw gSound_error_str("reset error: ",ERR_MMSYS_RESET)<<m_errtxt;
		}
	}

	void gCSoundIn::add_buf(gWaveHeader *hdr){
		if(!m_opened){
			throw gSound_error_str("Wave Out Device not open: ",ERR_MMSYS_DEVNOTOPEN)<<m_device_num;
		}
		//the reserved field is changed after play, must reset
		hdr->init_buf();
		m_mres=waveInPrepareHeader(m_hwi, hdr->get_pwh(),sizeof(WAVEHDR));
		if(m_mres!=MMSYSERR_NOERROR){
			m_errtxt[0]=0;
			waveInGetErrorText(m_mres,m_errtxt,512);
			throw gSound_error_str("Sound in prepreare header error: ",ERR_MMSYS_PREPHDR)<<m_errtxt;
		}

		m_mres=waveInAddBuffer(m_hwi, hdr->get_pwh(), sizeof(WAVEHDR));
		if(m_mres!=MMSYSERR_NOERROR){
			m_errtxt[0]=0;
			waveInGetErrorText(m_mres,m_errtxt,512);
			throw gSound_error_str("add buffer error: ",ERR_MMSYS_ADDBUF)<<m_errtxt;
		}
	}

	MMRESULT gCSoundIn::remove_buf(gWaveHeader *hdr){
		if(!m_opened){
			throw gSound_error_str("Wave Out Device not open: ",ERR_MMSYS_DEVNOTOPEN)<<m_device_num;
		}
		//do{
		//	mres=waveInUnprepareHeader(hwi,&pwh, sizeof(pwh));
		//}while(mres==WAVERR_STILLPLAYING);
		return waveInUnprepareHeader(m_hwi,hdr->get_pwh(), sizeof(WAVEHDR));
	}

	DWORD gCSoundIn::wait(DWORD msec){
		return WaitForSingleObject(m_finish_event,msec);
	}

	void gCSoundIn::start(){
		if(!m_opened){
			throw gSound_error_str("Wave Out Device not open: ",ERR_MMSYS_DEVNOTOPEN)<<m_device_num;
		}
		ResetEvent(m_finish_event);
		m_mres=waveInStart(m_hwi);
		if(m_mres!=MMSYSERR_NOERROR){
			m_errtxt[0]=0;
			waveInGetErrorText(m_mres,m_errtxt,512);
			throw gSound_error_str("Start error: ",ERR_MMSYS_START)<<m_errtxt;
		}
	}
	void gCSoundIn::pause(){
		if(!m_opened){
			throw gSound_error_str("Wave Out Device not open: ",ERR_MMSYS_DEVNOTOPEN)<<m_device_num;
		}
		m_mres=waveInStop(m_hwi);
		//printf("byte recoreded is %i\n",pwh.dwBytesRecorded);
		if(m_mres!=MMSYSERR_NOERROR){
			m_errtxt[0]=0;
			waveInGetErrorText(m_mres,m_errtxt,512);
			throw gSound_error_str("stop error: ",ERR_MMSYS_STOP)<<m_errtxt;
		}
	}
	void gCSoundIn::close(){
		if(!m_opened){
			throw gSound_error_str("Wave Out Device not open: ",ERR_MMSYS_DEVNOTOPEN)<<m_device_num;
		}
		m_opened=FALSE;
		m_mres=waveInClose(m_hwi);
		if(m_mres!=MMSYSERR_NOERROR){
			m_errtxt[0]=0;
			waveInGetErrorText(m_mres,m_errtxt,512);
			throw gSound_error_str("close error: ",ERR_MMSYS_CLOSE)<<m_errtxt;
		}
	}

	gCSoundIn::~gCSoundIn(){
		if(m_opened){
			try{
				close();
			}catch(gSound_error_str e){
				//throw e;
				e=e;	//if we throw, we lost some memory, so.....
			}
		}
		CloseHandle(m_finish_event);
	}







	gCSoundOut::gCSoundOut(UINT dev){
		//WAVEINCAPS pwic;
		m_device_num=dev;
		m_opened=FALSE;
		m_pwoc.szPname[0]=0; 
		m_mres=waveOutGetDevCaps(0,&m_pwoc,sizeof(m_pwoc));
		m_pwoc.szPname[MAXPNAMELEN-1]=0;
		if(m_mres!=MMSYSERR_NOERROR){
			m_pwoc.szPname[0]=0;
			m_pwoc.wChannels=0;
			m_pwoc.dwFormats=0;
			switch(m_mres){
			case MMSYSERR_NODRIVER:
				throw gSound_error_str("No driver present",ERR_MMSYS_NODRIVER);
			case MMSYSERR_BADDEVICEID:
				throw gSound_error_str("Bad device id",ERR_MMSYS_BADID);
			default:
				throw gSound_error_str("Out of memory",ERR_NO_MEM);
			}
		}

		set_sound_format(1,8000,8);
		m_finish_event=CreateEvent(NULL,FALSE,FALSE,NULL);
		if(m_finish_event==NULL)throw gSound_error_str("Can't create event");
	}


	void gCSoundOut::open(int nchannel,int khz,int bits){
		set_sound_format(nchannel,khz,bits);

		m_mres=waveOutOpen( &m_hwo,m_device_num,&m_pwfx, 
				(DWORD_PTR)m_finish_event,0,CALLBACK_EVENT );//WAVE_FORMAT_DIRECT);
		
		if(m_mres!=MMSYSERR_NOERROR){
			m_errtxt[0]=0;
			waveOutGetErrorText(m_mres,m_errtxt,512);
			throw gSound_error_str("wave Out open err: ",ERR_MMSYS_DEVOPEN)<<m_errtxt;
		}
		m_opened=TRUE;
	}

	void gCSoundOut::add_buf(gWaveHeader *hdr){
		if(!m_opened){
			throw gSound_error_str("Wave Out Device not open: ",ERR_MMSYS_DEVNOTOPEN)<<m_device_num;
		}
		hdr->init_buf();
		m_mres=waveOutPrepareHeader(m_hwo, hdr->get_pwh(),sizeof(WAVEHDR));
		if(m_mres!=MMSYSERR_NOERROR){
			m_errtxt[0]=0;
			waveInGetErrorText(m_mres,m_errtxt,512);
			throw gSound_error_str("Sound out prepreare header error: ",ERR_MMSYS_PREPHDR)<<m_errtxt;
		}
	}
	
	//waveOutGetPosition()
	void gCSoundOut::play(gWaveHeader *hdr){
		if(!m_opened){
			throw gSound_error_str("Wave Out Device not open: ",ERR_MMSYS_DEVNOTOPEN)<<m_device_num;
		}
		//hdr->init_buf();
		ResetEvent(m_finish_event);
		m_mres= waveOutWrite(m_hwo,hdr->get_pwh(),sizeof(WAVEHDR));
		if(m_mres!=MMSYSERR_NOERROR){
			waveOutGetErrorText(m_mres,m_errtxt,512);
			throw gSound_error_str("play error: ",ERR_MMSYS_PLAY)<<m_errtxt;
		}
	}

	void gCSoundOut::reset(){
		if(!m_opened){
			throw gSound_error_str("Wave Out Device not open: ",ERR_MMSYS_DEVNOTOPEN)<<m_device_num;
		}
		m_mres=waveOutReset(m_hwo); 
		if(m_mres!=MMSYSERR_NOERROR){
			m_errtxt[0]=0;
			waveOutGetErrorText(m_mres,m_errtxt,512);
			throw gSound_error_str("reset error: ",ERR_MMSYS_RESET)<<m_errtxt;
		}
	}

	MMRESULT gCSoundOut::remove_buf(gWaveHeader *hdr){
		if(!m_opened){
			throw gSound_error_str("Wave Out Device not open: ",ERR_MMSYS_DEVNOTOPEN)<<m_device_num;
		}
		//do{
		//	mres=waveInUnprepareHeader(hwi,&pwh, sizeof(pwh));
		//}while(mres==WAVERR_STILLPLAYING);
		return waveOutUnprepareHeader(m_hwo,hdr->get_pwh(), sizeof(WAVEHDR));
	}

	DWORD gCSoundOut::wait(DWORD msec){
		return WaitForSingleObject(m_finish_event,msec);
	}

	void gCSoundOut::pause(){
		if(!m_opened){
			throw gSound_error_str("Wave Out Device not open: ",ERR_MMSYS_DEVNOTOPEN)<<m_device_num;
		}
		m_mres=waveOutPause(m_hwo);
		//printf("byte recoreded is %i\n",pwh.dwBytesRecorded);
		if(m_mres!=MMSYSERR_NOERROR){
			m_errtxt[0]=0;
			waveOutGetErrorText(m_mres,m_errtxt,512);
			throw gSound_error_str("pause error: ",ERR_MMSYS_PAUSE)<<m_errtxt;
		}
	}

	void gCSoundOut::close(){
		if(!m_opened){
			throw gSound_error_str("Wave Out Device not open: ",ERR_MMSYS_DEVNOTOPEN)<<m_device_num;
		}
		m_opened=FALSE;
		m_mres=waveOutClose(m_hwo);
		if(m_mres!=MMSYSERR_NOERROR){
			m_errtxt[0]=0;
			waveOutGetErrorText(m_mres,m_errtxt,512);
			throw gSound_error_str("close error: ",ERR_MMSYS_CLOSE)<<m_errtxt;
		}
	}

	void gCSoundOut::get_vol(DWORD *vol){
		if(!m_opened){
			throw gSound_error_str("Wave Out Device not open: ",ERR_MMSYS_DEVNOTOPEN)<<m_device_num;
		}
		m_mres=waveOutGetVolume(m_hwo,vol);
		if(m_mres!=MMSYSERR_NOERROR){
			m_errtxt[0]=0;
			waveOutGetErrorText(m_mres,m_errtxt,512);
			throw gSound_error_str("Get bolume error: ",ERR_MMSYS_GETVOL)<<m_errtxt;
		}
	}

	void gCSoundOut::set_vol(DWORD vol){
		if(!m_opened){
			throw gSound_error_str("Wave Out Device not open: ",ERR_MMSYS_DEVNOTOPEN)<<m_device_num;
		}
		m_mres=waveOutSetVolume(m_hwo,vol);
		if(m_mres!=MMSYSERR_NOERROR){
			m_errtxt[0]=0;
			waveOutGetErrorText(m_mres,m_errtxt,512);
			throw gSound_error_str("Set Volume error: ",ERR_MMSYS_SETVOL)<<m_errtxt;
		}
	}

	gCSoundOut::~gCSoundOut(){
		if(m_opened){
			try{
				reset();
				close();
			}catch(gSound_error_str e){
				//throw e;
				e=e;
			}
		}
		CloseHandle(m_finish_event);
	}