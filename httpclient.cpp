/////////////////////////////////////////////////////////////////////
//
//	FILE:		httpclient.cpp
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

#include "httpclient.h"




	void gCHTTP_client::http_get_method(char *file,char *host){
		http_send_method(file,host,"GET ");
	}
	void gCHTTP_client::http_post_method(char *file,char *host){
		http_send_method(file,host,"POST ");
	}	

	//method="GET"
	void gCHTTP_client::http_send_method(char *file,const char *host,char *method){
		sendstr(method);
		sendstr(file);
		sendstr(" HTTP/1.1\xd\xa"
			"Accept: */*\xd\xa"
			"User-Agent: HTTP_SEND 0.1\xd\xa");
		sendstr("Host: ");
		if(host)sendstr(host);
		sendstr("\xd\xa");
		sendstr("\xd\xa");
	}

	void gCHTTP_client::openhttp(char *url){
		int port=HTTP_PORT;
		char host[512];
		char file[512];
		
		phrase_url(url,host,file,&port);
		close();		//make sure socket is closed
		tcp_client(host,port);
		http_get_method(file);
	}
	
	char *gCHTTP_client::after_header(char *buf){
		char *temp = strstr(buf,"\xd\xa\xd\xa");
		if(temp==NULL)temp=buf;else temp+=4;
		return temp;
	}

	//reutrns header length
	int gCHTTP_client::get_header(char *buf,char *head,int len){
		int head_len;
		char *tmp=after_header(buf);
		if(tmp!=buf){
			char a=*(tmp-1);
			*(tmp-1)=0;
			strncpy(head,buf,len);
			head_len=(int)strlen(head);
			*(tmp-1)=a;
		}
		return head_len;
	}

	char * gCHTTP_client::http_getall(char *url){
		int at;
		int len=0;
		char *buf=NULL;
		openhttp(url);
		while(http_get_file(&buf,&at,&len)>0);
		return buf;
	}

	//different from http_getall, this function need to be called 
	//repeatly till returns -1
	int gCHTTP_client::http_get_file(char *url,char **buf,int *at,int *len){
		if(get_fd()<0){
			openhttp(url);
		}
		return http_get_file(buf,at,len);
	}

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



	//url is the url http://user.icx.net/~gzhang/
	//on return, host=user.icx.net
	//		file=/~gzhang/ and port is 0
	//if url= http://user.icx.net:5000/~gzhang/, port=5000
	void gCHTTP_client::phrase_url(char *url, char *host,char *file,int *port){
		host[0]=0;
		strcpy(file,"/");	//setup default file path
		if( url[strlen(url)-1]=='\n' ) url[strlen(url)-1] = '\0';
		if( strnicmp(url,"http://",7) ){
			throw gHTTP_error_str("ERROR: Can only process http format URLs");
			//return;
		}

		url += 7;
		char *next = strpbrk(url,":/");
		if( next==NULL ){
			strcpy(host,url);
			return;
		}
		if(*next==':'){
			next[0] = 0;
			strcpy(host,url);
			next++;
			if( !isdigit(*next) )
				throw gHTTP_error_str("ERROR: Expected a port number");
			*port = atoi(next);
			while( isdigit(*next) ) next++;
		}else{
			*next=0;
			strcpy(host,url);
			*next='/';
		}
		if(*next)strcpy(file,next);
	}

	//buf must be NULL when first passed in
	//at is where the current writing possition of recving
	//len is the total length of the buffer
	//suppose len is 100, and has received 10 chars, at is 10
	int gCHTTP_client::http_get_file(char **buf,int *at,int *len){
		if(*buf==NULL){
			*at=0;
			*len=INIT_BUFF_LEN;
			*buf=new char[*len+1];
			if(*buf==NULL)throw gHTTP_error_str("Out of memory for http_get_file");
		}

		int res=recv(*buf+*at,*len-*at);
		if(res<=0){
			close();
			return res;
		}

		*at+=res;

		if( (*len-*at)<=2 ){
			char *tmp=new char[*len+BUFF_LEN_INC+1];
				if(tmp==NULL){
					delete *buf;	//out of memory
					*buf=NULL;
					throw gHTTP_error_str("Out of memory");
					//return -1;	//not reached
				}
				memcpy(tmp,*buf,*len);
				(*len)+=BUFF_LEN_INC;
				delete *buf;
				*buf=tmp;
		}
		*(*buf+*at)=0;
		return res;
	}

