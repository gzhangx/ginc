//	Program:	gtelnet.cpp
//	DESC:		implement telnet protocal functions
//	Author:		Gang Zhang
//	E-mail:		gzhang_ocx@yahoo.com or gazhang@geocities.com
//	HTTP:		http://members.xoom.com/gzhang
//	Date:		Apr 16, 1998
//	Revisions:
//			Sep 16, 1998:	revisit #1

#include "gtelnet.h"

//FILE *thelogfp=NULL;

//IAC commands
enum{
	EOR	=239,	//End Of Record
	IP	=243,	//Interrupt Process - suspend/abort process
	AO	=244,	//Abort Output - 
	AYT	=245,	//Are You There - Check to see if process still running
	EC	=246,	//Erase Character, delete last char sent
	EL	=247,	//Erase Line
	WILL=251,
	WONT=252,
	DO	=253,
	DONT=254,
	SB	=250,
	SE	=240,
	IAC	=255
};

enum{
	TELOPT_ECHO	=1,		// echo on/off
	TELOPT_EOR	=25,	// end of record
	TELOPT_NAWS	=31,	// NA-WindowSize
	TELOPT_TTYPE=24	// terminal type
};

enum{
	TELQUAL_IS		=0,
	TELQUAL_SEND	=1
};

//telnet state vars
enum{
	STATE_DATA		= 0,
	STATE_IAC		= 1,
	STATE_IACSB		= 2,
	STATE_IACWILL	= 3,
	STATE_IACDO		= 4,
	STATE_IACWONT	= 5,
	STATE_IACDONT	= 6,
	STATE_IACSBIAC	= 7,
	STATE_IACSBDATA	= 8,
	STATE_IACSBDATAIAC	= 9
};

//const unsigned char	IACWILL[2]	= { IAC, WILL };
//const unsigned char	IACWONT[2]	= { IAC, WONT };
//const unsigned char	IACDO[2]	= { IAC, DO	};
//const unsigned char	IACDONT[2]	= { IAC, DONT };
//const unsigned char	IACSB[2]	= { IAC, SB };
//const unsigned char	IACSE[2]	= { IAC, SE };

//const unsigned char gTelnet::IACWILL[2]	= { IAC, WILL };

//if fail, sockfd is set to be <0
//if ok, sockfd==0;
/*
int gTelnet::init(void){
	WORD wVersionRequested; 
	WSADATA wsaData; 
	int err; 
	wVersionRequested = MAKEWORD(1, 1); 
 
	err = WSAStartup(wVersionRequested, &wsaData); 
 
	if (err != 0) return err; 
    return 0;
}
*/

gTelnet::gTelnet(){
	IACSB[0]=IAC;
	IACSB[1]=SB;
	IACSE[0]=IAC;
	IACSE[1]=SE;
	
	//for(int i=0;i<TEL_BUFFER_SIZE;i++){
	//	receivedDX[i]=0;
	//	receivedWX[i]=0;
	//	sentDX[i]=0;
	//	sentWX[i]=0;
	//}
	//receivedDX=new unsigned char[256];
	//receivedWX=new unsigned char[256];
	//sentDX=new unsigned char[256];
	//sentWX=new unsigned char[256];

	//sockfd=0;
	//timeout=3000;	//default 3s timeout

	neg_state=0;
	current_sb=0;
	//not setup constants

	echo_ascii=0;
	return;
}

gTelnet::~gTelnet(){
	//delete receivedDX;
	//delete receivedWX;
	//delete sentDX;
	//delete sentWX;
	//close();
	//WSACleanup();
}

// return 0 is ok, return -1 can't open socket,
// return -2 can't bind, return -3 can't connect
/*
int gTelnet::open(char *serv_host_addr,int port){

	struct sockaddr_in cli_addr;
	if( (sockfd=socket(AF_INET,SOCK_STREAM,0)) <0 ){
		MessagePrint("Can't open datagram socket\n");
		return -1;
	}
	
	memset( (char*)&serv_addr, 0, sizeof(serv_addr) );
	serv_addr.sin_family		=AF_INET;

	struct hostent *host=NULL;
	if((serv_addr.sin_addr.s_addr=inet_addr(serv_host_addr))==INADDR_NONE){
		host=gethostbyname(serv_host_addr);

		if(!host){
			char msg[256];
			sprintf(msg,"Can't open %s",serv_host_addr);
			MessagePrint(msg);
			return -1;
		}
		memcpy(&(serv_addr.sin_addr), host->h_addr, host->h_length);
		serv_addr.sin_family=host->h_addrtype;
	}

	//serv_addr.sin_addr.s_addr	= inet_addr(serv_host_addr);


	serv_addr.sin_port		= htons(port);

	memset( (char*)&cli_addr, 0, sizeof(cli_addr));
	cli_addr.sin_family	=AF_INET;
	cli_addr.sin_addr.s_addr=htonl(INADDR_ANY);
	cli_addr.sin_port	=htons(0);
		
	if( bind(sockfd, (struct sockaddr *)&cli_addr, sizeof(cli_addr) )<0){
		MessagePrint("Can't bind local address\n");
		return -2;
	}

	if(connect(sockfd,(sockaddr *)&serv_addr,sizeof(serv_addr))!=0){
		MessagePrint("Can't connect to target\n");
		return -3;
	}

	//if(setsockopt(sockfd,SOL_SOCKET,SO_RCVTIMEO,(char*)&timeout,sizeof(timeout))!=0){
	//		return -4;
	//}
	return 0;
}

//a return of 1 means something in the socket to be received
int gTelnet::recv_select(int s)
{
	fd_set fdset;
	struct timeval tout;
	tout.tv_sec=s;
	tout.tv_usec=50;
	FD_ZERO(&fdset);
	FD_SET(sockfd, &fdset);

	if(s<0)return ::select(0,&fdset,NULL,NULL,NULL);
	return ::select(0,&fdset,NULL,NULL,&tout);
}
*/

int gTelnet::handle_sb(unsigned char type, unsigned char *sbdata, int sbcount){
	switch (type) {
	case TELOPT_TTYPE:
		if (sbcount>0 && sbdata[0]==TELQUAL_SEND) {
			sendIACSB();sendbyte(TELOPT_TTYPE);sendbyte(TELQUAL_IS);
			// debug bugFIXME: need more logic here if we use 
			// more than  one terminal type
			//MessagePrint("Sent terminal type as Vt100");
			sendstr("VT100");
			sendIACSE();
		}
	}
	return 0;	//return with no error
}


int gTelnet::negotiate(unsigned char* retbuf,unsigned char	*buf, int count){
	//unsigned char* nbuf= new unsigned char[count];
	//unsigned char* sbbuf= new unsigned char[count];
	unsigned char nbuf[TEL_BUFFER_SIZE];
	unsigned char sbbuf[TEL_BUFFER_SIZE];
	unsigned char sendbuf[3];
	unsigned char b,reply;
	int	sbcount = 0;
	int boffset = 0, noffset = 0;

	for(int i=0;i<count;i++){
		nbuf[i]=sbbuf[i]=0;
	}

	while(boffset < count) {
		b=buf[boffset++];
		switch (neg_state) {
		case STATE_DATA:
			if (b==IAC) {
				//MessagePrint("debug_string: state data ot iac");
				neg_state = STATE_IAC;
			} else {
				nbuf[noffset++]=b;
			}
			break;
		case STATE_IAC:
			//MessagePrint("debug_string: state ica");
			switch (b) {
			case IAC:
				neg_state = STATE_DATA;
				//nbuf[noffset++]=IAC;
				sbbuf[sbcount++]=IAC;
				break;
			case WILL:
				neg_state = STATE_IACWILL;
				break;
			case WONT:
				neg_state = STATE_IACWONT;
				break;
			case DONT:
				neg_state = STATE_IACDONT;
				break;
			case DO:
				neg_state = STATE_IACDO;
				break;
			case EOR:
				neg_state = STATE_DATA;
				break;
			case SB:
				neg_state = STATE_IACSB;
				sbcount = 0;
				break;
			default:
				//MessagePrint("debug: unknown ICA");
				neg_state = STATE_DATA;
				break;
			}
			break;
		case STATE_IACWILL:
			//MessagePrint("debug_string: stat ica will");
			switch(b) {
			case TELOPT_ECHO:
				reply = DO;
				//MessagePrint("debug_string: Begin Echo");
				//do something about echo
				echo_ascii=1;
				break;
			case TELOPT_EOR:
				//MessagePrint("debug_string: End Rec");
				reply = DO;
				break;
			default:
				reply = DONT;
				break;
			}

			//if (	reply != sentDX[b+128] ||
			//	WILL != receivedWX[b+128]
			//) {
				sendbuf[0]=IAC;
				sendbuf[1]=reply;
				sendbuf[2]=b;
				sendall(sendbuf,3);
				//sentDX[b+128] = reply;
				//receivedWX[b+128] = WILL;
			//}else{
			//	printf("debug exceptrion!\n");
			//}
			neg_state = STATE_DATA;
			break;
		case STATE_IACWONT:
			//MessagePrint("debug_string: Stat ica won't");
			switch(b) {
			case TELOPT_ECHO:
				//vec.addElement("LOCALECHO");
				//MessagePrint("debug_string: Stop Echo");
				reply = DONT;
				echo_ascii=0;
				break;
			case TELOPT_EOR:
				reply = DONT;
				//MessagePrint("debug_string: Stop Rec");
				break;
			default:
				reply = DONT;
				break;
			}
			//if (	reply != sentDX[b+128] ||
			//	WONT != receivedWX[b+128]
			//) {
				sendbuf[0]=IAC;
				sendbuf[1]=reply;
				sendbuf[2]=b;
				sendall(sendbuf,3);
				//sentDX[b+128] = reply;
				//receivedWX[b+128] = WILL;
			//}else{
			//	printf("debug exceptrion!\n");
			//}
			neg_state = STATE_DATA;
			break;
		case STATE_IACDO:
			//MessagePrint("debug_string: state ica do");
			switch (b) {
			case TELOPT_ECHO:
				//MessagePrint("debug_string: Will Echo");
				reply = WILL;
				break;
			case TELOPT_TTYPE:
				reply = WILL;
				break;
			default:
				//case TELOPT_NAWS:
				reply = WONT;
				break;
			}
			//if (	reply != sentWX[128+b] ||
			//	DO != receivedDX[128+b]
			//) {
				sendbuf[0]=IAC;
				sendbuf[1]=reply;
				sendbuf[2]=b;
				sendall(sendbuf,3);
				//sentWX[b+128] = reply;
				//receivedDX[b+128] = DO;
			//}else{
			//	printf("debug exceptrion!\n");
			//}
			neg_state = STATE_DATA;
			break;
		case STATE_IACDONT:
			//MessagePrint("debug_string: state ica don't");
			switch (b) {
			case TELOPT_ECHO:
				//MessagePrint("debug_string: Don't Echo");
				reply	= WONT;
				//"NOLOCALECHO"
				break;
			default:
				//case TELOPT_NAWS:
				reply	= WONT;
				break;
			}
			//if (	reply	!= sentWX[b+128] ||
			//	DONT	!= receivedDX[b+128]
			//) {
				sendbyte(IAC);sendbyte(reply);sendbyte(b);
				//sentWX[b+128]		= reply;
				//receivedDX[b+128]	= DONT;
			//}else{
			//	printf("debug exceptrion!\n");
			//}
			neg_state = STATE_DATA;
			break;
		case STATE_IACSBIAC:
			if (b == IAC) {
				sbcount		= 0;
				current_sb	= b;
				neg_state	= STATE_IACSBDATA;
			} else {
				//bad
				neg_state	= STATE_DATA;
			}
			break;
		case STATE_IACSB:
			switch (b) {
			case IAC:
				neg_state = STATE_IACSBIAC;
				break;
			default:
				current_sb	= b;
				sbcount		= 0;
				neg_state	= STATE_IACSBDATA;
				break;
			}
			break;
		case STATE_IACSBDATA:
			switch (b) {
			case IAC:
				neg_state = STATE_IACSBDATAIAC;
				break;
			default:
				sbbuf[sbcount++] = b;
				break;
			}
			break;
		case STATE_IACSBDATAIAC:
			switch (b) {
			case IAC:
				neg_state = STATE_IACSBDATA;
				sbbuf[sbcount++] = IAC;
				break;
			case SE:
				handle_sb(current_sb,sbbuf,sbcount);
				current_sb	= 0;
				neg_state	= STATE_DATA;
				break;
			case SB:
				handle_sb(current_sb,sbbuf,sbcount);
				neg_state	= STATE_IACSB;
				break;
			default:
				neg_state	= STATE_DATA;
				break;
			}
			break;
		default:
			//MessagePrint("wrong on neg_type");
			neg_state = STATE_DATA;
			break;
		}
	}
		
	memcpy(retbuf,nbuf,noffset);
	//delete nbuf;
	//delete sbbuf;
	return noffset;
}
