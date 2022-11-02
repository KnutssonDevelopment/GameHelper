#include <iostream>
#include <conio.h> //Wait for key press
#include <windows.h>
#include <psapi.h>
#include <map>
#include <sstream>
#include <vector>
#include <cstring>
#include "headers/scanner.h"


int main() {

    Scanner<int>& myScanner = Scanner<int>::Instance();
    std::cout << "MemScan - by Simon" << std::endl;
    //Create list of available windows
    std::map<HWND, std::string> listOfWin = myScanner.createWindowsList();
    //Output the list
    std::cout << "Position\t" << "PID\t" << "Name" << std::endl;
    for (std::map<HWND, std::string>::iterator it = listOfWin.begin(); it != listOfWin.end(); ++it) {
        std::cout << (std::distance(listOfWin.begin(), it)) << ".)\t" << it->first << " \t" << it->second << std::endl;
    }


    //Let user choose window to scan
    int select = 0;
    while (select == 0) {
        std::cout << "Select Window-Handle! ";
        std::cin >> select;
        if (select > listOfWin.size() || select <= 0) {
            std::cout << "No valid widnow handle selected!" << std::endl;
            select = 0;
        }
    }
    std::pair<HWND, std::string> hndlWindow;
    for (const std::pair<HWND, std::string>& elem : listOfWin) {
        //Get window handle pair by counting down
        if (select == 0) {
            hndlWindow = elem;
            break;
        }
        select--;
    }
    std::cout << std::endl; //Space
    std::cout << hndlWindow.second << " was selected" << std::endl;

    //Read what value to search
    int value = 10;
    std::cout << "What number to choose? \n";
    std::cin >> value;

    //Search for the value
    std::vector<int*> meineaddressen = myScanner.FirstDataScan(hndlWindow.first, Scanner<int>::BYTE_4, value);
    //Print the list
    for (int* elem : meineaddressen) {
        std::cout << elem << std::endl;
    }
    //Read new value
    value = 9;
    std::cout << "What number to choose? \n";
    std::cin >> value;

    //Rescan new found addresses
    myScanner.NextDataScan(hndlWindow.first, Scanner<int>::BYTE_4, value, meineaddressen);
    //Print the list
    for (int* elem : meineaddressen) {
        std::cout << elem << std::endl;
    }

    while (meineaddressen.size() > 1)
    {

        //Read new value
        value = 9;
        std::cout << "What number to choose? \n";
        std::cin >> value;

        //Rescan new found addresses
        myScanner.NextDataScan(hndlWindow.first, Scanner<int>::BYTE_4, value, meineaddressen);
        //Print the list
        for (int* elem : meineaddressen)
        {
            std::cout << elem << std::endl;
        }
    }

    //Write the new value
    //Read new value
    value = 0;
    std::cout << "What number to change to? \n";
    std::cin >> value;
    myScanner.WriteToProcessMemory(hndlWindow.first, meineaddressen.at(0), value);

    std::cin.get();
    return 0;
}

