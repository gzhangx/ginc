//	Program:	cpop3.h
//	DESC:		implement pop3 client protocal
//	Author:		Gang Zhang
//	E-mail:		gzhang_ocx@yahoo.com, gazhang@geocities.com
//	Date:		Apr 16, 1998
//	Note:	all functions throws, also they return '+' if success
//			This lib is not complete

#ifndef CPOP3_GANG_ZHANG_HEADER_FILE
#define CPOP3_GANG_ZHANG_HEADER_FILE

#ifndef GANG_DLL_EXPORT
#define GANG_DLL_EXPORT
#endif

#include <tcpbuf_s.h>

class GANG_DLL_EXPORT gCPOP3:public gCTcpIp{
	enum{
		PORT=110,
		BUF_LEN=512
	};
	int buf_len;
	char buf[BUF_LEN];
	char *CRLF;


	tcpbuf_s recvbuf;
	//char inter_buf[BUF_LEN];
	void gCPOP3::recvstr_getchar();
public:
	gCPOP3();
	gCPOP3(const char *ip);
	
	inline char *get_buf(){return buf;}
	char gCPOP3::open(const char *ip);

	char gCPOP3::user(const char *username);

	char gCPOP3::pass(const char *password);

	char gCPOP3::stat(int *numofmsgs,int *totalbytes);

	char gCPOP3::list(int which);

	char gCPOP3::dele(int which);

	char gCPOP3::top(int which,int line);

	char gCPOP3::retr(int which);

	int gCPOP3::retr(char *outsidebuf,int size);

	char gCPOP3::quit();
};

#endif
