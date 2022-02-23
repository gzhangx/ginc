/////////////////////////////////////////////////////////////////
//
//	FILE		gftp.h
//	Author		Gang Zhang (gazhang@geocities.com)
//	Homepage:	http://members.xoom.com/gzhang
//	Description	Ftp client function implement
//	History		1.0
//			Aug 10, 1997  GZ create file
//
//
//	set the following define to define for export
//	#define GANG_DLL_EXPORT __declspec( dllexport )
//
/////////////////////////////////////////////////////////////////
#ifndef GFtp_gtcpip_header_file
#define GFtp_gtcpip_header_file

#include <tcpbuf_s.h>
#include <stdio.h>

#include <fstream.h>
class GANG_DLL_EXPORT gFtp_error_str:public gerror_str{
protected:
	virtual void init(){
		set_class("gFtp_error_str");
	}
public:
	gFtp_error_str(const char *str1=NULL,const GERROR_CODES err_num=ERR_OK):gerror_str(str1,err_num){
		init();
	}
	gFtp_error_str(const gerror_str &err):gerror_str(err){
		init();
	}
};

class GANG_DLL_EXPORT gCFtp:public gCTcpIp{
protected:
	enum{
		FTP_PORT=21,
		GFTP_MAXBUF=2048
	};
	
	tcpbuf_s recvbuf;
	//char left_over_msg[GFTP_MAXBUF+3];

	char msg[GFTP_MAXBUF+3];
	gCTcpIp data_fd;
	BOOL passive;
public:
	enum FtpDspCode{
		OP_RET_CODE,	//return for operation (port, ls)
		PROG_DSP_CODE,	//progress for file download
		LIST_DSP_CODE,	//date of list
		FILE_ERR_CODE,	//for put or get operation
		SHOW_DSP_CODE	//for dsplay msg from srver xxx - hello!
	};
	void GFtp_init();
	gCFtp();
	gCFtp(char *ip,int port=FTP_PORT);
	~gCFtp();
	void open(char *ip,int port=FTP_PORT);


	virtual void dsply_msg(LPCSTR msg,FtpDspCode code=OP_RET_CODE)=0;
	//{printf("%s\n",msg);}

	virtual void dsply_part_msg(LPCSTR msg,FtpDspCode)=0;
	//{printf("%s",msg);}

	//only called after login, once
	int flash_dsplymsg(BOOL dsply=TRUE);

	virtual int recvstr();

	char *getmsg() const {return (char*)msg;}
	void user(char *user_name);
	void pass(char *pass_word);


	//TRUE if ok
protected:
	void do_pasv();
public:
	//return true if port ok
	//throws if socket closed
	void do_port();

	void get_port();

	void set_mode_pasv();

	void set_mode_port();

	//returns a data socket to accept ls
	//or -1 if error on data socket
	void do_ls(char *arg,int notnlst=FALSE);

	void do_get(char *arg);

	virtual void get_data(char *name, BOOL ascii);

	void do_put(char *arg);

	virtual void put_data(char *name, BOOL ascii);
	
	void do_cd(char *arg);

	int read_data_channel(char *buf,int size);
	
	void write_data_channel(char *buf,int size);
	
	void close_data_channel();

};

//example useage class
class GANG_DLL_EXPORT gCFtpCon:public gCFtp{
	BOOL verbose;
	BOOL ascii;
public:
	gCFtpCon(BOOL verb=FALSE,BOOL asciip=FALSE,BOOL passivep=FALSE){
		passive=passivep;
		ascii=asciip;
		verbose=verb;
	}

	int prase_cmd(char *msg);
};

/*
class GFtpDos: public GFtpCon{
public:
	GFtpDos(BOOL verb=TRUE,BOOL ascii=FALSE,BOOL passive=FALSE):GFtpCon(verb,ascii,passive){
	}	
	virtual void dsply_msg(LPCSTR msg,FtpDspCode code=OP_RET_CODE)
	{
		printf("%s\n",msg);
	}

	virtual void dsply_part_msg(LPCSTR msg,FtpDspCode code=OP_RET_CODE)
	{
		printf("%s",msg);
	}
};
*/
//
//
//	GFtpDos ftp;
//	char msg[512];
//	ftp.open(argv[1]);
//	ftp.flash_dsplymsg();
//	ftp.dsply_msg(ftp.getmsg());
//	do{
//		msg[0]=0;
//		gets(msg);
//		if(ftp.prase_cmd(msg)==FALSE)break;
//	}while(1);
//
//
#endif
