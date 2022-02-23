//
//	Filename:	scriptel.h
//	Author:		Gang Zhang (gzhang@icx.net)
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
#include <time.h>
#include <gtelnet.h>
#include <gqueue.h>
#include <gcthread.h>
#include <stdio.h>
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

#ifndef GSCRIPT_TELNET_HEADERFILE
#define GSCRIPT_TELNET_HEADERFILE

#ifndef GANG_DLL_EXPORT
#define GANG_DLL_EXPORT
#endif


class GANG_DLL_EXPORT Cscriptel_iomngr{
public:
	virtual void MessagePrint(const char*msg)=0;
	virtual void ErrorMessagePrint(const char*msg)=0;
	virtual void PromptMessagePrint(const char *buf)=0;
	virtual int Get_A_char()=0;
	virtual void Input_gets(char *buf)=0;
};

class GANG_DLL_EXPORT manu_input_thread:public gCThread{
gCTcpIp *tcp;
Cscriptel_iomngr *m_pdsply;
public:
	manu_input_thread(gCTcpIp *ptp,Cscriptel_iomngr *dsp):tcp(ptp),m_pdsply(dsp){}
	BOOL InitInstance();
};

struct stack_struct{
	int line;	//which line
	int pos;	//file pos
	int operator ==(const int line_num) const {
		if(line==line_num)return 1;
		return 0;
	}
};

class stack_queue:public gen_find_queue<stack_struct,int>{
public:
	void push(int line,int pos);
	//return true if there are anything in stack
	int pop(int *line,int *pos);

	void clear_stack(void);
};

#define VAR_LEN 512

struct label_struct{
	char label[VAR_LEN];
	int line;	//which line
	int pos;	//file pos
	int operator ==(const char *name) const {
		return !strncmp(label,name,VAR_LEN);
	}
};

class label_queue:public gen_find_queue<label_struct,const char *>{
public:
	void add_label(const char *name,int line,int pos);
	//returns line number if found, -1 if not found
	int find_label(const char *name,int *pos);
	int find_label(const char *name);
	void clear_label(void);
};


struct var_struct{
	char name[VAR_LEN];
	char val[VAR_LEN];
	int operator ==(const char *vname) const {
		return !strncmp(name,vname,VAR_LEN);
	}
};

class var_queue:public gen_find_queue<var_struct,const char *>{
public:
	void add_var(const char *name,const char *val);
	
	var_struct * find_var(const char *name);

	//return 0 is no var found, else reutrn value in val
	int get_var_val(const char *name,char *val);

	void clear_vars(void);
};

class GANG_DLL_EXPORT gCScript_tel:public gTelnet,public Cscriptel_iomngr{
protected:
	enum{
			COMMENTCHAR='*',
			LABEL_CHAR=':'
	};

	int debugmode;
	FILE *thelogfp;
	int waittimeout;

	char *filename;
	int line_number;
	FILE *fp;
	char buf[TEL_BUFFER_SIZE+1];
	char mixbuf[TEL_BUFFER_SIZE*3+1];
	char *OPEN;		//=open			//open first telnet connection
	char *CLOSE;	//=close		//close the telnet connection
	char *COMMENT;	//=//
	char *WAITFOR;	//=waitfor
	char *WAITSTR;	//wait for a string
	char *RECVWTO;	//recvwith timeout. keep receiving till no more data in timeout periods
	char *SEND;		//=send
	char *SENDSTR;	//=sendstr
	char *SENDROW;	// send a row hex char  sendrow 20 will send a space
	char *SENDCTRLC;
	char *DEFINE;	//=#define
	char *DEFINESTR;//=#definestr
	char *CALL;		//=call	another script file
	char *CLEAR;	//=clear, clears all defined vars
	char *LOGTO;	//=logto
	char *LOGSTOP;	//=logstop
	char *SLEEP;	//sleep for a period of seconds
	char *USERINPUT;	//get user input.  used to do a pause
	char *ECHO;		//=echo		echo what ever string back
	char *RETURN;	//return
	char *GOTO;		//goto
	char *TIMEOUT;
	char *MANUINPUT;
	
	var_queue *varsq;
	stack_queue *stackq;
	label_queue *labelq;
	void getstr(char *str);
public:
	virtual bool need_exit()=0;
	gCScript_tel(void);
	virtual ~gCScript_tel();

	//void MessagePrint(const char *buf)
	//{
	//	printf("%s",buf);
	//	if(thelogfp)fprintf(thelogfp,"%s",buf);
	//}

	void debugon(int level=1){
		debugmode=level;
	}

	void debugoff(void){
		debugmode=0;
	}

	void clear_vars(void);

	void add_var(const char *name,const char *val);

	//if a $ sign infront of a name, it is a varible
	//search for str, copy the result to res
	//returb 1 if get, 0 if not exist
	void search_var(char *name);

	void search_var_str(const char *str,char *finalres);

	//return 0 means nothing in the str, 1 means some thing need to be processed
	int resove_vars(char *buf,char *word);

	void str2low(char *str);

	void waitfor(const char *str);

	//return true if the string has a varible
	int hasvarible(const char *str);

	char *pass1word(char *str);

	int find_label(const char *name,int *line_num=NULL);

	void pop_stack(void);

	void preprocessor(FILE *tfp,char *tfilename,int linenum=0);

	//fp is an opened filepointer
	//returns line number
	//return -1 if need quit
	//or it will quit (but wont't return -1) when need_exit() is true
	int doscript();//FILE *tfp,char *tfilename,int linenum=0)

	void doall(FILE *tfp,char *tfilename);
};
#endif

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
