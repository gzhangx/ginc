//
//	Filename:	scriptel.cpp
//	Author:		Gang Zhang
//	E-mail:		gzhang_ocx@yahoo.com or gazhang@geocities.com
//	HTTP:		http://members.xoom.com/gzhang
//	DESC:		Scripted telnet program
//	Date:		Apr 16, 1998
//	Revisions:
//			GZ Nov 15, 1998:	revisit #1
//				Modified to be suitable to fit in GUI applications
//				You can use open ip port to open anyport
//				Now a empty waitfor will wait for enter
//				empty waitstr will wait for CRLF
//				
//#include <conio.h>
#include <scriptel.h>


/**************************************************************
#include <conio.h>
class scriptel_dos: public script_tel{
public:
	void MessagePrint(const char *buf)
	{
		printf("%s",buf);
		if(thelogfp)fprintf(thelogfp,"%s",buf);
	}
	void ErrorMessagePrint(const char *buf)
	{
		printf("%s",buf);
		if(thelogfp)fprintf(thelogfp,"%s",buf);
	}

	void PromptMessagePrint(const char *buf)
	{
		printf("%s",buf);
		if(thelogfp)fprintf(thelogfp,"%s",buf);
	}
	int Get_A_char(){
		return _getch();
	}
	void Input_gets(char *buf){
		gets(buf);
	}
	bool need_exit(){
		return false;
	}
};

**************************************************************/



	BOOL manu_input_thread::InitInstance(){
		char msg[512];
		int len;
		while(is_alive()){
			if(tcp->file_rdy(0,300000)){
				len=tcp->recv(msg,511);
				if(len<=0){
					go_exit();
					break;
				}else{
					msg[len]=0;
					m_pdsply->MessagePrint(msg);
				}
			}
		}
		return 0;
	}


	void stack_queue::push(int line,int pos){
		stack_struct *stk=new stack_struct;
		stk->line=line;
		stk->pos=pos;
		queue_tail(stk);
	}
	//return true if there are anything in stack
	int stack_queue::pop(int *line,int *pos){
		stack_struct *stk=unqueue_tail();
		if(stk){
			*line=stk->line;
			*pos=stk->pos;
			delete stk;
			return 1;
		}
		return 0;
	}

	void stack_queue::clear_stack(void){
		int line,pos;
		while(pop(&line,&pos));
	}



	
	void label_queue::add_label(const char *name,int line,int pos){
		if(find_label(name)>=0){
			return;
		}
		label_struct *stk=new label_struct;
		if(stk==NULL)throw gerror_str("Out of memory");
		stk->line=line;
		stk->pos=pos;
		strcpy(stk->label,name);
		queue_tail(stk);
	}
	//returns line number if found, -1 if not found
	int label_queue::find_label(const char *name,int *pos){
		label_struct *skt=find_val(name);
		if(skt){
			*pos=skt->pos;
			return skt->line;
		}
		return -1;
	}
	int label_queue::find_label(const char *name){
		label_struct *skt=find_val(name);
		if(skt){
			return skt->line;
		}
		return -1;
	}
	void label_queue::clear_label(void){
		label_struct *tmp;
		while((tmp=unqueue_tail())){
			delete tmp;
		}
	}


	void var_queue::add_var(const char *name,const char *val){
		var_struct *tmp=find_var(name);
		if(tmp){
			strcpy(tmp->val,val);
			return;
		}
		tmp=new var_struct;
		if(tmp==NULL)throw gerror_str("Out of memory");
		strcpy(tmp->name,name);
		strcpy(tmp->val,val);
		queue_tail(tmp);
	}
	
	var_struct * var_queue::find_var(const char *name){
		var_struct *skt=find_val(name);
		if(skt){
			return skt;
		}
		return NULL;
	}

	//return 0 is no var found, else reutrn value in val
	int var_queue::get_var_val(const char *name,char *val){
		var_struct *skt=find_val(name);
		if(skt){
			strcpy(val,skt->val);
			return 1;
		}
		return 0;
	}

	void var_queue::clear_vars(void){
		var_struct *tmp;
		while((tmp=unqueue_tail())){
			delete tmp;
		}
	}


	
	void gCScript_tel::getstr(char *str){
		if(waittimeout>0){
			time_t start,end;
			time(&start);
			end=start;
			while(file_rdy(0,300000)==0){
				if(need_exit()){
					str[0]=0;
					return;
				}
				time(&end);
				if( (end-start)>waittimeout )
					throw gerror_str("Timeout get data in file ",ERR_TIMEOUT)<<filename<<" line "<<line_number;
			}
		}
		try{
			gTelnet::getstr(str);
		}catch(gerror_str e){
			throw e.set_err(ERR_SOCKET_CLOSED)<<" in file "<<filename<<" line "<<line_number;
		}
	}


	gCScript_tel::gCScript_tel(void){
		varsq=new var_queue;
		stackq=new stack_queue;
		labelq=new label_queue;

		OPEN="open";
		COMMENT="//";
		CLOSE="close";
		WAITFOR="waitfor";
		WAITSTR="waitstr";
		RECVWTO="recvwto";
		SEND="send";
		MANUINPUT="manuinput";
		SENDSTR="sendstr";
		SENDROW="sendrow";
		DEFINE="#define";
		DEFINESTR="#definestr";
		CALL="call";
		CLEAR="clear";
		LOGTO="logto";
		LOGSTOP="logstop";
		ECHO="echo";
		SLEEP="sleep";
		USERINPUT="userinput";
		SENDCTRLC="send^c";
		RETURN="return";
		GOTO="goto";
		TIMEOUT="timeout";
		mixbuf[0]=0;
		//filename[0]=0;
		line_number=0;
		debugmode=0;
		waittimeout=-1;
		fp=NULL;
		thelogfp=NULL;
	}
	gCScript_tel::~gCScript_tel(){
		clear_vars();
		if(varsq)delete varsq;
		if(labelq)delete labelq;
		if(stackq)delete stackq;
		if(thelogfp)fclose(thelogfp);
	}

	//void MessagePrint(const char *buf)
	//{
	//	printf("%s",buf);
	//	if(thelogfp)fprintf(thelogfp,"%s",buf);
	//}

	

	void gCScript_tel::clear_vars(void){
		varsq->clear_vars();
	}

	void gCScript_tel::add_var(const char *name,const char *val){
		varsq->add_var(name,val);
	}

	//if a $ sign infront of a name, it is a varible
	//search for str, copy the result to res
	//returb 1 if get, 0 if not exist
	void gCScript_tel::search_var(char *name){
		if(!varsq->get_var_val(name,name)){
			throw gerror_str("Var ",ERR_SCRIPTEL_VAR_NOT_FOUND)<<name<<" not found in "<<filename<<" line "<<line_number;
		}
	}

	void gCScript_tel::search_var_str(const char *str,char *finalres){
		char res[VAR_LEN+1];
		int i=0;
		for(;;){
			while(*str!='$' && *str){
				res[i++]=*str;
				str++;
			}
			if(*str==0)break;
			else
			if(*(str+1)!='('){
				res[i++]=*str;
				str++;
				continue;
			}else{
				str+=2;		//step out the $( field
				char tmp[VAR_LEN+1];
				int j=0;
				while( (*str) && (*str!=')') ){
					tmp[j++]=*str;
					str++;
				}
				tmp[j]=0;
				search_var(tmp);
				res[i]=0;
				strcat(res,tmp);
				i+=strlen(tmp);
				if(*str!=')'){
					throw gerror_str("unmatch ) in ",ERR_SCRIPTEL_UNMATCH)<<filename<<" line "<<line_number;
				}
				str++;
			}
		}
		res[i]=0;
		strcpy(finalres,res);
	}

	//return 0 means nothing in the str, 1 means some thing need to be processed
	int gCScript_tel::resove_vars(char *buf,char *word){
		if(strlen(buf)<2)return 0;
		if(*buf==COMMENTCHAR)return 0;

		word[0]=0;
		sscanf(buf,"%s",word);
		if(!strcmp(word,DEFINE)){
			return 1;
		}else if(!strcmp(word,DEFINESTR)){
			return 1;
		}

		int var_depth_count=0;
		while(hasvarible(buf)){
			search_var_str(buf,buf);
			var_depth_count++;
			if(var_depth_count>10){
				throw gerror_str("Varible de-reference deeper than 10",ERR_SCRIPTEL_VAR_DEFC)<<" counts in "<<filename<<" line "<<line_number;
			}
		}
		return 1;
	}

	void gCScript_tel::str2low(char *str){
		while(*str){
			*str=tolower(*str);
			str++;
		}
	}

	void gCScript_tel::waitfor(const char *str){
		try{
		while(1){
			getstr(buf);
			if(need_exit())break;
			MessagePrint(buf);
			strcat(mixbuf,buf);
			char *ptr=strstr(mixbuf,str);
			if(ptr!=NULL){
				strcpy(mixbuf,ptr+strlen(str));
				return;
			}
			//shrink the extras (no keyword found side)
			if(strlen(mixbuf)>TEL_BUFFER_SIZE*2){
				strcpy(mixbuf,mixbuf+TEL_BUFFER_SIZE);
			}
		}
		}catch(gerror_str g){
			g=gerror_str("Timeout wait for ",ERR_SOCKET_CLOSED)<<str<<" in file "<<filename<<" line "<<line_number;
			throw g;
		}
	}

	//return true if the string has a varible
	int gCScript_tel::hasvarible(const char *str){
		for(int i=0;i<TEL_BUFFER_SIZE;i++){
			if(*(str+i)=='$')
				if(*(str+i+1)=='(')return 1;
			if(*(str+i)==0)break;
		}
		return 0;
	}

	char *gCScript_tel::pass1word(char *str){
		char *tmp=str;
		while(*tmp<=' ' && *tmp)tmp++;
		while(*tmp>' ')
			tmp++;
		if(*tmp)tmp++;		//get ride of the space
		return tmp;
	}

	int gCScript_tel::find_label(const char *name,int *line_num){
		int pos;
		int num=labelq->find_label(name,&pos);
		if(num<0){
			throw gerror_str("Label ",ERR_SCRIPTEL_LABEL_NOT_FOUND)<<name<<" not found in file "<<filename<<" line "<<line_number;
		}

		if(line_num)*line_num=num;
		return pos;
	}

	void gCScript_tel::pop_stack(void){
		int pos;
		
		if(stackq->pop(&line_number,&pos))
			fseek(fp,pos,SEEK_SET);
		else{
			throw gerror_str("Return without call in file ",ERR_SCRIPTEL_UNMATCH)<<filename<<" line "<< line_number;
		}
	}

	void gCScript_tel::preprocessor(FILE *tfp,char *tfilename,int linenum){
		char buf[512];
		char word[128];	//hold keyword	(waitfor, send...)
		char str[128];	//hold second word	
		char val[128];	//hold third word (for defines only)

		fp=tfp;
		filename=tfilename;
		line_number=linenum;

		//get all labels. not efficient
		while(fgets(buf,512,fp)){
			line_number++;
			
			if(resove_vars(buf,word)==0)continue;
			sscanf(buf,"%s %s %s",word,str,val);
			int len=strlen(word);
			if(len>=2){
				if(!strcmp(word,DEFINE)){
					add_var(str,val);
				}else if(!strcmp(word,DEFINESTR)){
					char *tmp=pass1word(pass1word(buf));
					add_var(str,tmp);
				}else if(word[len-1]==LABEL_CHAR){
					word[len-1]=0;
					labelq->add_label(word,line_number,ftell(fp));
				}
			}
		}
		fseek(fp,0,SEEK_SET);
		line_number=linenum;
	}

	//fp is an opened filepointer
	//returns line number
	//return -1 if need quit
	//or it will quit (but wont't return -1) when need_exit() is true
	int gCScript_tel::doscript(){//FILE *tfp,char *tfilename,int linenum=0){
		char buf[512];
		char word[128];	//hold keyword	(waitfor, send...)
		char str[128];	//hold second word	
		char val[128];	//hold third word (for defines only)

		/*
		fp=tfp;
		filename=tfilename;
		line_number=linenum;

		//get all labels. not efficient
		while(fgets(buf,512,fp)){
			line_number++;
			
			if(resove_vars(buf,word)==0)continue;
			sscanf(buf,"%s %s %s",word,str,val);
			int len=strlen(word);
			if(len>=2){
				if(!strcmp(word,DEFINE)){
					add_var(str,val);
				}else if(!strcmp(word,DEFINESTR)){
					char *tmp=pass1word(pass1word(buf));
					add_var(str,tmp);
				}else if(word[len-1]==LABEL_CHAR){
					word[len-1]=0;
					labelq->add_label(word,line_number,ftell(fp));
				}
			}
		}
		fseek(fp,0,SEEK_SET);
		line_number=linenum;
		*/

		//scripting
		//while(fgets(buf,512,fp)){
			if(fgets(buf,512,fp)==NULL)return -1;
			//if(need_exit())break;
			line_number++;

			if(debugmode){
				gerror_str dbgmsg("");
				dbgmsg<<line_number<<" "<<buf;
				MessagePrint(dbgmsg.get_str());
			}
			
			if(resove_vars(buf,word)==0)
				return line_number;
			//continue;

			word[0]=0;
			str[0]=0;
			val[0]=0;
			sscanf(buf,"%s %s %s",word,str,val);
			str2low(word);
			if(!strcmp(word,WAITFOR)){
				if(str[0]==0){
					strcpy(str,"\xa");
					//throw gerror_str("Wait for NULL charactor in ",NULL_ERR)<<filename<<" line "<<line_number;
				}
				waitfor(str);
			}else if(!strcmp(word,WAITSTR)){
				char *waitstr=pass1word(buf);
				if(*waitstr==0){
					strcpy(str,"\xd\xa");
					//throw gerror_str("Wait for NULL charactor in ",NULL_ERR)<<filename<<" line "<<line_number;
				}
				char *tmp=waitstr+strlen(waitstr);
				tmp--;		//don't try the 0
				while( (tmp>buf) && ( (*tmp==0x0a) || (*tmp==0x0d) ) )tmp--;
				*tmp=0;
				waitfor(waitstr);
			}else if(!strcmp(word,SENDSTR)){
				if(buf[strlen(buf)-1]!='\n')strcat(buf,"\n");
				char *tmp=pass1word(buf);
				//while(*tmp<=20 && *tmp)tmp++;
				//tmp+=strlen(SENDSTR);
				//if(*tmp)tmp++;		//get ride of the space
				if(*tmp)sendstr(tmp);
			}else if(!strcmp(word,SEND)){
				strcat(str,"\n");
				sendstr(str);
			}else if(!strcmp(word,CALL)){
				int filepos=ftell(fp);
				stackq->push(line_number,filepos);

				filepos=find_label(str,&line_number);
				fseek(fp,filepos,SEEK_SET);
			}else if(!strcmp(word,GOTO)){
				int filepos=find_label(str,&line_number);
				fseek(fp,filepos,SEEK_SET);
			}else if(!strcmp(word,RETURN)){
				pop_stack();
			}else if(!strcmp(word,DEFINE)){
				add_var(str,val);
			}else if(!strcmp(word,DEFINESTR)){
				char *tmp=pass1word(pass1word(buf));
				add_var(str,tmp);
			}else if(!strcmp(word,CLEAR)){
				clear_vars();
			}else if(!strcmp(word,OPEN)){
				try{
					if(val[0]==0)
						open(str);
					else{
						int port;
						sscanf(val,"%i",&port);
						open(str,port);
					}
				}catch(gerror_str g){
					if(val[0]==0)
						throw gerror_str("Can't open ",g.get_err())<<str<<" in "<<filename<<" line "<<line_number<<" ("<<g.get_str()<<")";
					else
						throw gerror_str("Can't open ",g.get_err())<<str<<" port "<<val<<" in "<<filename<<" line "<<line_number<<" ("<<g.get_str()<<")";
				}
			}else if(!strcmp(word,CLOSE)){
				close();
			}else if(!strcmp(word,LOGTO)){
				strcpy(val,"ab");			//it is the default
				sscanf(buf,"%s %s %s",word,str,val);
				if(thelogfp)fclose(thelogfp);
				thelogfp=fopen(str,val);
				if(thelogfp==NULL){
					throw gerror_str("Can't open  ",ERR_OPEN_FILE)<<str<<" for log inout in "<<filename<<" line "<<line_number;
				}
			}else if(!strcmp(word,LOGSTOP)){
				if(thelogfp)fclose(thelogfp);
				thelogfp=NULL;
			}else if(!strcmp(word,ECHO)){
				char *echostr=pass1word(buf);
				if(*echostr!=0){
					MessagePrint(echostr);
				}else{
					MessagePrint("\n");
				}
			}else if(!strcmp(word,SENDROW)){
				int rowi;
				sscanf(str,"%x",&rowi);
				char row=rowi;
				send(&row,1);
			}else if(!strcmp(word,SLEEP)){
				DWORD s=0;
				sscanf(str,"%i",&s);
				Sleep(s*1000);
			}else if(!strcmp(word,RECVWTO)){
				int timeout=0;
				sscanf(str,"%i",&timeout);
				time_t start,now;
				time(&start);
				while(1){
					if(need_exit())break;
					int res=file_rdy(1);
					if(res>0){
						time(&start);
						getstr(buf);
						if(need_exit())break;
						MessagePrint(buf);
					}else if(res<0){
						throw gerror_str("Socket closed",ERR_SOCKET_CLOSED);
					}else
						break;
					time(&now);
					if((now-start)>timeout)
						break;
				}
			}else if(!strcmp(word,USERINPUT)){
				Input_gets(buf);strcat(buf,"\n");
				sendstr(buf);
			}else if(!strcmp(word,MANUINPUT)){
				manu_input_thread* inpt=
					new manu_input_thread(this,this);
				if(inpt){
					if(inpt->CreateThread()==0){
						delete inpt;
						inpt=NULL;
					}
				}
				if(inpt==NULL){
					ErrorMessagePrint("Can't start manuinput thread");
				}else{
				PromptMessagePrint("Type exit to exit manu mode");
				char typed[6];
				memset(typed,0,6);
				while(1){
					if(need_exit())break;
					for(int i=1;i<5;i++){
						typed[i-1]=typed[i];
					}
					typed[4]=Get_A_char();
					//_getch() count CR as 0x0d
					if(typed[4]==0x0d)typed[4]=0x0a;
					if(!strcmp(typed,"exit\n")){
						break;
					}
					if(send(&typed[4],1)<0)break;
					typed[4]=tolower(typed[4]);
				}
				}
				if(inpt)delete inpt;
				//delete exit char
				send("\x8\x8\x8\x8",4);
			}else if(!strcmp(word,SENDCTRLC)){
				char c=3;
				send(&c,1);
			}else if(!strcmp(word,TIMEOUT)){
				//int timeout=0;
				sscanf(str,"%i",&waittimeout);
				//set_socket_timeout(timeout);
			}else if(!strncmp(word,COMMENT,2)){
			}else if(strlen(word)<2){
				throw gerror_str("Unkonwn keyword ",ERR_SCRIPTEL_UNKNOWN_KEYWORD)<<word<<" in "<<filename<<" line "<<line_number;
			}else if(word[strlen(word)-1]==LABEL_CHAR){
				//continue;
			}else{
				throw gerror_str("Unkonwn keyword ",ERR_SCRIPTEL_UNKNOWN_KEYWORD)<<word<<" in "<<filename<<" line "<<line_number;
			}
			//if(need_exit())break;
		//}
		return line_number;
	}

	
	void gCScript_tel::doall(FILE *tfp,char *tfilename){
		preprocessor(tfp,tfilename);
		while(doscript()>=0){
			if(need_exit())break;
		}
	}




/*
template<class T,class S>
class gen_queue{
	struct queue_struct{
		T *val;
		queue_struct *prev;
		queue_struct *next;
	};
protected:
	queue_struct *head;
	queue_struct *tail;
public:
	gen_queue(void){
		head=tail=NULL;
	}
	~gen_queue(void){
		clear();
	}
	void queue_tail(T *val){
		queue_struct *element=new queue_struct;
		element->val=val;
		if(head==NULL){
			head=tail=element;
			element->prev=NULL;
			element->next=NULL;
			return;
		}else{
			element->prev=tail;
			element->next=NULL;
			tail->next=element;
			tail=element;
		}
	}
	T* unqueue_tail(void){
		queue_struct *tmp=tail;
		if(tail){
			if(head==tail){
				head=tail=NULL;
			}else{
				tail=tail->prev;
				tail->next=NULL;
			}
			T *tt=tmp->val;
			delete tmp;
			return tt;
		}
		return NULL;
	}


	virtual T *find(S s){
		queue_struct *tmp=head;
		while(tmp){
			if(*tmp->val==s){
				return tmp->val;
			}
			tmp=tmp->next;
		}
		return NULL;
	}

	void clear(void){
		while(head){
			tail=head;
			head=head->next;
			delete tail;
		}
		head=tail=NULL;
	}
};
*/
