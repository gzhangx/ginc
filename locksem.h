/////////////////////////////////////////////////////////////////////////////
//
//	File:		locksem.h
//	Author:		Gang Zhang (gzhang@icx.net)
//	E-mail:		gzhang_ocx@yahoo.com or gazhang@geocities.com
//	HTTP:		http://members.xoom.com/gzhang
//	Date:		Aug 29,1998
//	Description:	Semaphore class
//
//	Revisions:	1.0 Aug 29, 1998 GZ Seperate from gqueue.h
//
/////////////////////////////////////////////////////////////////////////////

#ifndef GCTSLOCKSEMHEADERFILE
#define GCTSLOCKSEMHEADERFILE

#ifndef GANG_DLL_EXPORT
#define GANG_DLL_EXPORT
#endif

#include "gerror.h"

class GANG_DLL_EXPORT gSem_error_str:public gerror_str{
protected:
	virtual void gSem_error_str::init(){
		set_class("gSem_error_str");
	}
public:
	gSem_error_str(const char *str1=NULL,const GERROR_CODES err_num=ERR_OK):gerror_str(str1,err_num){
		init();
	}
	gSem_error_str(const gerror_str &err):gerror_str(err){
		init();
	}
};

//class single initially signaled lock semaphore
class GANG_DLL_EXPORT gCLockSem{
	HANDLE hsem;
	//CRITICAL_SECTION hsem;
	void init_semaphore_protection(const BOOL bInitialOwner,const BOOL bManualReset,LPCSTR lpName);
public:

	HANDLE get_handle() const {
		return hsem;
	}

	//create initialy open, auto reset semaphore
	gCLockSem(const BOOL bInitialOwner=TRUE,const BOOL bManualReset=FALSE,LPCSTR lpName=NULL){
		init_semaphore_protection(bInitialOwner,bManualReset,lpName);
	}

	virtual ~gCLockSem(){
		if(hsem)CloseHandle(hsem);
		//DeleteCriticalSection(&hsem);
	}

	//WAIT_OBJECT_0	The state of the specified object is signaled.
	//WAIT_TIMEOUT	The time-out interval elapsed, and the object's 
	DWORD GWait(int tm=INFINITE){
		//EnterCriticalSection(&hsem);
		return WaitForSingleObject(hsem,tm);
	}

	void GSignal(){
		//LeaveCriticalSection(&hsem);
		//ReleaseMutex(hsem);
		SetEvent(hsem);
	}
	void GReset(){
		//LeaveCriticalSection(&hsem);
		//ReleaseMutex(hsem);
		ResetEvent(hsem);
	}
};
#endif

