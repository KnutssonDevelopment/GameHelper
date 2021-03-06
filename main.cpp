#include <iostream>
#include <conio.h> //Wait for key press
#include <windows.h>
#include <psapi.h>
#include <map>
#include <sstream>
#include <vector>
#include <cstring>

//Information/SampleCode for process memery handeling (in C#)
//https://codingvision.net/c-how-to-scan-a-process-memory

//Memory analysis cmd for debug console(VSC does not have memory view like VS)
//-exec x/1w *hex-address*
//Options: x/1w -> num means how many, letter means type (w=4byte, b=1byte)
//Thats why memcpy is used to interpret data in the desired data type

//Map of window handle and window name
std::map<int, std::pair<HWND, std::string>> mapWinHndls;
int numOfPairs=0; //could be removed

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

//Test reading ability of one address(will be modified and reused later)
void readOneValue(int selectedWin){
    int* adr = (int*)0x00747778;
    

    //Continue only if list has items
    if(mapWinHndls.size()==0)
        return;

    //Window handle, used to get process id
    HWND hndlWindow = mapWinHndls[selectedWin].first;
    if(hndlWindow==NULL){
        std::cout << "No Window with that name found" << std::endl;

    }else{
        //get process id, used to access process
        DWORD procId;
        GetWindowThreadProcessId(hndlWindow, &procId);       
        
        HANDLE hndlProc;
        hndlProc = OpenProcess(PROCESS_VM_READ | PROCESS_QUERY_INFORMATION , false, procId);
        if(hndlWindow == NULL){
            std::cout << "Gettingg the handle failed."<< std::endl;
        }else{
            //Work within process

            //Get data
            int *data = new int();
            memset(data, 0, sizeof(int));
            SIZE_T numOfBytesRead;
            int readSuccess = ReadProcessMemory(hndlProc, adr, data, sizeof(int), &numOfBytesRead);
            if (readSuccess != 0 && numOfBytesRead > 0)
            {
                int value; //*data;
                std::memcpy(&value, &data[0], sizeof(int)); 
                std::cout << "memcpy: " << value << std::endl;
                
            }
            delete[] data;

            //After being done, close the handle
            CloseHandle(hndlProc);
           
        }
      
    }
    
}

//Move ptr adr by bytes(needs rework to work universally)
void* incPtrByBytes(void* adr, unsigned long long numOfBytes){
    char* tmp = (char*)adr;
    tmp+=(sizeof(byte)*numOfBytes);
    if(tmp==(void*)0x14b160000)
    {
        std::cout << "istNULL" << std::endl;
    }
    return (void*)tmp;
}

//Search for a certain value
bool scanFirstIteration(int selectedWin){
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
        hndlProc = OpenProcess(PROCESS_VM_READ | PROCESS_QUERY_INFORMATION , false, procId);
        if(hndlWindow == NULL){
            std::cout << "Gettingg the handle failed."<< std::endl;
        }else{
            //Work within process

            //Contains information about the current computer system.(processor/page size/other) 
            //Used to determine first nad last used address in memory (lpMinimumApplicationAddress/lpMaximumApplicationAddress)
            SYSTEM_INFO sysinfo;
            GetSystemInfo(&sysinfo);
            //Get used required infos
            //DWORD pageSize = sysinfo.dwPageSize;
            void* startAdr=(void*)sysinfo.lpMinimumApplicationAddress;
            void* endAdr=(void*)sysinfo.lpMaximumApplicationAddress;

            std::cout <<"Anfang: " << startAdr << std::endl;
            std::cout <<"Ende: "<< endAdr << std::endl;

            //List of addr where vlaue was found
            std::vector<int*> addresses;
            MEMORY_BASIC_INFORMATION lpBuffer; //page info that is returned
            //Here needs to start loop....
            std::cout << "Start\t" << "Ende\t" <<"Alt. StartAdr"<<std::endl;
            while (startAdr < endAdr)
            {
              
                //Info about the momry pages used by the process
                SIZE_T numOfBytesPage = VirtualQueryEx(hndlProc, startAdr, &lpBuffer, sizeof(lpBuffer));
                
                std::cout  << startAdr << "\t" << incPtrByBytes(startAdr, lpBuffer.RegionSize) << "\t" << lpBuffer.BaseAddress << std::endl;
            

                //Check if function failed
                if (numOfBytesPage == 0)
                {
                    std::cout << "Failed to get page informations." << std::endl;
                    exit(-1);
                }
                else
                {
                    SIZE_T regionSize = lpBuffer.RegionSize;
                    DWORD protection = lpBuffer.Protect; //access Protection
                    DWORD state = lpBuffer.State;        //State of page
                    //Check if rights to access data
                    if (protection == PAGE_READWRITE && state == MEM_COMMIT)
                    {
                        //Get data 
                        byte* data= new byte[regionSize];
                        memset(data, 0, sizeof(byte)*regionSize);
                        SIZE_T numOfBytesRead;
                        int readSuccess = ReadProcessMemory(hndlProc, lpBuffer.BaseAddress, data, regionSize, &numOfBytesRead);
                        if(readSuccess!=0 && numOfBytesRead>=4){
                            //search through data region for the number
                            
                            for(int position=0;position<(numOfBytesRead-(numOfBytesRead%4)); ++position){
                                int value=*(int*)&data[position]; 
                                //std::memcpy(&value, &data[position], sizeof(int)); //Alternatively, but more overhead
                                
                                //if searched value, then add to list
                                if(value==10){
                                    addresses.push_back((int*)incPtrByBytes(startAdr, position)); 
                                                              
                                }
                            }
                            
                        }
                        delete[] data;
                    }

                    
                }

                //Adjust starting address
                startAdr = incPtrByBytes(startAdr,lpBuffer.RegionSize);//(void*)tmp;
 
            }
            //After being done, close the handle
            CloseHandle(hndlProc);

        }
      
    }
    return true;
}

template<typename T> void foo(T n){
    std::cout << typeof(n). << std::endl;
}


int main(){
    /*
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
    scanFirstIteration(select);
    //readOneValue(select);

    //Read new value

    //Write the new value
    */
   foo(2);
   foo((float)2);
   foo((char)0xC);
   foo("asd");
   std::string s("SiMON");
   foo((std::string)s);
   foo((long)2);
    

    getch();
    return 0;
}

