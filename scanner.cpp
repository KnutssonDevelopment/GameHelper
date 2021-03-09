#include "Headers/scanner.hpp"


void Scanner::printWindowList(){
   for(auto e: m_listWinHndls){
        std::cout << e.first << ".) " << e.second.second << std::endl;
    }
}

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
    
    m_listWinHndls.insert(std::make_pair(m_listWinHndls.size()+1, values));
    
    return true;
}

//Callback for EnumWindows-Method
BOOL Scanner::callbackEnumWindows(HWND hndl, LPARAM param){
    Scanner* pScanner = reinterpret_cast<Scanner*>(param);
    return pScanner->addWinHndlToList(hndl);
}

//Create Map with window handles, will be stored in m_listWinHndls
void Scanner::createWindowsList(){
    std::cout << "List of Windows" << std::endl;
    //Use Window Handles
    LPARAM para;
    EnumWindows(callbackEnumWindows, reinterpret_cast<LPARAM>(this));

}

std::vector<int> Scanner::FirstDataScan(VALUETYPE numberType, int number){
    switch(numberType){
        case BYTE_1: break;
        case BYTE_2: break;
        case BYTE_4: 
            InitialValueScan(1, (int)number);
            break;
        case BYTE_8: break;
        case FPN: break;
        case DFPN: break;
    }
}


template<typename UNIT>
std::vector<int> Scanner::InitialValueScan(int selectedProcess, UNIT n){
        
}

std::vector<int> Scanner::ReadRegion(int adr, int size, int value, int hdnl){
    
}