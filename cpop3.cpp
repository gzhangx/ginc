//	Program:	cpop3.cpp
//	DESC:		implement pop3 client protocal
//	Author:		Gang Zhang
//	E-mail:		gzhang_ocx@yahoo.com or gazhang@geocities.com
//	HTTP:		http://members.xoom.com/gzhang
//	Date:		Apr 16, 1998
//	Note:	all functions throws, also they return '+' if success
//			This lib is not complete



#include "cpop3.h"
#include <strstrea.h>

void gCPOP3::recvstr_getchar(){
	int len=recvbuf.recvstr(buf);
	if(len==(BUF_LEN-1)){
		buf[12]=0;
		throw gTcp_error_str("Line too long: ",ERR_POP3_BUFLIMIT)+gTcp_error_str(buf);
	}
}

	gCPOP3::gCPOP3(){
		recvbuf.init(this,BUF_LEN);
		CRLF="\xd\xa";
	};
	gCPOP3::gCPOP3(const char *ip){
		recvbuf.init(this,BUF_LEN);
		CRLF="\xd\xa";
		tcp_client(ip,PORT);
		recvstr_getchar();
	}
	
	
	char gCPOP3::open(const char *ip){
		tcp_client(ip,PORT);
		recvstr_getchar();
		return buf[0];
	}

	char gCPOP3::user(const char *username){
		//sprintf(buf,"user %s%s",username,CRLF);
		ostrstream os(buf,BUF_LEN);
		os<<"user "<<username<<CRLF<<ends;
		sendstr(buf);
		recvstr_getchar();
		return buf[0];
	}

	char gCPOP3::pass(const char *password){
		//sprintf(buf,"pass %s%s",password,CRLF);
		ostrstream os(buf,BUF_LEN);
		os<<"pass "<<password<<CRLF<<ends;
		sendstr(buf);
		recvstr_getchar();
		return buf[0];
	}

	char gCPOP3::stat(int *numofmsgs,int *totalbytes){
		//sprintf(buf,"stat%s",CRLF);
		ostrstream os(buf,BUF_LEN);
		os<<"stat"<<CRLF<<ends;
		sendstr(buf);
		recvstr_getchar();
		sscanf(buf+3,"%i %i\n",numofmsgs,totalbytes);	
		//if successfull, sscanf(buf,"%s %i %i",tmp,total,size);
		return buf[0];
	}

	char gCPOP3::list(int which){
		//sprintf(buf,"list %i%s",which,CRLF);
		ostrstream os(buf,BUF_LEN);
		os<<"list "<<which<<CRLF<<ends;
		sendstr(buf);
		recvstr_getchar();
		return buf[0];
	}

	char gCPOP3::dele(int which){
		//sprintf(buf,"dele %i%s",which,CRLF);
		ostrstream os(buf,BUF_LEN);
		os<<"dele "<<which<<CRLF<<ends;
		sendstr(buf);
		recvstr_getchar();
		return buf[0];
	}

	char gCPOP3::top(int which,int line){
		//sprintf(buf,"top %i %i%s",which,line,CRLF);
		ostrstream os(buf,BUF_LEN);
		os<<"top "<<which<<line<<CRLF<<ends;
		sendstr(buf);
		recvstr_getchar();
		return buf[0];
	}

	char gCPOP3::retr(int which){
		//sprintf(buf,"retr %i%s",which,CRLF);
		ostrstream os(buf,BUF_LEN);
		os<<"retr "<<which<<CRLF<<ends;
		sendstr(buf);
		recvstr_getchar();
		return buf[0];
	}

	int gCPOP3::retr(char *outsidebuf,int size){
		recvall(outsidebuf,size);
		outsidebuf[size]=0;
		return size;
	}

	char gCPOP3::quit(){
		//sprintf(buf,"quit%s",CRLF);
		ostrstream os(buf,BUF_LEN);
		os<<"quit"<<CRLF<<ends;
		sendstr(buf);
		recvstr_getchar();
		return buf[0];
	}
