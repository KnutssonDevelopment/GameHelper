#include <iostream>
#include <conio.h> //Wait for key press
#include <windows.h>
#include <psapi.h>
#include <map>
#include <sstream>
#include <vector>

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
            SYSTEM_INFO sysinfo;
            GetSystemInfo(&sysinfo);
            //Get used required infos
            DWORD pageSize = sysinfo.dwPageSize;
            long * startAdr=(long*)sysinfo.lpMinimumApplicationAddress;
            long* endAdr=(long*)sysinfo.lpMaximumApplicationAddress;

            //List of addr where vlaue was found
            std::vector<LPVOID> addresses;
            //Here needs to start loop....
            while (startAdr < endAdr)
            {
                //Info about the momry pages used by the process
                MEMORY_BASIC_INFORMATION lpBuffer; //page info that is returned
                SIZE_T numOfBytesPage = VirtualQueryEx(hndlProc, startAdr, &lpBuffer, sizeof(lpBuffer));

                //Check if function failed
                if (numOfBytesPage == 0)
                {
                    std::cout << "Failed to get page informations." << std::endl;
                    exit(-1);
                }
                else
                {
                    LPVOID baseAddress = lpBuffer.BaseAddress;
                    SIZE_T regionSize = lpBuffer.RegionSize;
                    DWORD protection = lpBuffer.Protect; //access Protection
                    DWORD state = lpBuffer.State;        //State of page
                    //Check if rights to access data
                    if (protection == PAGE_READONLY && state == MEM_COMMIT)
                    {
                        //Get data 
                        DWORD* data = new DWORD[sizeof(regionSize)]; 
                        SIZE_T numOfBytesRead;
                        int readSuccess = ReadProcessMemory(hndlProc, baseAddress, data, sizeof(regionSize), &numOfBytesRead);
                        if(readSuccess!=0){
                            //search through data region for the number
                            for(int i=0;i<numOfBytesRead; ++i){
                                DWORD value = data[i];
                                
                                //if searched value, then add to list
                                if(value==10){
                                    addresses.push_back((LPVOID)data[i]);
                                    DWORD* data2 = new DWORD[sizeof(DWORD)];
                                    SIZE_T numOfBytesRead2;
                                    ReadProcessMemory(hndlProc, addresses.at(0), data2, sizeof(DWORD), &numOfBytesRead2);
                                }
                            }
                        }
                        free(data);
                    }

                    //Adjust starting address
                    startAdr += sizeof(regionSize);
                }
            }
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
    processStuff(select);

    //Read new value

    //Write the new value

    
    getch();
    return 0;
}

