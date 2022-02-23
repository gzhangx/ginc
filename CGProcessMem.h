#ifndef GANGGETPROCESSMEMHEADERFILE
#define GANGGETPROCESSMEMHEADERFILE
#include <windows.h>
#include <tlhelp32.h> // needed for tool help declarations


// Type definitions for pointers to call tool help functions. 
typedef BOOL (WINAPI *MODULEWALK)(HANDLE hSnapshot, 
    LPMODULEENTRY32 lpme);
typedef BOOL (WINAPI *THREADWALK)(HANDLE hSnapshot, 
    LPTHREADENTRY32 lpte); 
typedef BOOL (WINAPI *PROCESSWALK)(HANDLE hSnapshot,
    LPPROCESSENTRY32 lppe); 
typedef HANDLE (WINAPI *CREATESNAPSHOT)(DWORD dwFlags, 
    DWORD th32ProcessID); 
 
//to use: first create a new instance of cgprocessMem, overwrite checkMem.
//call
class CGProcessMem{
private :
  static CREATESNAPSHOT pCreateToolhelp32Snapshot; 
  static MODULEWALK  pModule32First  ; 
  static MODULEWALK  pModule32Next   ; 
  static PROCESSWALK pProcess32First ; 
  static PROCESSWALK pProcess32Next  ; 

  static THREADWALK  pThread32First  ; 
  static THREADWALK  pThread32Next   ; 

  //
  //void close();
  static BOOL InitToolhelp32 (void) ;
  static BOOL GetProcessModule (HANDLE hProcess,DWORD dwPID);
public:
  CGProcessMem(){}

  //DWORD   modBaseSize;
  //MODULEENTRY32 *getModule(){return &mMe32;}
  
  
  

  
  
  

  
  //static BOOL GetProcessList (void (callback)(MODULEENTRY32*));
  static BOOL getProcessByName(char *name, CGProcessMem *me);
    
  protected:
    
    virtual void checkMemory()=0;
    static CGProcessMem * gzThis;
    HANDLE mProc;
	  MODULEENTRY32 *mMe32;
    unsigned long getModuleSize(){return mMe32->modBaseSize;}
    BOOL readMem(char *buf,int at,int size,unsigned long *sizeread);
    BOOL writeMem(char *buf,int at,int size,unsigned long *sizewrite);
  

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
  
    //02/10/2002  changed to use mod name to id module
    //DWORD dwModuleID ;

};


#endif
