//////////////////////////////////////////////////////////////////////
//
//	Program:	creg.h
//	Author:		Gang Zhang
//	E-mail:		gzhang_ocx@yahoo.com or gazhang@geocities.com
//	Version:	1.0
//	Date:		08/23/1998
//
//	Description:	Registry read/write key functions
//	Note:		delete_key() can only used to delete a subkey
//			not the key called it.
//			!!!! 09/28/1998 must define STRICT when use
//			as dll, otherwise symbol table might not match
//	Lib:		advapi32.lib
//
//
//Simple usage:
//try{
//	GCRegKey key(&GCRegKey(),"Software\\Microsoft\\Windows\\CurrentVersion\\Internet Settings");
//	key.ReadRegStr("ProxyServer",msg,512);
//}catch(gerror_str e){
//}
//
//////////////////////////////////////////////////////////////////////	

#ifndef CGANGREG_HEADER_FILE 
#define CGANGREG_HEADER_FILE 

#ifndef STRICT
#define STRICT TRUE
#endif

#ifndef GANG_DLL_EXPORT
#define GANG_DLL_EXPORT
#endif

#include <windows.h>
#include <winreg.h>
#include "gerror.h"

class GANG_DLL_EXPORT gReg_error_str:public gerror_str{
protected:
	virtual void init(){
		set_class("gReg_error_str");
	}
public:
	gReg_error_str(const char *str1=NULL,const GERROR_CODES err_num=ERR_OK):gerror_str(str1,err_num){
		init();
	}
	gReg_error_str(const gerror_str &err):gerror_str(err){
		init();
	}
};

class GANG_DLL_EXPORT GCRegKey{
	enum{
		REG_MAX_LEN=512
	};
	HKEY m_regKey;
	BOOL need_close;
public:
	GCRegKey(HKEY toplevel=HKEY_CURRENT_USER);

	//CRegKey rg(&CRegKey(HKEY_CURRENT_USER),"Software\\Bajie Soft\\Test");
	//CRegKey rg(&CRegKey(),"Software\\Bajie Soft\\Test");
	GCRegKey(GCRegKey *rkey,const char *subkey,BOOL create=TRUE,DWORD regoption=REG_OPTION_NON_VOLATILE,REGSAM acc=KEY_ALL_ACCESS,LPSECURITY_ATTRIBUTES sec=NULL);

	void Open(HKEY key,const char *subkey,REGSAM acc=KEY_ALL_ACCESS,LPSECURITY_ATTRIBUTES sec=NULL);

	void Create(HKEY key,const char *subkey,DWORD regoption=REG_OPTION_NON_VOLATILE,REGSAM acc=KEY_ALL_ACCESS,LPSECURITY_ATTRIBUTES sec=NULL);

	LONG nextKey(DWORD index,char *buf,DWORD len);

	~GCRegKey();

	void Close();

	void WriteRegStr(LPCSTR keyname, LPCSTR val);

	void DeleteSubKey(const char *subkey);

	void RegDeleteValue(const char *valuename);
 

	void ReadRegStr(LPCSTR keyname, char * val,DWORD size);


	void WriteRegInt(LPCSTR keyname,int val);

	int ReadRegInt(LPCSTR keyname);
};


#endif
