//////////////////////////////////////////////////////////////////////////
//
//	Program:	gerror.cpp
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

#include <strstream>
#include "gerror.h"
using namespace std;

//class GANG_DLL_EXPORT errstr_ostrstream
//		char *str;
//		int size;
//		int curpos;
//	public:
		errstr_ostrstream::errstr_ostrstream(char *buf,int bufsize)
			:str(buf),size(bufsize),curpos(0){
		}
		errstr_ostrstream & errstr_ostrstream::seekp(const int pos){
			curpos=pos;
			return *this;
		}
		errstr_ostrstream & errstr_ostrstream::operator<<(const char*opstr){
			ostrstream os(str,size);
			os.seekp(curpos);
			os<<opstr;
			curpos=os.tellp();
			return *this;
		}
		errstr_ostrstream & errstr_ostrstream::operator<<(const int err){
			ostrstream os(str,size);
			os.seekp(curpos);
			os<<err;
			curpos=os.tellp();
			return *this;
		}


//class gerror_str{
//	enum{
//		MAX_ERRSTR_LEN=128
//	};
//	const char *err_class;
//protected:
//	errstr_ostrstream errstrm;
	//ostrstream errstrm;
//	char str[MAX_ERRSTR_LEN];
//	int error_num;
	void gerror_str::add_end_str(){
		int len=errstrm.tellp();
		if( (len>=0) && (len<MAX_ERRSTR_LEN) ){
			m_str[len]=0;
			return;
		}
		m_str[MAX_ERRSTR_LEN-1]=0;
	}
	void gerror_str::init(){
		err_class="gerror_str";
	}

	
	gerror_str::gerror_str(const char *str1,const GERROR_CODES err_num)
		:errstrm(m_str,MAX_ERRSTR_LEN)
	{
		init();
		m_str[0]=0;
		m_error_num=err_num;
		get_sys_error();
		if(str1)
			errstrm<<str1;
		else
			last_err();
		add_end_str();
	}

	//
	//This will cause this method been called on the
	//catch side and over write err_class.  It is also a un_nessessary step
	//
	gerror_str::gerror_str(const gerror_str &err)
		:errstrm(m_str,MAX_ERRSTR_LEN)
	{
		//init();	//should change to 
		err_class=err.get_class();
		m_error_num=err.m_error_num;
		m_sys_error_num=err.m_sys_error_num;
		errstrm<<err.m_str;
		add_end_str();
	}

	//gerror_str(char *str1,char *str2,int err_num=0){
	//	error_num=err_num;
	//	strcpy(str,str1);
	//	strcat(str,str2);
	//}



	gerror_str & gerror_str::set_err(const gerror_str &e){
		m_error_num=e.m_error_num;
		m_sys_error_num=e.m_sys_error_num;
		return *this;
	}

	gerror_str & gerror_str::set_err(const GERROR_CODES e){
		m_error_num=e;
		return *this;
	}

	gerror_str & gerror_str::set_class(const char*clas){
		err_class=clas;
		return *this;
	}
	
	const char *gerror_str::get_class() const {
		return err_class;
	}

	void gerror_str::operator=(const gerror_str &err){
		m_error_num=err.m_error_num;
		m_sys_error_num=err.m_sys_error_num;
		errstrm.seekp(0);
		errstrm<<err.m_str;
		add_end_str();
	}

	gerror_str & gerror_str::operator+(const gerror_str &err){
		errstrm<<err.m_str;
		add_end_str();
		return *this;
	}

	gerror_str & gerror_str::operator<<(const gerror_str &err){
		errstrm<<err.m_str;
		add_end_str();
		return *this;
	}

	gerror_str & gerror_str::operator<<(const char *pstr){
		if(pstr!=NULL)errstrm<<pstr;
		add_end_str();
		return *this;
	}

	gerror_str & gerror_str::operator<<(const int err){
		errstrm<<err;
		add_end_str();
		return *this;
	}
	//gerror_str(int err_num,char *fmt,...){
	//	error_num=err_num;
	//	va_list marker;
	//	va_start( marker, fmt );
	//	vsprintf(str,fmt,marker); 
	//	va_end( marker );
	//}
	
	
	//get the last error of the process
	

	gerror_str & gerror_str::last_err(LPCSTR msg,int code){
		m_error_num=ERR_FALSE;
		m_sys_error_num=code;		

		if(msg)errstrm<<msg;
#ifdef WIN32
		char* lpMsgBuffer;
		add_end_str();

        if(FormatMessage(
			FORMAT_MESSAGE_ALLOCATE_BUFFER |
			FORMAT_MESSAGE_FROM_SYSTEM,
			NULL,
			m_error_num,
			MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), //The user default language
			(LPTSTR) &lpMsgBuffer,
			0,
			NULL )==0)
		{
			errstrm<<"Can't get last error";
		}else{
			errstrm<<lpMsgBuffer;
		}
		add_end_str();
		LocalFree( lpMsgBuffer );
#endif
		return *this;
	}


