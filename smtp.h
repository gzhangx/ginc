//	Program:	smtp.h
//	DESC:		implement SMTP protocal functions
//	Author:		Gang Zhang
//	E-mail:		gzhang_ocx@yahoo.com or gazhang@geocities.com
//	HTTP:		http://members.xoom.com/gzhang
//	Date:		Apr 16, 1998
//	Revisions:
//			Sep 16, 1998:	revisit #1
//			follow steps:
//			connect()
//			begin_session();
//			set_receiver();
//			begin_data();
//			send data
//			end_session();

#ifndef GSMTPHEADERFILE
#define GSMTPHEADERFILE

#ifndef GANG_DLL_EXPORT
#define GANG_DLL_EXPORT
#endif

#include <iostream.h>
#include <string.h>
#include <stdlib.h>
#include <gtcpip.h>

/********************************************************
class mp:public smtp{
public:
	void print_responce(const char *buf){
		cout<<buf;
		cout.flush();
	}
};

void main(void){
	mp::init();
	try{
		mp mp;
		mp.connect("icx.net");
		mp.begin_session("secrity","bbsender");
		mp.set_receiver("gzhang");
		mp.begin_data();
		mp.sendstr("Subject: test\xd\xa");
		mp.sendstr("From: Gang <bbsender@celerity.com>\xd\xa");
		mp.sendstr("hi\xd\xa");
		mp.end_session();
	}catch(gerror_str e){
		cout<<e.get_str()<<endl;
	}
}
*********************************************************/

class GANG_DLL_EXPORT gCSmtp:public gCTcpIp{
public:
	enum{
		OPEN_FILE_ERR=100,
		MEM_ERR
	};

	enum{
		MAX_BUF_SIZE=512
	};

	enum{
		SMTP_PORT=25
	};

protected:
	char *CRLF;
	char *end_of_file;
	char buf[MAX_BUF_SIZE];

	void send_end_of_file(void){
		sendstr(end_of_file);
	}

	void send_quit(void){
		sendstr("quit\xd\xa");
	}

	void sendbufstr(){
		strcat(buf,CRLF);
		sendstr(buf);
	}

public:
	gCSmtp(){
		CRLF="\xd\xa";
		end_of_file="\xd\xa.\xd\xa";
	};
	~gCSmtp(){};

	char *getbuf(){return buf;}
	//same as receive
	virtual void print_responce(const char *buf)=0;

	void connect(const char *ip){
		tcp_client(ip,SMTP_PORT);
	}

	//need check the result by check if atoi(buf)==501
	//501 is error
	void begin_session(const char *hostname,const char *sender){
		recvstr(buf,MAX_BUF_SIZE);
		print_responce(buf);
		ostrstream os(buf,MAX_BUF_SIZE);
		os<<"helo "<<hostname<<ends;
		//sprintf(buf,"helo %s",hostname);
		sendbufstr();
		recvstr(buf,MAX_BUF_SIZE);
		print_responce(buf);
		if(*buf=='5')throw gerror_str(buf);

		//sprintf(buf,"mail from: <%s>",sender);
		os.clear();
		os.seekp(0);
		os<<"mail from: <"<<sender<<">"<<ends;
		sendbufstr();
		recvstr(buf,MAX_BUF_SIZE);

		//if( atoi(buf)==501 ){
		//	throw gSMTP_error_str(buf);
		//}
		print_responce(buf);
		if(*buf=='5')throw gerror_str(buf);
	}

	void set_receiver(char *receiver){
		//sprintf(buf,"rcpt to: <%s>",receiver);
		ostrstream os(buf,MAX_BUF_SIZE);
		os<<"rcpt to: <"<<receiver<<">"<<ends;

		sendbufstr();
		recvstr(buf,MAX_BUF_SIZE);
		print_responce(buf);
		if(*buf=='5')throw gerror_str(buf);
	}

	void begin_data(void){
		sendstr("data\xd\xa");
		recvstr(buf,MAX_BUF_SIZE);
		print_responce(buf);
		if(*buf=='5')throw gerror_str(buf);
	}

	void end_session(){
		send_end_of_file();
		recvstr(buf,MAX_BUF_SIZE);
		print_responce(buf);
		send_quit();
		recvstr(buf,MAX_BUF_SIZE);
		print_responce(buf);
		if(*buf=='5')throw gerror_str(buf);
	}
};


#endif
