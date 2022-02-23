////////////////////////////////////////////////////////////////////
//
//	File:		gudp.h
//	Author:		Gang Zhang (gzhang@icx.net)
//	E-mail:		gzhang_ocx@yahoo.com or gazhang@geocities.com
//	HTTP:		http://members.xoom.com/gzhang
//	Description:	header file for udp class implementions
//	Revisions:	09/11/1998	Gang Zhang Create file
//
////////////////////////////////////////////////////////////////////

#ifndef GUDP_HEADER_FILE
#define GUDP_HEADER_FILE


#ifndef GANG_DLL_EXPORT
#define GANG_DLL_EXPORT
#endif


#include "gerror.h"
//#include <stdlib.h>
//#include <string.h>
#include <winsock.h>

class GANG_DLL_EXPORT gUdp_error_str:public gerror_str{
protected:
	virtual void init(){
		set_class("gUdp_error_str");
	}
public:
	gUdp_error_str(const char *str1=NULL,const GERROR_CODES err_num=ERR_OK):gerror_str(str1,err_num)
	{
		init();
	}
	gUdp_error_str(const gerror_str &err):gerror_str(err){
		init();
	}
};

class GANG_DLL_EXPORT gCUdp{
	SOCKET sockfd;
	struct sockaddr_in serv_addr;
public:
	SOCKET get_fd(){return sockfd;}
	void set_fd(int fd){
		close();
		sockfd=fd;
	}
	void udp_server(int port);

	void udp_client(char *serv_host_addr,int port);

	void send(char *buf,int len);

	int recv(char *buf,int len,struct sockaddr* cliaddr=NULL,int *fromlen=NULL);

	gCUdp(){
		sockfd=-1;
	}
	void close();
	~gCUdp();
	int wait_file_rdy();

	int file_rdy(void);

	int file_rdy(int ts,int tms=0);
};


#endif