//////////////////////////////////////////////////////////////////////////
//
//	Program:	gerror.h
//	DESC:		implement common throwable object
//	Author:		Gang Zhang
//	E-mail:		gzhang_ocx@yahoo.com or gazhang@geocities.com
//	HTTP:		http://members.xoom.com/gzhang
//	Date:		Apr 16, 1998
//	Revisions:
//			Aug 23, 1998  Seperate from gtcpip.h to this file
//
//
//	Revision 1.1:	Aug 28,1998 Add GANG_DLL_EXPORT
//			#define GANG_DLL_EXPORT __declspec( dllexport )
//			will make it exportable
//	Revision 1.2:	Oct 24,1998 Add err_class
//			Now all catches should catch gerror_str
//			so no throw will be missed, and use get_class to 
//			see what classes is throwed
//////////////////////////////////////////////////////////////////////////
#ifndef gError_strGANG_ZHANGHEADERFILE
#define gError_strGANG_ZHANGHEADERFILE

//#include <strstrea.h>
//#include <iostream.h>
#include <windows.h>


#ifndef GANG_DLL_EXPORT
#define GANG_DLL_EXPORT
#endif

enum GERROR_CODES{
	ERR_OK=0,				//no error
	ERR_SOCKET=0x100,		//General Socket error
	ERR_SOCKET_ALLOC,		//can't allocate socket
	ERR_SOCKET_CLOSED,		//socket closed
	ERR_SOCKET_CONNECT,
	ERR_SOCKET_BIND,		//ERROR_BIND
	ERR_SOCKET_PORT_ALLOC,	//can't allocate port
	ERR_SOCKET_SETNOBLOCK,	//can;t set to noblock mode
	ERR_SOCKET_GETHOST,
	ERR_SOCKET_LISTEN,
	ERR_SOCKET_INVALID,		//socket invalid
	ERR_SOCKET_CONNECTTIMEOUT,
	ERR_SOCKET_TIMEOUT,		//socket timeout
	ERR_SOCKET_NOT_INIT,	//socket not initialized
	ERR_SOCKET_NOSUPPORT,	//not supported
	ERR_PROXY,
	ERR_PROXY_SERVER,		//general proxy server err
	ERR_PROXY_RULE,
	ERR_PROXY_NREACH,		//Network unreachable
	ERR_PROXY_REFUSE,		//Connection refused
	ERR_PROXY_TIMEOUT,		//TTL expired
	ERR_PROXY_NOSUPPORT,	//Command not supported
	ERR_PROXY_HOST_NREACH,	//Host unreachable

	ERR_POP3_BUFLIMIT,		//pop3 line too long

	ERR_TIMEOUT=0x200,		//general timeout
	ERR_NO_MEM,				//out of memory
	ERR_SEM_ALLOC=0x300,	//can't alloc sem
	ERR_SEM_DEADLOCK,		//deadlock
	ERR_SEM_PROCESS_DEAD,	//other process hold sem dead
	ERR_SEM_ERR,			//general sem error

	ERR_THREAD_CREATE,		//thread creat failed
	ERR_THREAD_CREATE_AGAIN,//Try to create a thread again in the same class

	//mm sys (sound)
	ERR_MMSYS_NODRIVER,		//no driver
	ERR_MMSYS_BADID,		//Bad device ID
	ERR_MMSYS_DEVOPEN,		//Can't open device
	ERR_MMSYS_DEVNOTOPEN,	//need open device first
	ERR_MMSYS_RESET,		//reset error
	ERR_MMSYS_PREPHDR,		//prepare header err
	ERR_MMSYS_ADDBUF,		//error queue sound buf
	ERR_MMSYS_START,		//can't start
	ERR_MMSYS_PLAY,		//sound out err
	ERR_MMSYS_STOP,		//can't stop
	ERR_MMSYS_PAUSE,
	ERR_MMSYS_GETVOL,
	ERR_MMSYS_SETVOL,
	ERR_MMSYS_CLOSE,		//Can't close

	ERR_SQL_ALLOC_ENV,			//Can't alloc evn handle
	ERR_SQL_ALLOC_CONNECTION,	//can't alloc connect handle
	ERR_SQL_DSN_CONNECT,		//Can't connect to dsn
	ERR_SQL_ALLOC_STATEMENT,	//Can't allocate statement
	ERR_SQL_INVALID_HANDLE,
	ERR_SQL_COMMIT,
	ERR_SQL_ERR,
	ERR_SQL_STILL_EXECUTING,
	ERR_SQL_NO_DATA_FOUND,


	ERR_OPEN_FILE,

	//scriptel error
	ERR_SCRIPTEL_LABEL_NOT_FOUND,
	ERR_SCRIPTEL_UNMATCH,
	ERR_SCRIPTEL_VAR_NOT_FOUND,
	ERR_SCRIPTEL_VAR_DEFC,
	ERR_SCRIPTEL_UNKNOWN_KEYWORD,

	ERR_FALSE=(~0)			//general ERROR
};
//using namespace std;

class GANG_DLL_EXPORT errstr_ostrstream{
		char *str;
		int size;
		int curpos;
	public:
		errstr_ostrstream(char *buf,int bufsize);
		int tellp() const {
			return curpos;
		}
		errstr_ostrstream & seekp(const int pos);
		errstr_ostrstream & operator<<(const char*opstr);
		errstr_ostrstream & operator<<(const int err);
	};

class GANG_DLL_EXPORT gerror_str{
	enum{
		MAX_ERRSTR_LEN=128
	};
	const char *err_class;
protected:
	errstr_ostrstream errstrm;
	//ostrstream errstrm;
	char m_str[MAX_ERRSTR_LEN];
	GERROR_CODES m_error_num;
	DWORD m_sys_error_num;
	char * m_function_name;
	void add_end_str();
	virtual void init();
public:
	
	//inform in which function the error happens
	void set_func_name(char *pname){
		m_function_name=pname;
	}
	char *get_func_name() const {
		return m_function_name;
	}

	DWORD get_sys_error(){
		return m_sys_error_num;
	}
	void set_sys_error(){
		m_sys_error_num=GetLastError();
	}
	gerror_str(const char *str1=NULL,const GERROR_CODES err_num=ERR_FALSE);

	//
	//This will cause this method been called on the
	//catch side and over write err_class.  It is also a un_nessessary step
	//
	gerror_str(const gerror_str &err);


	inline char *get_str(void){
		return m_str;
	}

	inline int get_len(){
		return errstrm.tellp();
	}
	inline GERROR_CODES get_err(void){
		return m_error_num;
	}

	gerror_str & set_err(const gerror_str &e);

	gerror_str & set_err(const GERROR_CODES e);

	gerror_str & set_class(const char*clas);
	virtual const char *get_class() const;
	//const char *gerror_str::get_class() const 
	//{
	//	return err_class;
	//}

	void operator=(const gerror_str &err);

	gerror_str & operator+(const gerror_str &err);

	gerror_str & operator<<(const gerror_str &err);

	gerror_str & operator<<(const char *pstr);

	gerror_str & operator<<(const int err);
	
	
	
	//get the last error of the process
	inline gerror_str & last_err(LPCSTR msg=NULL){
		set_sys_error();
		return last_err(msg,get_sys_error());
	}

	gerror_str & last_err(LPCSTR msg,int code);
};

#endif

