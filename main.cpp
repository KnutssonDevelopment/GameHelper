#include <iostream>
#include <conio.h> //Wait for key press
#include <windows.h>
#include <psapi.h>
#include <map>
#include <sstream>
#include <vector>
#include <cstring>
#include "Headers/scanner.hpp"




/*
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
*/

int main(){
    
    std::cout << "MemScan - by Simon" << std::endl;
    //Create list of available windows
    std::map<HWND, std::string> listOfWin = Scanner::createWindowsList();
    //Output the list
    std::cout << "Position\t" << "PID\t"<< "Name" << std::endl; 
    for(std::map<HWND, std::string>::iterator it=listOfWin.begin(); it != listOfWin.end(); ++it){
        std::cout << (std::distance(listOfWin.begin(), it) )<< ".)\t" << it->first << " \t" << it->second << std::endl;
    }

    
    //Let user choose window to scan
    int select=0;
    while(select==0){
        std::cout << "Select Window-Handle! ";
        std::cin >> select;
        if(select>listOfWin.size() || select<=0){
            std::cout <<"No valid widnow handle selected!" << std::endl;
            select=0;
        }
    }
    std::pair<HWND, std::string> hndlWindow;
    for(const std::pair<HWND, std::string> &elem : listOfWin){
        //Get window handle pair by counting down
        if(select==0){
            hndlWindow=elem;    
            break;   
        }
        select--;
    }
    std::cout << std::endl; //Space
    std::cout << hndlWindow.second << " was selected" <<std::endl;
    
    //Read what value to search
    int value=10;
    std::cout << "What number to choose? ";
    std::cin >> value;
        
    //Search for the value
    std::vector<int*> meineaddressen = Scanner::FirstDataScan(hndlWindow.first, Scanner::BYTE_4, value);
    //Print the list
    for(int* elem: meineaddressen){
        std::cout << elem << std::endl;
    }
    //Read new value

    //Write the new value
    
   

    getch();
    return 0;
}

