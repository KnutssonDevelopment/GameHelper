#include <vector>
#include <windows.h>
#include <map>
#include <iostream>

//-lpsapi => needs to be added to build cmd

template<class UNIT>
class Scanner{
/*
Information/SampleCode for process memery handeling (in C#)
https://codingvision.net/c-how-to-scan-a-process-memory

Memory analysis cmd for debug console(VSC does not have memory view like VS)
-exec x/1w *hex-address*
Options: x/1w -> num means how many, letter means type (w=4byte, b=1byte)
Thats why memcpy could be used to interpret data in the desired data type
*/
    public:
        /*
        Used to determine what size the datatype has
        BYTE_X: used for whole numbers(inc. negative values)
        FPN: float(4bytes)
        DPFN: double(8bytes)
        */
        enum VALUETYPE{BYTE_1, BYTE_2, BYTE_4, BYTE_8, FPN, DFPN};

        //Singleton
        static Scanner& Instance();

        //Search in selected window for given value
        std::vector<int*> FirstDataScan(HWND procID, VALUETYPE numberType, UNIT number);
        //Further scan the vales from the address list, reference to aAddresses will be changed 
        void NextDataScan(HWND selectedWinHndl, VALUETYPE numberType, UNIT number, std::vector<int*> &aAddresses);
        //Create map with window handles and name
        static std::map<HWND, std::string> createWindowsList();
        

    private:
        //Variables
        std::map<HWND, std::string> m_listWinHndls; //Stores window handles

        //Functions
        //Singleton safety measures
        Scanner(){};
        Scanner(const Scanner&)=delete;

        //Functions regarding scanning/reading memory
        std::vector<int *> InitialValueScan(HWND selectedWindowHndl, UNIT pValue);
        //Search list of addresses if value is equals to new value
        bool RescanAddressList(HWND selectedWinHndl, UNIT number, std::vector<int *> &aAddresses);
        void ReadAddressRange(const void *pAddr, SIZE_T pSize, UNIT pValue, HANDLE pHdnl, std::vector<int *> &pAddresses);
        
        //Helper functions
        static void *incPtrByBytes(const void *adr, unsigned long long numOfBytes); //HelperFnct
     

        //Used to get some kind of process/window list
        static BOOL callbackEnumWindows(HWND hndl, LPARAM param);
        bool addWinHndlToList(HWND hndl);
};

template<typename UNIT>
Scanner<UNIT>& Scanner<UNIT>::Instance(){
    static Scanner impl;
    return impl;
}

template<typename UNIT>
bool Scanner<UNIT>::addWinHndlToList(HWND hndl){
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
template<typename UNIT>
BOOL Scanner<UNIT>::callbackEnumWindows(HWND hndl, LPARAM param){
    Scanner* pScanner = reinterpret_cast<Scanner*>(param);
    return pScanner->addWinHndlToList(hndl);
}

//Create Map with window handles, will be stored in m_listWinHndls
template<typename UNIT>
std::map<HWND, std::string> Scanner<UNIT>::createWindowsList(){
    Scanner& scannerRef= Scanner::Instance();
    //Use Window Handles
    EnumWindows(callbackEnumWindows, reinterpret_cast<LPARAM>(&scannerRef));
    return scannerRef.m_listWinHndls;
}

//Move ptr adr by bytes(needs rework to work universally)
template<typename UNIT>
void* Scanner<UNIT>::incPtrByBytes(const void* adr, unsigned long long numOfBytes){
    char* tmp = (char*)adr;
    tmp+=(sizeof(char)*numOfBytes);
    return (void*)tmp;
}

template<typename UNIT>
std::vector<int*> Scanner<UNIT>::FirstDataScan(HWND procID, VALUETYPE numberType, UNIT number){

    switch (numberType)
    {
    case BYTE_1:
        return InitialValueScan(procID, number);
        break;
    case BYTE_2:
        return InitialValueScan(procID, number);
        break;
    case BYTE_4:
        return InitialValueScan(procID, number);
        break;
    case BYTE_8:
        return InitialValueScan(procID, number);
        break;
    case FPN:
        return InitialValueScan(procID, number);
        break;
    case DFPN:
        return InitialValueScan(procID, number);
        break;

    default:
        return std::vector<int *>();
    }
}

template<typename UNIT>
std::vector<int*> Scanner<UNIT>::InitialValueScan(HWND selectedWindowHndl, UNIT pValue){
    
    //List of addr where pValue was found
    std::vector<int *> addresses;

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

template<typename UNIT>
void Scanner<UNIT>::ReadAddressRange(const void* pAddr, SIZE_T pSize, UNIT pValue, HANDLE pHdnl, std::vector<int*> &pAddresses)
{
    short varSize=sizeof(pValue);
    //Get data
    char *data = new char[pSize];
    memset(data, 0, sizeof(char) * pSize);
    SIZE_T numOfBytesRead;
    int readSuccess = ReadProcessMemory(pHdnl, pAddr, data, pSize, &numOfBytesRead);
    if (readSuccess != 0 && numOfBytesRead >= varSize)
    {
        //search through data region for the number

        for (int position = 0; position < (numOfBytesRead - (numOfBytesRead % varSize)); ++position)
        {
            UNIT value = *(UNIT*)&data[position];
            //std::memcpy(&value, &data[position], sizeof(int)); //Alternatively, but more overhead

            //if searched value, then add to list
            if (value == pValue)
            {
                //incPtrByBytes is used to get the address from the found value
                pAddresses.push_back((int *)incPtrByBytes(pAddr, position));
            }
        }
    }
    delete[] data;
}

template<typename UNIT>
void Scanner<UNIT>::NextDataScan(HWND selectedWinHndl, VALUETYPE numberType, UNIT number, std::vector<int*> &aAddresses){
     switch (numberType)
     {
     case BYTE_1:
         RescanAddressList(selectedWinHndl, number, aAddresses);
         break;
     case BYTE_2:
        RescanAddressList(selectedWinHndl, number, aAddresses);
         break;
     case BYTE_4:
         RescanAddressList(selectedWinHndl,number, aAddresses);
         break;
     case BYTE_8:
         RescanAddressList(selectedWinHndl,number, aAddresses);
         break;
     case FPN:
         RescanAddressList(selectedWinHndl, number, aAddresses);
         break;
     case DFPN:
         RescanAddressList(selectedWinHndl,number, aAddresses);
         break;
     }
 }

template<typename UNIT>
bool Scanner<UNIT>::RescanAddressList(HWND selectedWinHndl, UNIT aNumber, std::vector<int*> &aAddresses){

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
                //Read a given address in memory
                ReadAddressRange(currentAddress, sizeof(UNIT), aNumber, hndlProc, tmpList);

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