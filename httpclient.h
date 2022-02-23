/////////////////////////////////////////////////////////////////////
//
//	FILE:		httpclient.h
//	Author:		Gang Zhang
//	E-mail:		gzhang_ocx@yahoo.com or gazhang@geocities.com
//	HTTP:		http://members.xoom.com/gzhang
//	Version:	1.0
//	History:	Jan 1,1998, GZ Create File
//	
//	Description:	http client for pulling html of internet
//			
//			first connect to site (example: user.icx.net)
//				at port 80
//			then send: GET <filename> HTTP1.1<CRLF>
//				and other header stuff
//			and send another <CRLF> to complete
//
//			Example:  GET /gzhang/\xd\xa\xd\xa
//			will get http://members.xoom.com/gzhang/
//
//	Use: 	
//		char * http_getall(char *url);
//		int http_get_file(char *url,char **buf,int *at,int *len);
//		to get the file.  Buf is the buffer that contains the 
//		document.
//
//////////////////////////////////////////////////////////////////////
#ifndef HTTP_CLIENT_HEADER_FILE
#define HTTP_CLIENT_HEADER_FILE
#include "gtcpip.h"

#ifndef GANG_DLL_EXPORT
#define GANG_DLL_EXPORT
#endif

class GANG_DLL_EXPORT gHTTP_error_str:public gerror_str{
protected:
	virtual void init(){
		set_class("gHTTP_error_str");
	}
public:
	gHTTP_error_str(const char *str1=NULL,const GERROR_CODES err_num=ERR_OK):gerror_str(str1,err_num){
		init();
	}
	gHTTP_error_str(const gerror_str &err):gerror_str(err){
		init();
	}
};

class GANG_DLL_EXPORT gCHTTP_client:public gCTcpIp{
	enum{
		HTTP_PORT=80
	};
	enum{
			BUFF_LEN_INC=512,
			INIT_BUFF_LEN=1024
	};
public:

	void http_get_method(char *file,char *host="Top Secret");
	void http_post_method(char *file,char *host="Top Secret");	

	void http_send_method(char *file,const char *host,char *method="GET ");

	void openhttp(char *url);
	
	char *after_header(char *buf);

	//reutrns header length
	int get_header(char *buf,char *head,int len);

	char * http_getall(char *url);

	//different from http_getall, this function need to be called 
	//repeatly till returns -1
	int http_get_file(char *url,char **buf,int *at,int *len);

	//		example how to get htt
	//char buf[512];
	//htp.openhttp(*(argv+1));//"/quotes?symbols=pby&detailed=1y");
	//do{
	//	res=htp.recv(buf,512);
	//	if(res>0)buf[res]=0;
	//	fprintf(fp,"%s",buf);
	//	printf("%s",buf);
	//}while(res>0);

	//		example to get header info
	//char *last=htp.after_header(buf);
	//*(last-2)=0;
	//printf("header info:\n%s",buf);

protected:

	//url is the url http://user.icx.net/~gzhang/
	//on return, host=user.icx.net
	//		file=/~gzhang/ and port is 0
	//if url= http://user.icx.net:5000/~gzhang/, port=5000
	void phrase_url(char *url, char *host,char *file,int *port);

	//buf must be NULL when first passed in
	//at is where the current writing possition of recving
	//len is the total length of the buffer
	//suppose len is 100, and has received 10 chars, at is 10
	int http_get_file(char **buf,int *at,int *len);
};
#endif