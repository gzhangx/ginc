/////////////////////////////////////////////////////////////////////
//
//	FILE:		inetproxyreg.h
//	Author:		Gang Zhang
//	E-mail:		gzhang_ocx@yahoo.com or gazhang@geocities.com
//	HTTP:		http://members.xoom.com/gzhang
//	Version:	1.0
//	History:	02/19,1999, GZ Create File
//	
//	Description:	Util for get and set socks proxy registory setting
//			
//
//////////////////////////////////////////////////////////////////////
#ifndef GCINETPROXYREG_HEADER_FILE
#define GCINETPROXYREG_HEADER_FILE

#ifndef GANG_DLL_EXPORT
#define GANG_DLL_EXPORT
#endif
//class for phrase IE proxy setting
//if return proxy =="", no proxy found in registry
//call with no parameter to get proxy, call with ip and port to set proxy
class GANG_DLL_EXPORT gCInetProxyReg{
	enum{
		MAX_LEN=512
	};
	char m_msg[MAX_LEN];
	int m_port;
	void get_proxy();

	//get ride of the socks= line in m_msg
	void shrinksockline();

public:
	char *get_ip() const {return (char *)m_msg;}
	int get_port() const {return m_port;}


	gCInetProxyReg(const char *ip,const int port=1080);

	gCInetProxyReg();
};



#endif
/// end of header