
#include "CGProcessMem.h"

/*
// Type definitions for pointers to call tool help functions. 
typedef BOOL (WINAPI *MODULEWALK)(HANDLE hSnapshot, 
    LPMODULEENTRY32 lpme);
typedef BOOL (WINAPI *THREADWALK)(HANDLE hSnapshot, 
    LPTHREADENTRY32 lpte); 
typedef BOOL (WINAPI *PROCESSWALK)(HANDLE hSnapshot,
    LPPROCESSENTRY32 lppe); 
typedef HANDLE (WINAPI *CREATESNAPSHOT)(DWORD dwFlags, 
    DWORD th32ProcessID); 
*/
// File scope globals. These pointers are declared because of the need 

// to dynamically link to the functions.  They are exported only by 
// the Windows 95 kernel. Explicitly linking to them will make this
// application unloadable in Microsoft(R) Windows NT(TM) and will 
// produce an ugly system dialog box. 

//class throws char *
  CGProcessMem::CGProcessMem(HANDLE proc,MODULEENTRY32 me32){
    mProc=proc;   
    memcpy(&mMe32,&me32,sizeof(me32));
    //if(InitToolhelp32()!=TRUE){
	  //  throw "Can't init toolhelp";
    //}
  }
  CGProcessMem::~CGProcessMem(){
    close();
    CloseHandle(mProc);
  }

  
  BOOL CGProcessMem::InitToolhelp32 (void) 
  { 
    BOOL   bRet  = FALSE;
    HMODULE hKernel = NULL;
 
    // Obtain the module handle of the kernel to retrieve addresses of
    // the tool helper functions. 
    hKernel = GetModuleHandle("KERNEL32.DLL"); 
 
    if (hKernel){ 
        pCreateToolhelp32Snapshot = 
            (CREATESNAPSHOT)GetProcAddress(hKernel,
            "CreateToolhelp32Snapshot"); 

 
        pModule32First  = (MODULEWALK)GetProcAddress(hKernel, 
            "Module32First"); 
        pModule32Next   = (MODULEWALK)GetProcAddress(hKernel, 
            "Module32Next"); 

        pProcess32First = (PROCESSWALK)GetProcAddress(hKernel, 
            "Process32First");
        pProcess32Next  = (PROCESSWALK)GetProcAddress(hKernel, 
            "Process32Next"); 
 
        pThread32First  = (THREADWALK)GetProcAddress(hKernel, 
            "Thread32First"); 

        pThread32Next   = (THREADWALK)GetProcAddress(hKernel, 
            "Thread32Next");
 
        // All addresses must be non-NULL to be successful. 
        // If one of these addresses is NULL, one of 
        // the needed lists cannot be walked. 
        bRet =  pModule32First && pModule32Next  && pProcess32First && 
                pProcess32Next && pThread32First && pThread32Next &&
                pCreateToolhelp32Snapshot; 
    }
    else 
        bRet = FALSE; // could not even get the module handle of kernel 

 
    return bRet; 
  }


  void CGProcessMem::close(){
    if(mProc!=NULL){
      CloseHandle(mProc);
      mProc=NULL;
    }
  }
  
  CGProcessMem* CGProcessMem::getProcessByName(char *name){
    HANDLE         hProcessSnap = NULL;
    //BOOL           bRet      = FALSE;
    PROCESSENTRY32 pe32      = {0};

    if(pCreateToolhelp32Snapshot==NULL)InitToolhelp32();
    //  Take a snapshot of all processes currently in the system.
    hProcessSnap = pCreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (hProcessSnap == (HANDLE)-1)
        return (FALSE);

    //  Fill in the size of the structure before using it.
    pe32.dwSize = sizeof(PROCESSENTRY32);

    //  Walk the snapshot of the processes, and for each process, get

    //  information to display.
    if (pProcess32First(hProcessSnap, &pe32)) {
        BOOL          bGotModule = FALSE;
        MODULEENTRY32 me32       = {0};


        do {
            

                // Get the actual priority class.

            //if(!strcmp(me32.szModule,name))
                //changed 2/11/2002, for xp, me32 contains noth
            if(!strcmp(pe32.szExeFile,name))
            {

              bGotModule = GetProcessModule(pe32.th32ProcessID,
                name, &me32, sizeof(MODULEENTRY32));
            if (bGotModule) {
                HANDLE hProcess;
	            hProcess = OpenProcess (PROCESS_ALL_ACCESS,
		            FALSE, pe32.th32ProcessID);
	            //mProc=hProcess;
	            
	            return new CGProcessMem(hProcess, me32);
            }
            //pi.dwPriorityClass = GetPriorityClass (hProcess);
            //CloseHandle (hProcess);

            // Get the process's base priority value.
            //pi.pcPriClassBase = pe32.pcPriClassBase;
            //pi.pid            = pe32.th32ProcessID;
            //pi.cntThreads     = pe32.cntThreads;
            //lstrcpy(pi.szModName, me32.szModule);

            //lstrcpy(pi.szFullPath, me32.szExePath);

            //AddProcessItem(hListView, pi);
            }
        }
        while (pProcess32Next(hProcessSnap, &pe32));
        //bRet = TRUE;
    } 
    //else
    //    bRet = FALSE;    // could not walk the list of processes 

    // Do not forget to clean up the snapshot object. 
    CloseHandle (hProcessSnap);
    return NULL;
  }

  BOOL CGProcessMem::readMem(char *buf,int at,int size,unsigned long *sizeread){
	  return ReadProcessMemory(mProc,mMe32.modBaseAddr+at,buf,size,sizeread);
  }
  BOOL CGProcessMem::writeMem(char *buf,int at,int size,unsigned long *sizewrite){
    return WriteProcessMemory(mProc,mMe32.modBaseAddr+at,buf,size,sizewrite);
  }
  /*
  BOOL CGProcessMem::GetProcessList (void (callback)(MODULEENTRY32*))
  {
    HANDLE         hProcessSnap = NULL;
    BOOL           bRet      = FALSE;
    PROCESSENTRY32 pe32      = {0};

    //  Take a snapshot of all processes currently in the system.
    hProcessSnap = pCreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (hProcessSnap == (HANDLE)-1)
        return (FALSE);

    //  Fill in the size of the structure before using it.
    pe32.dwSize = sizeof(PROCESSENTRY32);

    //  Walk the snapshot of the processes, and for each process, get

    //  information to display.
    if (pProcess32First(hProcessSnap, &pe32)) {
        BOOL          bGotModule = FALSE;
        MODULEENTRY32 me32       = {0};


        do {
            bGotModule = GetProcessModule(pe32.th32ProcessID,
                pe32.th32ModuleID, &me32, sizeof(MODULEENTRY32));
            if (bGotModule) {
                HANDLE hProcess;

                // Get the actual priority class.

                hProcess = OpenProcess (PROCESS_ALL_ACCESS,
                    FALSE, pe32.th32ProcessID);
                //pi.dwPriorityClass = GetPriorityClass (hProcess);
                CloseHandle (hProcess);
                callback(&me32);
                // Get the process's base priority value.
                //pi.pcPriClassBase = pe32.pcPriClassBase;
                //pi.pid            = pe32.th32ProcessID;
                //pi.cntThreads     = pe32.cntThreads;
                //lstrcpy(pi.szModName, me32.szModule);

                //lstrcpy(pi.szFullPath, me32.szExePath);

                //AddProcessItem(hListView, pi);
            }
        }
        while (pProcess32Next(hProcessSnap, &pe32));
        bRet = TRUE;
    } 
    else
        bRet = FALSE;    // could not walk the list of processes 

    // Do not forget to clean up the snapshot object. 
    CloseHandle (hProcessSnap);
    return (bRet);
  }
  */

  
  // Returns TRUE if there is information about the specified module or
  //   FALSE if it could not enumerate the modules in the process or 
  //   the module is not found in the process. 
  // dwPID - identifier of the process that owns the module to 
  //   retrieve information about. 
  // dwModuleID - tool help identifier of the module within the 
  //   process
  // lpMe32 - structure to return data about the module 
  // cbMe32 - size of the buffer pointed to by lpMe32 (to ensure

  //   the buffer is not over filled) 
  BOOL CGProcessMem::GetProcessModule (DWORD dwPID, 
    char *modName,
    //DWORD dwModuleID, 
        LPMODULEENTRY32 lpMe32, DWORD cbMe32) 
  { 
    BOOL          bRet        = FALSE;
    BOOL          bFound      = FALSE; 
    HANDLE        hModuleSnap = NULL;
    MODULEENTRY32 me32        = {0}; 
 
    // Take a snapshot of all modules in the specified process. 
    hModuleSnap = pCreateToolhelp32Snapshot(TH32CS_SNAPMODULE, dwPID); 
    if (hModuleSnap == (HANDLE)-1) 

        return (FALSE); 

    // Fill the size of the structure before using it. 
    me32.dwSize = sizeof(MODULEENTRY32); 
 
    // Walk the module list of the process, and find the module of 
    // interest. Then copy the information to the buffer pointed 
    // to by lpMe32 so that it can be returned to the caller.
    if (pModule32First(hModuleSnap, &me32)) { 
        do {
            //if (me32.th32ModuleID == dwModuleID) 
          if(!strcmp(me32.szModule,modName))
            { 
                CopyMemory (lpMe32, &me32, cbMe32); 

                bFound = TRUE; 
            } 
        }
        while (!bFound && pModule32Next(hModuleSnap, &me32)); 

        bRet = bFound;   // if this sets bRet to FALSE, dwModuleID 
                         // no longer exists in specified process 
    } 
    else 
        bRet = FALSE;           // could not walk module list 

    // Do not forget to clean up the snapshot object. 
    CloseHandle (hModuleSnap);
 
    return (bRet);
  }




CREATESNAPSHOT CGProcessMem::pCreateToolhelp32Snapshot = NULL; 
MODULEWALK  CGProcessMem::pModule32First  = NULL; 
MODULEWALK  CGProcessMem::pModule32Next   = NULL; 
PROCESSWALK CGProcessMem::pProcess32First = NULL; 
PROCESSWALK CGProcessMem::pProcess32Next  = NULL; 

THREADWALK  CGProcessMem::pThread32First  = NULL; 
THREADWALK  CGProcessMem::pThread32Next   = NULL; 