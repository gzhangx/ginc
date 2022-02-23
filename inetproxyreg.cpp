/////////////////////////////////////////////////////////////////////
//
//	FILE:		inetproxyreg.cpp
//	Author:		Gang Zhang
//	E-mail:		gzhang_ocx@yahoo.com or gazhang@geocities.com
//	HTTP:		http://members.xoom.com/gzhang
//	Version:	1.0
//	History:	02/19,1999, GZ Create File
//	
//	Description:	Util for get and set socks proxy registory setting
//			
//	Note:  still use sprintf and strcat, didn't check len (bad)
//////////////////////////////////////////////////////////////////////

#include "inetproxyreg.h"
#include "creg.h"
//#include <stdio.h>
#include <iostream>
using namespace std;

	void gCInetProxyReg::get_proxy(){
		m_msg[0]=0;
		GCRegKey key(&GCRegKey(),"Software\\Microsoft\\Windows\\CurrentVersion\\Internet Settings");
		key.ReadRegStr("ProxyServer",m_msg,MAX_LEN);
	}

	//get ride of the socks= line in m_msg
	void gCInetProxyReg::shrinksockline(){
		try{
			get_proxy();
		}catch(gerror_str e){
			e=e;
			m_msg[0]=0;
			return;
		}
		if(m_msg[0]==0)return;
		for(int i=0;m_msg[i];i++){
			m_msg[i]=tolower(m_msg[i]);
		}
		for(i=0;m_msg[i]!=0;i++){
			if(!strncmp(m_msg+i,"socks=",6)){
				break;
			}
		}
		if(m_msg[i]==0){
			return;
		}

		for(int k=i;m_msg[k]!=0;k++){
			if(m_msg[k]==';')break;
		}

		//no others behind socks line
		if(m_msg[k]==0){
			m_msg[i]=0;
			return;
		}

		k++;
		for(;m_msg[k];k++){
			m_msg[i++]=m_msg[k];
		}
		m_msg[i]=0;
	}



	gCInetProxyReg::gCInetProxyReg(const char *ip,const int port){
		shrinksockline();
		strcat(m_msg,"socks=");
		strcat(m_msg,ip);
		strcat(m_msg,":");
		size_t i=strlen(m_msg);
		sprintf(m_msg+i,"%i",port);
		m_port=port;
	}


	gCInetProxyReg::gCInetProxyReg(){
		m_msg[0]=0;
		get_proxy();

		if(m_msg[0]==0)return;	//error

		for(int i=0;m_msg[i];i++){
			m_msg[i]=tolower(m_msg[i]);
		}
		for(i=0;m_msg[i]!=0;i++){
			if(!strncmp(m_msg+i,"socks=",6)){
				break;
			}
		}
		if(m_msg[i]==0){
			m_msg[0]=0;
			throw gerror_str("Can't find sock entry");
		}
		i+=6;
		for(int k=0;m_msg[k];k++){
			m_msg[k]=m_msg[i+k];
		}
		m_msg[k]=0;
		//now the format is (socks: is out) socks.ibm.com:1080 may be followed by ;

		k=0;
		for(i=0;m_msg[i];i++){
			if(m_msg[i]==':'){
				//shouldn't have : in the ip address twice
				if(k!=0)throw gerror_str("error ip format");
				k=i;
			}
			if(m_msg[i]==';'){
				m_msg[i]=0;
				break;
			}
		}

		//now the format is (socks: is out) socks.ibm.com:1080
		m_port=1080;	//default
		if(k!=0){
			m_msg[k]=0;
			m_port=0;
			if(sscanf(&m_msg[k+1],"%i",&m_port)==0)m_port=1080;
		}
	}



