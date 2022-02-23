////////////////////////////////////////////////////////////////////
//
//	File:		gcudp.cpp
//	Author:		Gang Zhang (gzhang@icx.net)
//	E-mail:		gzhang_ocx@yahoo.com or gazhang@geocities.com
//	HTTP:		http://members.xoom.com/gzhang
//	Description:	header file for udp class implementions
//	Revisions:	09/11/1998	Gang Zhang Create file
//
////////////////////////////////////////////////////////////////////


#ifndef GANG_DLL_EXPORT
#define GANG_DLL_EXPORT
#endif


#include "gcudp.h"




	
	void gCUdp::udp_server(int port){
		if( (sockfd=socket(AF_INET,SOCK_DGRAM,0)) <0 ){
			throw gUdp_error_str("Can't open socket ").last_err();
		}
	
		memset( (char*)&serv_addr, 0, sizeof(serv_addr) );
		serv_addr.sin_family		=AF_INET;
		serv_addr.sin_port		= htons(port);
		serv_addr.sin_addr.s_addr=htonl(INADDR_ANY);
		
		if( bind(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr) )<0){
			throw gUdp_error_str("Can't bind local address");
		}
	}

	void gCUdp::udp_client(char *serv_host_addr,int port){
		struct sockaddr_in cli_addr;
		struct hostent *host;
		if( (sockfd=socket(AF_INET,SOCK_DGRAM,0)) <0 ){
			throw gUdp_error_str("Can't open socket ").last_err();
		}
	
		memset( (char*)&serv_addr, 0, sizeof(serv_addr) );
		serv_addr.sin_family		=AF_INET;
		serv_addr.sin_port		= htons(port);

		if(serv_host_addr==NULL)
			serv_addr.sin_addr.s_addr	= htonl(INADDR_BROADCAST );
		else
			serv_addr.sin_addr.s_addr	= inet_addr(serv_host_addr);
		if(serv_addr.sin_addr.s_addr==INADDR_NONE){
			host=gethostbyname(serv_host_addr);

			if(!host){
				throw gUdp_error_str("Can't open ",ERR_SOCKET_GETHOST)+gUdp_error_str(serv_host_addr);
			}
			memcpy(&(serv_addr.sin_addr), host->h_addr, host->h_length);
			serv_addr.sin_family=host->h_addrtype;
		}

		memset( (char*)&cli_addr, 0, sizeof(cli_addr));
		cli_addr.sin_family	=AF_INET;
		cli_addr.sin_addr.s_addr=htonl(INADDR_ANY);
		cli_addr.sin_port	=htons(0);
		
		if( bind(sockfd, (struct sockaddr *)&cli_addr, sizeof(cli_addr) )<0){
			throw gUdp_error_str("Can't bind local address");
		}
	}

	void gCUdp::send(char *buf,int len){
		int slen=sendto(sockfd,buf,len,0,(struct sockaddr *)&serv_addr,
			sizeof(serv_addr));
		if(slen!=len){
			throw gUdp_error_str("Send failed ").last_err();
		}
	}

	int gCUdp::recv(char *buf,int len,struct sockaddr* cliaddr,int *fromlen){
		int slen=recvfrom(sockfd,buf,len,0,cliaddr,fromlen);
		if(slen<=0){
			throw gUdp_error_str("Recv failed ").last_err();
		}
		return slen;
	}

	
	void gCUdp::close(){
		if(sockfd>0){
			closesocket(sockfd);
			sockfd=-1;
		}
	}
	gCUdp::~gCUdp(){
		close();
	}
	int gCUdp::wait_file_rdy(){
		fd_set fdset;
		FD_ZERO(&fdset);
		FD_SET(sockfd, &fdset);
		return select(FD_SETSIZE,&fdset,NULL,NULL,NULL);
	}

	int gCUdp::file_rdy(void){
		fd_set fdset;
		struct timeval tout;
		tout.tv_sec=0;
		tout.tv_usec=0;
		FD_ZERO(&fdset);
		FD_SET(sockfd, &fdset);
		return select(FD_SETSIZE,&fdset,NULL,NULL,&tout);
	}

	int gCUdp::file_rdy(int ts,int tms){
		fd_set fdset;
		struct timeval tout;
		tout.tv_sec=ts;
		tout.tv_usec=tms;
		FD_ZERO(&fdset);
		FD_SET(sockfd, &fdset);
		return select(FD_SETSIZE,&fdset,NULL,NULL,&tout);
	}



