#include <iostream>
#include <conio.h> //Wait for key press
#include <windows.h>
#include <psapi.h>
#include <map>
#include <sstream>

//Information/SampleCode for process memery handeling (in C#)
//https://codingvision.net/c-how-to-scan-a-process-memory

//Map of window handle and window name
std::map<int, std::pair<HWND, std::string>> mapWinHndls;
int numOfPairs=0;

//Callback for EnumWindows-Method
BOOL callbackEnumWindows(HWND hndl, LPARAM param){
    //Use handle to get the name of the window
    int titleLength = GetWindowTextLength(hndl);
    if(!IsWindowVisible(hndl) || titleLength==0) return TRUE;
    //TCHAR mystring[titleLength+1];
    //TCHAR mystring[titleLength+1];
    LPSTR mystring = new char[titleLength+1];
    GetWindowTextA(hndl,mystring, titleLength+1);
    
    //Add pair to map
    std::string s=mystring;
    std::pair<HWND, std::string> values = std::make_pair(hndl, s);  
    mapWinHndls.insert(std::make_pair(++numOfPairs, values));
    
    return TRUE;
}

//Create Map with window handles
void createWindowsList(){
    std::cout << "List of Windows" << std::endl;
    //Use Window Handles
    LPARAM para;
    EnumWindows(callbackEnumWindows, para);

    for(auto e: mapWinHndls){
        std::cout << e.first << ".) " << e.second.second << std::endl;
    }
}

//
bool processStuff(int selectedWin){
    //Continue only if list has items
    if(mapWinHndls.size()==0)
        return false;

    //Window handle, used to get process id
    HWND hndlWindow = mapWinHndls[selectedWin].first;
    if(hndlWindow==NULL){
        std::cout << "No Window with that name found" << std::endl;

    }else{
        //get process id, used to access process
        DWORD procId;
        GetWindowThreadProcessId(hndlWindow, &procId);       
        
        HANDLE hndlProc;
        hndlProc = OpenProcess(PROCESS_VM_READ | PROCESS_QUERY_INFORMATION , true, procId);
        if(hndlWindow == NULL){
            std::cout << "Gettingg the handle failed."<< std::endl;
        }else{
            //Work within process

            //Contains information about the current computer system.(processor/page size/other) 
            //Used to determine first nad last used address in memory (lpMinimumApplicationAddress/lpMaximumApplicationAddress)
            LPSYSTEM_INFO sysinfo;
            GetSystemInfo(sysinfo);
            //Get used required infos
            DWORD pageSize = sysinfo->dwPageSize;
            LPVOID startAdr=sysinfo->lpMinimumApplicationAddress;
            LPVOID endAdr=sysinfo->lpMaximumApplicationAddress;

            //Info about the momry pages used by the process 
            PMEMORY_BASIC_INFORMATION lpBuffer = new MEMORY_BASIC_INFORMATION(); //information will be returned in there
            VirtualQueryEx(hndlProc, startAdr, lpBuffer, sizeof(lpBuffer));

            //After being done, close the handle
            CloseHandle(hndlProc);
        }
      
    }
    return true;
}

/*
BOOL ReadProcessMemory(
  HANDLE  hProcess,
  LPCVOID lpBaseAddress,
  LPVOID  lpBuffer,
  SIZE_T  nSize,
  SIZE_T  *lpNumberOfBytesRead
); */

int main(){
    std::cout << "MemScan - by Simon" << std::endl;
    //Create list of available windows
    createWindowsList();

    //Let user choose window to scan
    int select=0;
    while(select==0){
        std::cout << "Select Window-Handle! ";
        std::cin >> select;
        if(select>numOfPairs || select<=0){
            std::cout <<"NO valid widnow handle selected!" << std::endl;
            select=0;
        }
    }
    std::cout << std::endl; //Space
    std::cout << select << " was selected" <<std::endl;

    //Read what value to search

    //Search for the value

    //Read new value

    //Write the new value

    
    getch();
    return 0;
}

