#include "Headers/scanner.hpp"

//Create Map with window handles
void Scanner::createWindowsList(std::map<int, std::pair<HWND, std::string>>* listWinHndls){
    std::cout << "List of Windows" << std::endl;
    //Use Window Handles
    LPARAM para;
    EnumWindows(callbackEnumWindows, para);

    for(auto e: *listWinHndls){
        std::cout << e.first << ".) " << e.second.second << std::endl;
    }
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