#include <vector>
#include <windows.h>
#include <map>
#include <iostream>

class Scanner{
    public:
        /*
        Used to determine what size the datatype has
        BYTE_X: used for whole numbers(inc. negative values)
        FPN: float(4bytes)
        DPFN: double(8bytes)
        */
        enum VALUETYPE{BYTE_1, BYTE_2, BYTE_4, BYTE_8, FPN, DFPN};

        //Search in selected window for given value
        static std::vector<int> FirstDataScan(VALUETYPE numberType,int number);
        //Create map with window handles and name
        void createWindowsList();
        void printWindowList();

    private:
        //Variables
        std::map<int, std::pair<HWND, std::string>> m_listWinHndls;

        //Functions
        template<typename UNIT>
        static std::vector<int> InitialValueScan(int selectedProcess, UNIT n);
        static std::vector<int> ReadRegion(int adr, int size, int value, int hdnl);

        static BOOL callbackEnumWindows(HWND hndl, LPARAM param);
        bool addWinHndlToList(HWND hndl);
};