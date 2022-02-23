/////////////////////////////////////////////////////////////////////////////
//
//	File:		tcpbuf_s.h
//	Author:		Gang Zhang (gzhang@icx.net)
//	E-mail:		gzhang_ocx@yahoo.com or gazhang@geocities.com
//	HTTP:		http://members.xoom.com/gzhang
//	Date:		09/28/1998
//	Description:	a string buffer used to return a string
//			before a mark char, usually a '\n'.
//
//	Revisions:	09/28/1998, seperated from cpop3.h
//
/////////////////////////////////////////////////////////////////////////////

#ifndef GANG_TCPBUF_S_HEADERFILE
#define GANG_TCPBUF_S_HEADERFILE


#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <gtcpip.h>


#ifndef GANG_DLL_EXPORT
#define GANG_DLL_EXPORT
#endif

// the application provid both buffers (the internal one, which
// keep the left over message, and the one the application uses
// Make sure they are both the same length.
// Note that if return length is bszie-1, the buffer is returned
// without mark char
class GANG_DLL_EXPORT tcpbuf_s{
public:
	char *left_over_msg;
	int left_over_msg_buf_size;
	int left_over_len;
	char mark;
	gCTcpIp *tcp;
	tcpbuf_s(){
		init(NULL,0);
	}
	tcpbuf_s(gCTcpIp *ptcp,int bsize,char pmark='\xa'){
		init(ptcp,bsize,pmark);
	}
	~tcpbuf_s(){
		if(left_over_msg)delete left_over_msg;
	}
	void init(gCTcpIp *ptcp,int bsize,char pmark='\xa'){
		left_over_len=0;

		if(bsize>0){
			left_over_msg=new char [bsize];
			if(left_over_msg==NULL){
				throw gerror_str("Out of memory",ERR_NO_MEM);
			}
		}else
			left_over_msg=NULL;
		left_over_msg_buf_size=bsize;
		mark=pmark;

		tcp=ptcp;
	}

	inline char *getbuf(){return left_over_msg;}
	inline int getlen(){return left_over_len;}
	//put the reset of the data in buf[size]
	//return size of data in buf
	int flush_data(char *buf,int size){
		if(size>=left_over_len){
			memcpy(buf,left_over_msg,left_over_len);
			size=left_over_len;
			left_over_len=0;
			left_over_msg[0]=0;
			//return size;
		}else{
			memcpy(buf,left_over_msg,size);
			int leftsize=left_over_len-size;
			for(int i=0;i<leftsize;i++){
				left_over_msg[i]=left_over_msg[i+size];
			}
			left_over_msg[leftsize]=0;
			left_over_len=leftsize;
			//return size;
		}
		return size;
	}
	//resize the buf, if fail, return  original len
	//if newlen < original len, return original len
	int resize_buf(int newlen){
		if(left_over_msg_buf_size>=newlen)
			return left_over_msg_buf_size;
		char *tmp=new char[newlen];
		if(tmp==NULL)
			return left_over_msg_buf_size;
		if(left_over_msg)memcpy(tmp,left_over_msg,left_over_len);
		delete left_over_msg;
		left_over_msg=tmp;
		left_over_msg_buf_size=newlen;
		return newlen;
	}
	int recvstr(char *buf){
		int len=left_over_len;
		memcpy(buf,left_over_msg,left_over_len);
		buf[len]=0;
		left_over_len=0;
		left_over_msg[0]=0;

		if(left_over_msg==NULL)return 0;
		for(int i=0;i<len;i++){
			if(buf[i]==mark){
				strcpy(left_over_msg,buf+i+1);
				left_over_len=len-i-1;
				left_over_msg[left_over_len]=0;
				buf[i]=0;
				return len;
			}
		}
		while(1){
			int tmplen=tcp->recv_t(buf+len,
				left_over_msg_buf_size-len-1);

			buf[len+tmplen]=0;	//debugdebug assert
			int total_len=len+tmplen;
			for(;len<total_len;len++){
				if(buf[len]==mark){
					left_over_len=total_len-len-1;
					memcpy(left_over_msg,buf+len+1,left_over_len);
					left_over_msg[left_over_len]=0;
					buf[len]=0;
					return len;
				}
			}
			if(len>=(left_over_msg_buf_size-1)){
				memcpy(buf,left_over_msg,left_over_len);
				buf[left_over_len]=0;
				left_over_len=0;
				left_over_msg[0]=0;
				return len;
			}
		}
		//shouldn't reach here
		return 0;
	}
};

#endif
