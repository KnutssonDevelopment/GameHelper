#include "Headers/scanner.hpp"

Scanner& Scanner::Instance(){
    static Scanner impl;
    return impl;
}
/*
void Scanner::printWindowList(){
   for(auto e: m_listWinHndls){
        std::cout << e.first << ".) " << e.second<< std::endl;
    }
}*/

bool Scanner::addWinHndlToList(HWND hndl){
    //Use handle to get the name of the window
    int titleLength = GetWindowTextLength(hndl);
    if(!IsWindowVisible(hndl) || titleLength==0) return true;
    //TCHAR mystring[titleLength+1];
    //TCHAR mystring[titleLength+1];
    LPSTR mystring = new char[titleLength+1];
    GetWindowTextA(hndl,mystring, titleLength+1);
    
    //Add pair to map
    std::string s=mystring;
    std::pair<HWND, std::string> values = std::make_pair(hndl, s);
    
    m_listWinHndls.insert(values);
    
    return true;
}

//Callback for EnumWindows-Method
BOOL Scanner::callbackEnumWindows(HWND hndl, LPARAM param){
    Scanner* pScanner = reinterpret_cast<Scanner*>(param);
    return pScanner->addWinHndlToList(hndl);
}

//Create Map with window handles, will be stored in m_listWinHndls
std::map<HWND, std::string> Scanner::createWindowsList(){
    Scanner& scannerRef= Scanner::Instance();
    //Use Window Handles
    EnumWindows(callbackEnumWindows, reinterpret_cast<LPARAM>(&scannerRef));
    return scannerRef.m_listWinHndls;
}

std::vector<int*> Scanner::FirstDataScan(HWND procID, VALUETYPE numberType, int number){
    /*
    std::vector<int> tmp;
    switch(numberType){
        case BYTE_1: return tmp;break;
        case BYTE_2: return tmp;break;
        case BYTE_4: 
            return InitialValueScan(procID, (int)number);
            break;
        case BYTE_8: return tmp;break;
        case FPN: return tmp;break;
        case DFPN: return tmp;break;
    }*/
    return InitialValueScan<int>(procID, (int)number);
}


template<typename UNIT>
std::vector<int*> Scanner::InitialValueScan(HWND selectedWindowHndl, int pValue){
    
    //List of addr where pValue was found
    std::vector<int*> addresses;

    //Continue only if window is active(savety check)
    //if(mapWinHndls.size()==0)
     //   return false;

    if(selectedWindowHndl==NULL){
        std::cout << "No Window with that name found" << std::endl;

    }else{
        //get process id, used to access process
        DWORD procId;
        GetWindowThreadProcessId(selectedWindowHndl, &procId);       
        
        HANDLE hndlProc;
        hndlProc = OpenProcess(PROCESS_VM_READ | PROCESS_QUERY_INFORMATION , false, procId);
        if(hndlProc == NULL){
            std::cout << "Getting the handle failed."<< std::endl;
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

            //DEBUG
            //std::cout <<"Anfang: " << startAdr << std::endl;
           // std::cout <<"Ende: "<< endAdr << std::endl;

            MEMORY_BASIC_INFORMATION lpBuffer; //page info that is returned
           
            //DEBUG
            //std::cout << "Start\t" << "Ende\t" <<"Alt. StartAdr"<<std::endl;
            while (startAdr < endAdr)
            {
              
                //Info about the momry pages used by the process
                SIZE_T numOfBytesPage = VirtualQueryEx(hndlProc, startAdr, &lpBuffer, sizeof(lpBuffer));
                
                //DEBUG
                //std::cout  << startAdr << "\t" << incPtrByBytes(startAdr, lpBuffer.RegionSize) << "\t" << lpBuffer.BaseAddress << std::endl;
            

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
                        //Read a give region of memory
                        ReadAddressRange(startAdr, regionSize, pValue, hndlProc, addresses);
                        
                    }

                    
                }

                //Adjust starting address
                startAdr = incPtrByBytes(startAdr,lpBuffer.RegionSize);//(void*)tmp;
 
            }
            //After being done, close the handle
            CloseHandle(hndlProc);

        }
      
    }
    return addresses;
}

//Move ptr adr by bytes(needs rework to work universally)
void* Scanner::incPtrByBytes(const void* adr, unsigned long long numOfBytes){
    char* tmp = (char*)adr;
    tmp+=(sizeof(byte)*numOfBytes);
    return (void*)tmp;
}

void Scanner::ReadAddressRange(const void* pAddr, SIZE_T pSize, int pValue, HANDLE pHdnl, std::vector<int*> &pAddresses)
{
    //Get data
    byte *data = new byte[pSize];
    memset(data, 0, sizeof(byte) * pSize);
    SIZE_T numOfBytesRead;
    int readSuccess = ReadProcessMemory(pHdnl, pAddr, data, pSize, &numOfBytesRead);
    if (readSuccess != 0 && numOfBytesRead >= 4)
    {
        //search through data region for the number

        for (int position = 0; position < (numOfBytesRead - (numOfBytesRead % 4)); ++position)
        {
            int value = *(int *)&data[position];
            //std::memcpy(&value, &data[position], sizeof(int)); //Alternatively, but more overhead

            //if searched value, then add to list
            if (value == pValue)
            {
                pAddresses.push_back((int *)incPtrByBytes(pAddr, position));
            }
        }
    }
    delete[] data;
}

bool Scanner::RescanAddressList(HWND selectedWinHndl, VALUETYPE numberType, int aNumber, std::vector<int*> &aAddresses){

    //Continue only if list has items(addresses)
    if(aAddresses.size()==0)
        return false;

    if(selectedWinHndl==NULL){
        std::cout << "No Window with that name found" << std::endl;

    }else{
        //get process id, used to access process
        DWORD procId;
        GetWindowThreadProcessId(selectedWinHndl, &procId);       
        
        HANDLE hndlProc;
        hndlProc = OpenProcess(PROCESS_VM_READ | PROCESS_QUERY_INFORMATION , false, procId);
        if(hndlProc == NULL){
            std::cout << "Getting the handle failed."<< std::endl;
        }else{
            //Work within process

            //Contains information about the current computer system.(processor/page size/other) 
            //Used to determine first nad last used address in memory (lpMinimumApplicationAddress/lpMaximumApplicationAddress)
            SYSTEM_INFO sysinfo;
            GetSystemInfo(&sysinfo);
        
            void* currentAddress;//=(void*)aAddresses.at(0);            
     
            int listIdx=0;
            do
            {
                //Adjust starting address  
                currentAddress = (void*)aAddresses.at(listIdx);

                //Temp storage for value
                std::vector<int*> tmpList;
                //Read a give address in memory
                ReadAddressRange(currentAddress, sizeof(int), aNumber, hndlProc, tmpList);

                //Check return
                if(tmpList.size()==0){
                    //address was not put in List due to value not being the new value
                    //meaning the address does not store the search address which stores the our vlaue
                    //removing item from list, keeping listindex at same position
                    aAddresses.erase(aAddresses.begin()+listIdx);
                }else{
                    //move index to next item
                    listIdx++;
                }

                
               // std::cout << listIdx << std::endl;
            }while(listIdx < aAddresses.size());
            //After being done, close the handle
            CloseHandle(hndlProc);

        }
      
    }
    //true if list was succesfully updated
    return true;
}
