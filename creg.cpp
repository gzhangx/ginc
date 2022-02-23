//////////////////////////////////////////////////////////////////////
//
//	Program:	creg.cpp
//	Author:		Gang Zhang
//	E-mail:		gzhang_ocx@yahoo.com or gazhang@geocities.com
//	HTTP:		http://members.xoom.com/gzhang
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
//////////////////////////////////////////////////////////////////////	


#ifndef STRICT
#define STRICT TRUE
#endif



#include "creg.h"

	//=HKEY_CURRENT_USER
	GCRegKey::GCRegKey(HKEY toplevel){
		m_regKey=toplevel;
		need_close=FALSE;
	}

	//CRegKey rg(&CRegKey(HKEY_CURRENT_USER),"Software\\Bajie Soft\\Test");
	//CRegKey rg(&CRegKey(),"Software\\Bajie Soft\\Test");
	//acc=KEY_ALL_ACCESS
	GCRegKey::GCRegKey(GCRegKey *rkey,const char *subkey,BOOL create,DWORD regoption,REGSAM acc,LPSECURITY_ATTRIBUTES sec){
		m_regKey=NULL;
		if(create)
			Create(rkey->m_regKey,subkey,regoption,acc,sec);
		else
			Open(rkey->m_regKey,subkey,acc,sec);
	}

	void GCRegKey::Open(HKEY key,const char *subkey,REGSAM acc,LPSECURITY_ATTRIBUTES sec){
		Close();
		if(subkey==NULL)throw gReg_error_str("Can't open NULL key");
		if(RegOpenKeyEx( key, subkey, 0,acc, &m_regKey)!=ERROR_SUCCESS){
			throw gReg_error_str("Open Key error ")<<subkey;
			//e.last_err();
			//throw e;
		}
		need_close=TRUE;
	}

	void GCRegKey::Create(HKEY key,const char *subkey,DWORD regoption,REGSAM acc,LPSECURITY_ATTRIBUTES sec){
		Close();
		DWORD dw;
		if(subkey==NULL)throw gReg_error_str("Can't create NULL key");
		if(RegCreateKeyEx(key,subkey,0,REG_NONE,regoption, acc, sec,&m_regKey, &dw)!=ERROR_SUCCESS){
			throw gReg_error_str("Create Key error ")<<subkey;
			//e.last_err();
			//throw e;
		}
		need_close=TRUE;
	}

	LONG GCRegKey::nextKey(DWORD index,char *buf,DWORD len){
		return RegEnumKey(m_regKey,index,buf,len);
	}

	GCRegKey::~GCRegKey(){
		Close();
	}

	void GCRegKey::Close(){
		if(need_close){
			if(m_regKey!=NULL){
				if(RegCloseKey(m_regKey)!=ERROR_SUCCESS){
					throw gReg_error_str("Can't close reg");
				}
			}
		}
		m_regKey=NULL;
	}

	void GCRegKey::WriteRegStr(LPCSTR keyname, LPCSTR val)
	{ 
		if(RegSetValueEx(m_regKey,keyname,0,REG_SZ,(CONST BYTE *)val,(DWORD)strlen(val)+1) !=ERROR_SUCCESS){
			throw gReg_error_str("Write str: Can't save ")+gReg_error_str(val)+gReg_error_str(" to ")+gReg_error_str(keyname);
		}
	}

	void GCRegKey::DeleteSubKey(const char *subkey)
	{
		if(::RegDeleteKey(m_regKey,subkey)!=ERROR_SUCCESS){
			GCRegKey key;
			key.Open(m_regKey,subkey);
			FILETIME time;
			TCHAR szBuffer[256];
			DWORD dwSize = 256;
			while (::RegEnumKeyEx(key.m_regKey, 0, szBuffer, &dwSize, NULL, NULL, NULL,&time)==ERROR_SUCCESS)
			{
				key.DeleteSubKey(szBuffer);
				dwSize = 256;
			}
			key.Close();
			if(::RegDeleteKey(m_regKey,subkey)!=ERROR_SUCCESS)
				throw gReg_error_str("Can't delete key ")<<subkey;
		}
	}

	void GCRegKey::RegDeleteValue(const char *valuename){
		if(::RegDeleteValue(m_regKey,valuename)!=ERROR_SUCCESS){
			throw gReg_error_str("Can't delete value ")<<valuename;
			//e.last_err();
			//throw e;
		}
	}
 

	void GCRegKey::ReadRegStr(LPCSTR keyname, char * val,DWORD size)
	{
		val[0]=0;

		DWORD type;
		if(RegQueryValueEx(m_regKey,keyname,0,&type,(BYTE *)val,&size) !=ERROR_SUCCESS){
			throw gReg_error_str("Can't read ")+gReg_error_str(keyname);
		}

		if((int)size>0)val[size-1]=0;
		if(type!=REG_SZ){
			throw gReg_error_str("Wrong Registry type");
		}
	}


	void GCRegKey::WriteRegInt(LPCSTR keyname,int val){
		if(RegSetValueEx(m_regKey,keyname,0,REG_DWORD,(CONST BYTE *)&val,sizeof(val)) !=ERROR_SUCCESS){
			throw gReg_error_str("Can't set ")+gReg_error_str(keyname);			
		}
	}

	int GCRegKey::ReadRegInt(LPCSTR keyname){
		int val;
		DWORD type,size=sizeof(DWORD);
		if(RegQueryValueEx(m_regKey,keyname,0,&type,(BYTE *)&val,&size) !=ERROR_SUCCESS){
			throw gReg_error_str("Can't read int ")+gReg_error_str(keyname);
		}
		return val;
	}



