#include <windows.h>
#include <gerror.h>

#ifndef GANG_DLL_EXPORT
#define GANG_DLL_EXPORT
#endif

class GANG_DLL_EXPORT gCMixerVolCtl{
protected:
	HMIXER m_hmix;
	MMRESULT m_res;
	MIXERCONTROL m_mxctl;
	enum VOLCTL{
		GETVOL,
		SETVOL
	};
	void clean(){
		if(m_hmix)mixerClose(m_hmix);
		m_hmix=NULL;
	}
public:
	gCMixerVolCtl(){
		m_hmix=NULL;
		m_res=mixerOpen( &m_hmix,0 ,0,0,0);
	}
	~gCMixerVolCtl(){
		clean();
	}
	void get_set_vol(VOLCTL set,int *vol,DWORD devtype,DWORD ctltype=MIXERCONTROL_CONTROLTYPE_VOLUME);
	int get_mic_vol(){
		int vol;
		get_set_vol(GETVOL,&vol,MIXERLINE_COMPONENTTYPE_SRC_MICROPHONE);
		return vol;
	}
	int get_spk_vol(){
		int vol;
		get_set_vol(GETVOL,&vol,MIXERLINE_COMPONENTTYPE_DST_SPEAKERS);
		return vol;
	}
	void set_mic_vol(int vol){
		get_set_vol(SETVOL,&vol,MIXERLINE_COMPONENTTYPE_SRC_MICROPHONE);
	}
	void set_spk_vol(int vol){
		get_set_vol(SETVOL,&vol,MIXERLINE_COMPONENTTYPE_DST_SPEAKERS);
	}
};

