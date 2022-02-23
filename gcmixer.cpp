#include <windows.h>
#include <gerror.h>
#include <gcmixer.h>
/*
class gCMixerVolCtl{
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
*/
void gCMixerVolCtl::get_set_vol(VOLCTL set,int *vol,DWORD devtype,DWORD ctltype){
	if(m_res!=MMSYSERR_NOERROR)throw gerror_str("Can't open mixer ")<<m_res;
	int i;
	MIXERLINECONTROLS mxlc;
	MIXERLINE mxl;
	for(i=0;;i++){
		mxlc.cbStruct=sizeof(MIXERLINECONTROLS);
		mxlc.dwLineID=1;
		mxlc.cControls=1;
		mxlc.cbmxctrl=sizeof(m_mxctl);
		mxlc.pamxctrl=&m_mxctl;
		mxlc.dwControlID=i;
		m_res=mixerGetLineControls((HMIXEROBJ)m_hmix,&mxlc,MIXER_GETLINECONTROLSF_ONEBYID ); 
		if(m_res!=MMSYSERR_NOERROR)throw gerror_str("Can't Get Line Controls ")<<m_res;
		mxl.cbStruct=sizeof(MIXERLINE);
		mxl.dwLineID=mxlc.dwLineID;
		m_res=mixerGetLineInfo((HMIXEROBJ)m_hmix, &mxl,MIXER_GETLINEINFOF_LINEID );
		if(m_res!=MMSYSERR_NOERROR)throw gerror_str("Can't Get Line Info ")<<m_res;

		if(m_mxctl.dwControlType==ctltype){
			if(mxl.dwComponentType==devtype)
			{
				//printf("%i (%s)		(%s), %i %i \n",i,m_mxctl.szShortName,
				//	m_mxctl.szName,
				//	mxlc.dwLineID,mxl.dwComponentType);
				MIXERCONTROLDETAILS mxcd;
				MIXERCONTROLDETAILS_UNSIGNED stval;
				mxcd.cbStruct=sizeof(mxcd);
				mxcd.cbDetails=sizeof(stval);
				mxcd.dwControlID=i;
				mxcd.cChannels=1;
				mxcd.cMultipleItems =0;
				mxcd.paDetails=&stval;
				if(set==GETVOL){
					m_res=mixerGetControlDetails((HMIXEROBJ)m_hmix,&mxcd, MIXER_GETCONTROLDETAILSF_VALUE  ); 
					if(m_res!=MMSYSERR_NOERROR)throw gerror_str("Can't Get control details ")<<m_res;
					//printf("value is %i\n",stval.dwValue);
					*vol=stval.dwValue;
				}else{
					stval.dwValue=*vol;
					m_res=mixerSetControlDetails((HMIXEROBJ)m_hmix,&mxcd, MIXER_GETCONTROLDETAILSF_VALUE  ); 
					if(m_res!=MMSYSERR_NOERROR)throw gerror_str("Can't Set control details ")<<m_res;
				}
				break;
			}
		}
	}
}
