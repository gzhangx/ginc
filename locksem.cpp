/////////////////////////////////////////////////////////////////////////////
//
//	File:		locksem.cpp
//	Author:		Gang Zhang (gzhang@icx.net)
//	E-mail:		gzhang_ocx@yahoo.com or gazhang@geocities.com
//	HTTP:		http://members.xoom.com/gzhang
//	Date:		Aug 29,1998
//	E-mail:		gzhang_ocx@yahoo.com or gzhang@xoom.com
//	Homepage:	http://members.xoom.com/gzhang
//	Description:	Semaphore class
//
//	Revisions:	1.0 Aug 29, 1998 GZ Seperate from gqueue.h
//
/////////////////////////////////////////////////////////////////////////////

#include "locksem.h"



void gCLockSem::init_semaphore_protection(const BOOL bInitialOwner,const BOOL bManualReset,LPCSTR lpName){
	
	SECURITY_ATTRIBUTES saAttr; 
	saAttr.nLength = sizeof(SECURITY_ATTRIBUTES); 
	saAttr.bInheritHandle = TRUE; 
	saAttr.lpSecurityDescriptor = NULL; 
	hsem=CreateEvent(
		&saAttr,
		bManualReset,
		bInitialOwner,	// flag for initial ownership 
		lpName 	// pointer to mutex-object name  
	);
	
	//InitializeCriticalSection(&hsem);
	if(!hsem)
		throw gSem_error_str();
}


