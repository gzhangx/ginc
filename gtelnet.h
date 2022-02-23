//	Program:	gtelnet.h
//	DESC:		implement telnet protocal functions
//	Author:		Gang Zhang
//	E-mail:		gzhang_ocx@yahoo.com or gazhang@geocities.com
//	HTTP:		http://members.xoom.com/gzhang
//	Date:		Apr 16, 1998
//	Revisions:
//			Sep 16, 1998:	revisit #1

#ifndef GTELNET_HEADER_FILE 
#define GTELNET_HEADER_FILE 

#ifndef GANG_DLL_EXPORT
#define GANG_DLL_EXPORT
#endif

#define TEL_BUFFER_SIZE 256

#include "gtcpip.h"

class GANG_DLL_EXPORT gTelnet:public gCTcpIp{
	unsigned char	IACSB[2];
	unsigned char	IACSE[2];
	public:


	enum{
		TELNET_PORT=23
	};
	unsigned char neg_state;
	unsigned char current_sb;

	int echo_ascii;

	gTelnet();
	~gTelnet();
	void open(const char *serv_host_addr,int port=TELNET_PORT){
		//23 is the default telnet port
		tcp_client(serv_host_addr,port);
	}
	
	protected:
	void sendIACSB(void){
		sendall(IACSB,2);
	}

	void sendIACSE(void){
		sendall(IACSE,2);
	}

	/*int recv_select(int s=0);*/
	int handle_sb(unsigned char type, unsigned char *sbdata, int sbcount);
	public:
	int negotiate(unsigned char* retbuf,unsigned char	*buf, int count);
	//main function.  make sure strbuf is of size TEL_BUFFER_SIZE
	int getstr(char *strbuf){
		unsigned char rbuf[TEL_BUFFER_SIZE];
		int getsize=recv(rbuf,TEL_BUFFER_SIZE);
		if(getsize<=0){
			strbuf[0]=0;
			//return getsize;
			throw gerror_str("Socket closed",ERR_SOCKET_CLOSED);
		}

		int size=negotiate((unsigned char *)strbuf,rbuf,getsize);
		strbuf[size]=0;
		return size;
	}
};

#endif
