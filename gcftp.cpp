/////////////////////////////////////////////////////////////////
//
//	FILE		gcftp.cpp
//	Author		Gang Zhang
//	E-mail:		gzhang_ocx@yahoo.com or gazhang@geocities.com
//	HTTP:		http://members.xoom.com/gzhang
//	Description	Ftp client function implement
//	History		1.0
//			Aug 10, 1997  GZ create file
//
//				1.1	Add proxy support.
//
//	set the following define to define for export
//	#define GANG_DLL_EXPORT __declspec( dllexport )
//
/////////////////////////////////////////////////////////////////

#include <gcftp.h>
#include <strstrea.h>


	void gCFtp::GFtp_init(){
		passive=FALSE;
		//left_over_msg[0]=0;
		recvbuf.init(this,GFTP_MAXBUF);
	}
	gCFtp::gCFtp(){
		GFtp_init();
	}
	gCFtp::gCFtp(char *ip,int port){
		GFtp_init();
		open(ip,port);
	}
	gCFtp::~gCFtp(){}
	void gCFtp::open(char *ip,int port){
		close();
		tcp_client(ip,port);
	}


	//only called after login, once
	int gCFtp::flash_dsplymsg(BOOL dsply){
		int len;
		msg[0]=0;
		do{
			len=(int)strlen(msg);
			if(len>=4){
				if( isdigit(msg[0])
					&& isdigit(msg[1])
					&& isdigit(msg[2])
					&& (msg[3]!='-')
					){
					if(dsply)dsply_msg(msg);
					return len;
				}
			}
			if(len)dsply_msg(msg,SHOW_DSP_CODE);
			len=gCFtp::recvstr();
		}while(len>0);
		throw gFtp_error_str("Socket closed");
		return len;
	}

	int gCFtp::recvstr(){
		int len;
		try{
			len=recvbuf.recvstr(msg);
		}catch(gerror_str e){
			throw gFtp_error_str(e);
		}
		return len;
	
		/*
		int len=left_over_len;
		strncpy(msg,left_over_msg,left_over_len);
		msg[len]=0;
		left_over_len=0;
		left_over_msg[0]=0;
		for(int i=0;i<len;i++){
			if(msg[i]=='\n'){
				strcpy(left_over_msg,msg+i+1);
				left_over_len=len-i-1;
				//ASSERT
				//if(left_over_len!=strlen(left_over_msg)){
				//	dsply_msg("debugdebug\n");
				//}

				msg[i]=0;
				return len;
			}
		}

		while(1){
			if(len>=(GFTP_MAXBUF-1)){
				throw gFtp_error_str("len too long");
			}
			int tmplen=recv(msg+len, GFTP_MAXBUF-len);
			if(tmplen<=0){
				throw gFtp_error_str("socket closed");
			}

			msg[len+tmplen]=0;	//debugdebug assert
			int total_len=len+tmplen;
			for(;len<total_len;len++){
				if(msg[len]=='\n'){
					left_over_len=total_len-len-1;
					memcpy(left_over_msg,msg+len+1,left_over_len);
					left_over_msg[left_over_len]=0;
					msg[len]=0;
					return len;
				}
			}
		}
		msg[len]=0;
		return len;
		*/
	}


	void gCFtp::user(char *user_name){
		//sprintf(msg,"USER %s\xd\xa",user_name);
		ostrstream os(msg,GFTP_MAXBUF);
		os<<"USER "<<user_name<<"\xd\xa"<<ends;
		sendstr(msg);
		recvstr();
	}
	void gCFtp::pass(char *pass_word){
		//sprintf(msg,"PASS %s\xd\xa",pass_word);
		ostrstream os(msg,GFTP_MAXBUF);
		os<<"PASS "<<pass_word<<"\xd\xa"<<ends;
		sendstr(msg);
		flash_dsplymsg();
		//recvstr();
	}


	//TRUE if ok

	void gCFtp::do_pasv(){
		sendstr("PASV\xd\xa");
		int len=recvstr();
		char arg[128];
		sscanf(msg,"%s",arg);
		if(strcmp(arg,"227")){
			throw gFtp_error_str(msg);
		}
		
		char ip[32];
		for(int i=0;i<len;i++){
			if(msg[i]=='('){
				break;
			}
		}
		i++;
		if(i>=len){
			throw gFtp_error_str("ftp pasv responce error");
		}
		//count for 4 ,s
		int count=0;
		int ipi=0;
		for(;i<len;i++,ipi++){
			ip[ipi]=msg[i];
			if(ip[ipi]==','){
				ip[ipi]='.';
				count++;
				if(count==4){
					ip[ipi]=0;
					break;
				}
			}
		}

		if(count!=4){
			throw gFtp_error_str("ftp pasv ip string error");
		}

		unsigned long upp,dpp;
		sscanf(&msg[i+1],"%i,%i",&upp,&dpp);
		unsigned short port=(unsigned short)(upp<<8|dpp)&0xffff;
		try{
			data_fd.tcp_client(ip,port);
			data_fd.set_socket_linger(TRUE);
		}catch(gerror_str e){
			throw gFtp_error_str(e);
		}
	}

	//return true if port ok
	//throws if socket closed
	void gCFtp::do_port(){
		if(passive){
			do_pasv();
			return;
		}
		int data_port=0;

		try{
			data_fd.close();
			data_fd.tcp_server(&data_port);
			data_fd.set_socket_linger(TRUE);
		}catch(gerror_str e){
			throw gFtp_error_str(e);
		}

		unsigned char myip[4];
		get_sock_name(myip);
		//sprintf(msg,"PORT %i,%i,%i,%i,%i,%i\xd\xa",
		//	myip[0],myip[1],myip[2],myip[3],
		//	((data_port>>8)&0xff),
		//	(data_port&0xff)
		//	);
		ostrstream os(msg,GFTP_MAXBUF);
		os<<"PORT "<<(unsigned int)myip[0]
				<<","<<(unsigned int)myip[1]
				<<","<<(unsigned int)myip[2]
				<<","<<(unsigned int)myip[3]
				<<","<<(unsigned int)((data_port>>8)&0xff)
				<<","<<(unsigned int)(data_port&0xff)
				<<"\xd\xa"<<ends;
		
		sendstr(msg);

		flash_dsplymsg(FALSE);
		if(msg[0]=='5'){
			data_fd.close();
			throw gFtp_error_str(msg);
		}
		dsply_msg(getmsg());
	}

	void gCFtp::get_port(){
		if(passive==TRUE)return;
		SOCKET cskt=data_fd.accept_connection();
		data_fd.close();
		data_fd.set_fd(cskt);
	}

	void gCFtp::set_mode_pasv(){
		passive=TRUE;
	}

	void gCFtp::set_mode_port(){
		passive=FALSE;
	}

	//returns a data socket to accept ls
	//or -1 if error on data socket
	void gCFtp::do_ls(char *arg,int notnlst){
		do_port();

		if(arg==NULL)arg="*";
		ostrstream os(msg,GFTP_MAXBUF);
		if(notnlst)
			//sprintf(msg,"LIST %s\xd\xa",arg);
			os<<"LIST "<<arg<<"\xd\xa"<<ends;
		else
			//sprintf(msg,"NLST %s\xd\xa",arg);
			os<<"NLST "<<arg<<"\xd\xa"<<ends;
		sendstr(msg);

		flash_dsplymsg(FALSE);
		if(msg[0]=='5'){
			throw gFtp_error_str(msg);
		}
		
		dsply_msg(getmsg());
		get_port();

		int len;
		do{
			len=data_fd.recv(msg,GFTP_MAXBUF);
			if(len<=0)break;
			msg[len]=0;
			dsply_part_msg(msg,LIST_DSP_CODE);
		}while(len>0);
		flash_dsplymsg();
	}

	void gCFtp::do_get(char *arg){
		do_port();
		//sprintf(msg,"RETR %s\xd\xa",arg);
		ostrstream os(msg,GFTP_MAXBUF);
		os<<"RETR "<<arg<<"\xd\xa"<<ends;
		sendstr(msg);
		flash_dsplymsg(FALSE);
		if(msg[0]=='5'){
			throw gFtp_error_str(getmsg());
		}
		get_port();
		dsply_msg(getmsg());
	}

	void gCFtp::get_data(char *name, BOOL ascii){
		//FILE *fp;
		//if(ascii)
		//	fp=fopen(name,"w");
		//else
		//	fp=fopen(name,"wb");
		//int len;
		fstream fs;
		if(ascii)
			fs.open(name,fstream::out,filebuf::sh_read);
		else
			fs.open(name,fstream::out|fstream::binary,filebuf::sh_read);
		int len;
		if(fs.good()){
			while((len=read_data_channel(msg,GFTP_MAXBUF))>0){
				fs.write(msg,len);
				dsply_part_msg("#",PROG_DSP_CODE);
			}
			fs.close();
		}else{
			//sprintf(msg,"ftp get: can't opne file %s",name);
			ostrstream os(msg,GFTP_MAXBUF);
			os<<"ftp get: can't open file "<<name<<ends;
			dsply_msg(msg,FILE_ERR_CODE);
		}
		close_data_channel();
		dsply_msg("",PROG_DSP_CODE);
		flash_dsplymsg();
	}

	void gCFtp::do_put(char *arg){
		do_port();
		//sprintf(msg,"STOR %s\xd\xa",arg);
		ostrstream os(msg,GFTP_MAXBUF);
		os<<"STOR "<<arg<<"\xd\xa"<<ends;
		sendstr(msg);
		flash_dsplymsg(FALSE);
		if(msg[0]=='5'){
			throw gFtp_error_str(getmsg());
		}
		get_port();
		dsply_msg(getmsg());
	}

	void gCFtp::put_data(char *name, BOOL ascii){
		//FILE *fp;
		fstream fs;
		if(ascii)
			fs.open(name,fstream::in,filebuf::sh_read);
			//fp=fopen(name,"r");
		else
			fs.open(name,fstream::in|fstream::binary,filebuf::sh_read);
			//fp=fopen(name,"rb");
		//int len;
		if(fs.good()){
			try{
				while(!fs.eof()){
					//(len=fread(msg,1,GFTP_MAXBUF,fp)) >0){
					fs.read(msg,GFTP_MAXBUF);
					write_data_channel(msg,fs.gcount());
					dsply_part_msg("#",PROG_DSP_CODE);
				}
			}catch(gerror_str e){
				//fclose(fp);
				fs.close();
				//sprintf(msg,"ftp put: data closed while send %s",name);
				ostrstream os(msg,GFTP_MAXBUF);
				os<<"ftp put: data closed while send "<<name<<ends;
				dsply_msg(msg,FILE_ERR_CODE);
				close_data_channel();
				flash_dsplymsg();
				throw gFtp_error_str(e);
			}
			fs.close();
			//fclose(fp);
		}else{
			//sprintf(msg,"ftp put: can't opne file %s",name);
			ostrstream os(msg,GFTP_MAXBUF);
			os<<"ftp put: can't open file "<<name<<ends;
			dsply_msg(msg,FILE_ERR_CODE);
		}
		close_data_channel();
		dsply_msg("",PROG_DSP_CODE);
		flash_dsplymsg();
	}

	void gCFtp::do_cd(char *arg){
		//sprintf(msg,"CWD %s\xd\xa",arg);
		ostrstream os(msg,GFTP_MAXBUF);
		os<<"CWD "<<arg<<"\xd\xa"<<ends;
		sendstr(msg);
		flash_dsplymsg();
	}

	int gCFtp::read_data_channel(char *buf,int size){
		return data_fd.recv(buf,size);
	}
	
	void gCFtp::write_data_channel(char *buf,int size){
		data_fd.sendall(buf,size);
	}
	
	void gCFtp::close_data_channel(){
		data_fd.close();
	}



//example useage class

	int gCFtpCon::prase_cmd(char *msg){
		//int portdone=0;
		char cmd[GFTP_MAXBUF+3],arg[GFTP_MAXBUF+3];
		cmd[0]=0;
		arg[0]=0;
		sscanf(msg,"%s %s",cmd,arg);

		if(!strcmp(cmd,"ls")){
			do_ls(arg[0]?arg:NULL);
		}else if(!strcmp(cmd,"dir")){
			do_ls(arg[0]?arg:NULL,TRUE);
		}else if(!strcmp(cmd,"get")){
			do_get(arg);
			get_data(arg,ascii);
		}else if(!strcmp(cmd,"put")){
			do_put(arg);
			put_data(arg,ascii);
		}else if(!strcmp(cmd,"cd")){
			do_cd(arg);
		}else if(!strcmp(cmd,"quit")){
			return FALSE;
		}else if(!strcmp(cmd,"port")){
			set_mode_port();
			cout<<"Transfer mode Active"<<endl;
		}else if(!strcmp(cmd,"pasv")){
			set_mode_pasv();
			cout<<"Transfer mode Pasive"<<endl;
		}else if(!strcmp(cmd,"bin")){
			sendstr("TYPE I\xd\xa");
			flash_dsplymsg();
		}else if(!strcmp(cmd,"ascii")){
			sendstr("TYPE A\xd\xa");
			flash_dsplymsg();
		}else{
			strcat(msg,"\xd\xa");
			sendstr(msg);
			flash_dsplymsg();
		}
		return TRUE;
	}


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

