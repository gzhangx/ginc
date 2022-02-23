//	Program:	gtcpip.cpp
//	DESC:		implement tcpip protocal functions
//	Author:		Gang Zhang
//	E-mail:		gzhang_ocx@yahoo.com or gazhang@geocities.com
//	HTTP:		http://members.xoom.com/gzhang
//	Date:		Apr 16, 1998
//	Revisions:
//			July 16, 1998: add wait_file_rdy() 
//				to wait for a ready signal
//			Aug 27,1998: tcp_client now do a nonblock 
//				connect() call 
//			Aug 28,1998: add GANG_DLL_EXPORT,
//				#define GANG_DLL_EXPORT __declspec( dllexport )
//			and the whole class will be exported in dll
//			Feb 19,1999: Seperated header and cpp file, add proxy support
//				proxy_init() for socks


#include "gtcpip.h"
#include <stdio.h>
#include <sstream>

char gCTcpIp::proxy_addr[]={0};
int gCTcpIp::proxy_port=1080;


	gCTcpIp::~gCTcpIp(){
		close();
	}

	int gCTcpIp::de_init(){
		return WSACleanup();
	}
	//windows specific
	void gCTcpIp::init(void){
		WORD wVersionRequested; 
		WSADATA wsaData; 
		wVersionRequested = MAKEWORD(1, 1); 
		if(WSAStartup(wVersionRequested, &wsaData)!=0)
			throw gTcp_error_str("Can't init windows socket");
	}

	void gCTcpIp::set_socket_timeout(int timeout){
		if(setsockopt(m_sockfd,SOL_SOCKET,SO_RCVTIMEO,(char*)&timeout,sizeof(timeout))!=0){
			throw gTcp_error_str("Can't set socket timeout opt");
		}
	}

	void gCTcpIp::set_socket_linger(BOOL on,int timeout){
		struct linger lign;
		lign.l_onoff=on;
		//0 for implement depend, default to 6 seconds, 
		lign.l_linger=timeout;
		if(setsockopt(m_sockfd,SOL_SOCKET ,SO_LINGER ,(char*)&lign,sizeof(lign))!=0){
			throw gTcp_error_str("Set linger failed");
		}
	}





	void gCTcpIp::sendall(const unsigned char *buf,const int len){
		int res;
		int left=len;
		while(left>0){
			res=send(buf,left);
			//if(res<0)return res;
			if(res<0)throw gTcp_error_str("Socket closed",ERR_SOCKET_CLOSED);
			left-=res;
			buf+=res;
		}
	}



	void gCTcpIp::recvall(unsigned char *buf,const int len){
		int res;
		int left=len;
		while(left>0){
			res=recv(buf,left);
			if(res<=0)	//return res;
				throw gTcp_error_str("Socket closed",ERR_SOCKET_CLOSED);
			left-=res;
			buf+=res;
		}
	}


	void gCTcpIp::recvall_timeout_s(char*buf,const int size,const int timeout){
		int left,writen;
		time_t when,now;
		time(&when);
		left=size;
		while(left>0){
			if(file_rdy(timeout)>0){
				writen=recv(buf,left);
				if(writen<=0){
					throw gTcp_error_str("Socket err on recv",ERR_SOCKET_CLOSED);
				}
				left-=writen;
				buf+=writen;
				if(left==0)break;
				time(&now);
				if( (now-when)>timeout ){
					throw gTcp_error_str("socket recv timeout",ERR_SOCKET_TIMEOUT);
				}
			}else{
				throw gTcp_error_str("socket recv timeout",ERR_SOCKET_TIMEOUT);
			}
		}
	}
	
	void gCTcpIp::recvall_t(char*buf,const int size,const int s,int us){
		int left,writen;
		left=size;
		while(left>0){
			if(file_rdy(s,us)>0){
				writen=recv(buf,left);
				if(writen<=0){
					throw gTcp_error_str("Socket err on recv",ERR_SOCKET_CLOSED);
				}
				left-=writen;
				buf+=writen;
			}else{
				throw gTcp_error_str("Socket recv timeout",ERR_SOCKET_TIMEOUT);
			}
		}
	}

	void gCTcpIp::sendstr(const char *buf){
		sendall(buf,(int)strlen(buf));
	}
	
	void gCTcpIp::sendstr(char *buf){
		sendstr((const char*)buf);
	}

	int gCTcpIp::recvstr(char *buf,int len){
		len=recv(buf,len);
		if(len<=0){
			buf[0]=0;
			throw gTcp_error_str("Socket closed",ERR_SOCKET_CLOSED);
		}
		buf[len]=0;
		return len;
	}

	struct hostent *gCTcpIp::name2addr(char *ip,const int iplen,const char *hostname)
	{
		struct hostent *host;
		host=gethostbyname(hostname);

		unsigned char *addr=(unsigned char*)host->h_addr_list[0];
    std::ostringstream os(ip,iplen);
    os<<addr[0]<<"."<<addr[1]<<"."<<addr[2]<<"."<<addr[3]<<std::ends;
		if(iplen>0)ip[iplen-1]=0;
		//sprintf(ip,"%i.%i.%i.%i",addr[0],addr[1],addr[2],addr[3]);
		return host;
	}

  std::string name2IpAddr(const std::string hostname) {
    struct hostent *host = gethostbyname(hostname.c_str());
    unsigned char *addr=(unsigned char*)host->h_addr_list[0];
    std::ostringstream os;
    os<<addr[0]<<"."<<addr[1]<<"."<<addr[2]<<"."<<addr[3];
    return os.str();
  }

	void gCTcpIp::tcp_server(int *port,int backlog){
		struct sockaddr_in srv_addr;

		memset((char*)&srv_addr,0, sizeof(srv_addr));
		srv_addr.sin_family = AF_INET;
		srv_addr.sin_port     = htons((short)*port);
		srv_addr.sin_addr.s_addr=htonl(INADDR_ANY);

		if ((m_sockfd = socket(AF_INET, SOCK_STREAM, 0)) == INVALID_SOCKET) 
		{
			throw gTcp_error_str("Error get socket",ERR_SOCKET_ALLOC);
			//return -1;
		}	

		if (bind(m_sockfd, (struct sockaddr*)&srv_addr, sizeof(srv_addr)) != 0) {
			throw gTcp_error_str("Error bind",ERR_SOCKET_BIND);
			//return -2;
		}

		if(*port==0){
			int len=sizeof(srv_addr);
			if(getsockname(m_sockfd,(struct sockaddr*)&srv_addr,&len)==0){
				*port=ntohs(srv_addr.sin_port);
			}else
				throw gTcp_error_str("Init socket: Error get port id",ERR_SOCKET_PORT_ALLOC);
		}

		if (listen(m_sockfd, backlog) !=0) {
			throw gTcp_error_str("Error listen",ERR_SOCKET_LISTEN);
			//return -3;
		}
	}

	void gCTcpIp::tcp_server(int port,int backlog){
		tcp_server(&port,backlog);
	}

		//nblock=0 for block, =1 for block
	//return 0 on success
	int gCTcpIp::fd_block(u_long nblock){
		return ioctlsocket (m_sockfd,FIONBIO, &nblock );
	}

	int gCTcpIp::file_write_rdy(int ts,int tus){
		fd_set fdset;
		struct timeval tout;
		tout.tv_sec=ts;
		tout.tv_usec=tus;
		FD_ZERO(&fdset);
		FD_SET(m_sockfd, &fdset);
		return select(FD_SETSIZE,NULL,&fdset,NULL,&tout);
	}
	
	int gCTcpIp::last_sock_error(){
		return WSAGetLastError();
	}
	//nblock for block or non block connection mode,not for socket
	//default nblock is 0 for block connection mode
	//connection break_tm is the time in us for how long to 
	//check if forced close is on in us	
	//all time in milliseconds
	void gCTcpIp::tcp_client_noproxy(const char *serv_ip,int port,int time_outs,int time_outus,u_long nblock,
	   long connection_break_tm){	
		struct sockaddr_in srv_addr;
		struct sockaddr_in cli_addr;
		struct hostent *host;
		int err;

		if( (m_sockfd=socket(AF_INET,SOCK_STREAM,0)) ==INVALID_SOCKET  )
		{
			throw gTcp_error_str("Can't open stream socket",ERR_SOCKET_ALLOC);
		}

		memset(&srv_addr, 0, sizeof(srv_addr) );
		srv_addr.sin_family		=AF_INET;

		if((srv_addr.sin_addr.s_addr=inet_addr(serv_ip))==INADDR_NONE){
			host=gethostbyname(serv_ip);

			if(!host){
				close();
				throw gTcp_error_str("Error get host name for ",ERR_SOCKET_GETHOST)+gTcp_error_str(serv_ip);
			}
			//memcpy(&(srv_addr->sin_addr), host->h_addr, host->h_length);
			srv_addr.sin_addr = *(struct in_addr*)(host->h_addr_list[0]);
			srv_addr.sin_family=host->h_addrtype;
		}

		srv_addr.sin_port		= htons(port);

		memset( (char*)&cli_addr, 0, sizeof(cli_addr));
		cli_addr.sin_family	=AF_INET;
		cli_addr.sin_addr.s_addr=htonl(INADDR_ANY);
		cli_addr.sin_port	=htons(0);

		err=bind(m_sockfd, (struct sockaddr *)&cli_addr, sizeof(cli_addr) );	
		if( err<0){
			//err=last_sock_error();
			close();
			throw gTcp_error_str("Can't bind local address",ERR_SOCKET_BIND);
			//return -2;
		}

		err=fd_block(nblock);
		if(err<0){
			//err=last_sock_error();
			close();
			throw gTcp_error_str("Can't set to non block mode",ERR_SOCKET_SETNOBLOCK);
		}
		err=connect(m_sockfd,(sockaddr *)&srv_addr,sizeof(srv_addr));

		if(nblock==0){
			if(err!=0){
				//err=last_sock_error();
				close();
				throw gTcp_error_str("Can't connect to ",ERR_SOCKET_CONNECT)+gTcp_error_str(serv_ip);
			}
		}else{
			if(err!=0){
				err=last_sock_error();
				if(err!= WSAEWOULDBLOCK){
					close();
					throw gTcp_error_str("Can't connect to ",ERR_SOCKET_CONNECT)+gTcp_error_str(serv_ip);
				}
			}
			nblock=0;
			err=ioctlsocket (m_sockfd,FIONBIO, &nblock );
			if(err<0){
				err=last_sock_error();
				close();
				throw gTcp_error_str("Can't set blocking mode",ERR_SOCKET_SETNOBLOCK);
			}
			do{
				//becaurefull file_write_rdy(s,us), and connection_break_tm is ms
				err=file_write_rdy(0,connection_break_tm);
				if(err!=0)break;
				if(time_outus>0)
					time_outus-=connection_break_tm;
				else{
					while(time_outus<0){
						time_outs--;
						time_outus+=1000000;
						time_outus-=connection_break_tm;
						if(time_outs<0)break;
					}
				}
				if(time_outs<0 && time_outus<0 ){
					close();
					throw gTcp_error_str("Connect to ")+gTcp_error_str(serv_ip)+gTcp_error_str(" timeout");
				}
			}while(1);
			if(err<0){
				err=last_sock_error();
				close();
				throw gTcp_error_str("Can't connect to ",ERR_SOCKET_CONNECTTIMEOUT)+gTcp_error_str(serv_ip);
			}
		}

	}

	void gCTcpIp::proxy_init(const char *proxyaddr,const int proxyport){
		proxy_port=proxyport;
		strncpy(proxy_addr,proxyaddr,64);
	}

	//note that tcp_client_noproxy is called at 3 locations
	int gCTcpIp::tcp_client(const char *ip,const int port,int time_outs,int time_outus,u_long nblock,
	   long connection_break_tm,char *bindip,int iplen,int *bindport)
	{

		if(proxy_addr[0]==0){
			tcp_client_noproxy(ip,port,time_outs,time_outus,nblock,connection_break_tm);
			return 0;
		}else{
			//there is no . in ip, probaboly localhost or local
			for(int i=0;i<256;i++){
				if(ip[i]==0){
					tcp_client_noproxy(ip,port,time_outs,time_outus,nblock,connection_break_tm);
					return 0;
				}
				if(ip[i]=='.')break;
			}
		}
		unsigned char msg[64];
		tcp_client_noproxy(proxy_addr,proxy_port,time_outs,time_outus,nblock,connection_break_tm);
		sendall("\x5\x1\x0",3);
		recvall_t((char*)msg,2,time_outs,time_outus);
		if(msg[1]!=0){
			throw gerror_str("Socks proxy failed",ERR_PROXY);//msg[2];
		}

		msg[0]=5;
		msg[1]=1;
		msg[2]=0;
		msg[3]=1;
		//ip
		struct hostent *host=gethostbyname(ip);


		if(!host){
			sscanf(ip,"%i.%i.%i.%i",&msg[4],&msg[5],&msg[6],&msg[7]);
		}else{
			if(host->h_length!=4)throw gerror_str("Not IP4 socket");
			memcpy(&(msg[4]), host->h_addr, host->h_length);
			//msg[4]=host->h_addr[3];
			//msg[5]=host->h_addr[2];
			//msg[6]=host->h_addr[1];
			//msg[7]=host->h_addr[0];
		}
		//port
		msg[8]=(port>>8)&0xff;
		msg[9]=port&0xff;
		sendall(msg,10);
		recvall_t((char*)msg,10,time_outs,time_outus);

		switch(msg[1]){
		case 0:
			//X'00' succeeded
			break;
		case 1:
            throw gerror_str("Proxy: general SOCKS server failure",ERR_PROXY_SERVER);
		case 2:
			throw gerror_str("Proxy: connection not allowed by ruleset",ERR_PROXY_RULE);
		case 3:
			throw gerror_str("Proxy: Network unreachable",ERR_PROXY_NREACH);
		case 4:
			throw gerror_str("Proxy: Host unreachable",ERR_PROXY_HOST_NREACH);
		case 5:
			throw gerror_str("Proxy: Connection refused",ERR_PROXY_REFUSE);
		case 6:
			throw gerror_str("Proxy: TTL expired",ERR_PROXY_TIMEOUT);
		case 7:
			throw gerror_str("Proxy: Command not supported",ERR_PROXY_NOSUPPORT);
		case 8:
			throw gerror_str("Proxy: Address type not supported",ERR_PROXY);
		default:
			throw gerror_str("Proxy: unassigned error ",ERR_PROXY)<<(unsigned int)msg[1];
		}

		int bindiplen=0;

		if(bindip){
      std::ostringstream os(bindip,iplen);
			switch(msg[3]){
			case 1:
				//dot addr
				//printf("debug bind returns %x.%x.%x.%x\n",msg[7],msg[6],msg[5],msg[4]);
        os<<(unsigned int)msg[7]<<"."<<(unsigned int)msg[6]<<"."<<(unsigned int)msg[5]<<"."<<(unsigned int)msg[4]<<std::ends;
				bindiplen=(int)strlen(bindip);
				break;
			case 2:
				{	//not tested yet
					bindiplen=msg[4];	//len
					int uselen=bindiplen;
					if(uselen>iplen)uselen=iplen;
					memcpy(bindip,&msg[4],uselen);
					msg[4+uselen]=0;
				}
				break;
			default:
				throw gerror_str("Proxy: IP6 not supported");
			}

		}

		if(bindport){
			switch(msg[3]){
			case 1:
				*bindport=msg[8]<<8|msg[9];
				break;
			case 2:
				bindiplen=msg[4];	//len
				*bindport=msg[4+bindiplen]<<8|msg[4+1+bindiplen];
				break;
			default:
				throw gerror_str("Proxy: IP6 not supported");
			}
		}
		return bindiplen;
	}

	//order version of socks
	int gCTcpIp::tcp_client_sock4(const char *ip,const int port,char *bindip,int iplen,int *bindport){

		if(proxy_addr[0]==0){
			tcp_client_noproxy(ip,port);
			return 0;
		}
		unsigned char msg[64];
		tcp_client_noproxy(proxy_addr,proxy_port);

		//ip
		struct hostent *host=gethostbyname(ip);


		if(!host){
			sscanf(ip,"%i.%i.%i.%i",&msg[4],&msg[5],&msg[6],&msg[7]);
		}else{
			if(host->h_length!=4)throw gerror_str("Not IP4 socket");
			memcpy(&(msg[4]), host->h_addr, host->h_length);
			//msg[4]=host->h_addr[3];
			//msg[5]=host->h_addr[2];
			//msg[6]=host->h_addr[1];
			//msg[7]=host->h_addr[0];
		}
		//port
		msg[0]=4;
		msg[1]=1;
		msg[2]=(port>>8)&0xff;
		msg[3]=port&0xff;
		msg[8]=0;
		strcpy((char*)&msg[8],"gzhang");
		sendall(msg,15);
		recvall((char*)msg,8);

		switch(msg[1]){
		case 90:
			//90: request granted	91: request rejected or failed
			break;
		case 92:
			throw gerror_str("Proxy: request rejected becasue SOCKS server cannot connect to identd on the client");
			break;
		case 93:
			throw gerror_str("Proxy: request rejected because the client program and identd report different user-ids");
			break;
		default:
			throw gerror_str("Proxy: unknow error");
		}

		int bindiplen=0;

		if(bindip){
      std::ostringstream os(bindip,iplen);
			//printf("debug bind returns %x.%x.%x.%x\n",msg[7],msg[6],msg[5],msg[4]);
			os<<(unsigned int)msg[7]<<"."<<(unsigned int)msg[6]<<"."<<(unsigned int)msg[5]<<"."<<(unsigned int)msg[4]<<std::ends;
			bindiplen=(int)strlen(bindip);
		}

		if(bindport){
			*bindport=msg[2]<<8|msg[3];
		}
	
		return bindiplen;
	}

	void gCTcpIp::close(void){
		m_socket_sem.GWait();
		if(m_sockfd!=INVALID_SOCKET){
			closesocket(m_sockfd);
			m_sockfd=INVALID_SOCKET;
		}
		m_socket_sem.GSignal();
	}

	int gCTcpIp::wait_file_rdy(){
		fd_set fdset;
		FD_ZERO(&fdset);
		FD_SET(m_sockfd, &fdset);
		return select(FD_SETSIZE,&fdset,NULL,NULL,NULL);
	}

	int gCTcpIp::file_rdy(void){
		fd_set fdset;
		struct timeval tout;
		tout.tv_sec=0;
		tout.tv_usec=0;
		FD_ZERO(&fdset);
		FD_SET(m_sockfd, &fdset);
		return select(FD_SETSIZE,&fdset,NULL,NULL,&tout);
	}

	int gCTcpIp::file_rdy(int ts,int tms){
		fd_set fdset;
		struct timeval tout;
		tout.tv_sec=ts;
		tout.tv_usec=tms;
		FD_ZERO(&fdset);
		FD_SET(m_sockfd, &fdset);
		return select(FD_SETSIZE,&fdset,NULL,NULL,&tout);
	}

	gCTcpIp gCTcpIp::accept_connection(void){
		struct sockaddr_in cli_addr;
  		int len = sizeof(cli_addr);
  		cli_addr.sin_family = AF_INET;
  		return gCTcpIp(accept(m_sockfd, (struct sockaddr*)&cli_addr, &len));
	}
	gCTcpIp gCTcpIp::accept_connection(struct sockaddr_in *cli_addr){
  		int len = sizeof(*cli_addr);
  		cli_addr->sin_family = AF_INET;
  		return gCTcpIp(accept(m_sockfd, (struct sockaddr*)cli_addr, &len));
	}

	void gCTcpIp::get_peer_name(unsigned char *cip,unsigned short *port){
		struct sockaddr_in addr;
		int len=sizeof(struct sockaddr_in);
		if(getpeername (m_sockfd,(struct sockaddr *)&addr,&len )!=0){
			throw gTcp_error_str("Can't get peer name");
		}
		*(cip  )=addr.sin_addr.S_un.S_un_b.s_b1;
		*(cip+1)=addr.sin_addr.S_un.S_un_b.s_b2;
		*(cip+2)=addr.sin_addr.S_un.S_un_b.s_b3;
		*(cip+3)=addr.sin_addr.S_un.S_un_b.s_b4;
		if(port)*port=addr.sin_port;
	}

	void gCTcpIp::get_sock_name(unsigned char *cip,unsigned short *port){
		struct sockaddr_in addr;
		int len=sizeof(struct sockaddr_in);
		if(getsockname (m_sockfd,(struct sockaddr *)&addr,&len )!=0){
			throw gTcp_error_str("Can't get sock name");
		}
		*(cip  )=addr.sin_addr.S_un.S_un_b.s_b1;
		*(cip+1)=addr.sin_addr.S_un.S_un_b.s_b2;
		*(cip+2)=addr.sin_addr.S_un.S_un_b.s_b3;
		*(cip+3)=addr.sin_addr.S_un.S_un_b.s_b4;
		if(port)*port=addr.sin_port;
	}

