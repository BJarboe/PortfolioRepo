/**
 * @file disassem.cpp
 * @brief "Disassembles" hex machine code to assembly instruction.
 *
 * @author Bryce Jarboe     RedID 825033151
 * @date 10/30/2023
 */

#ifndef DISASSEM_H
#define DISASSEM_H

#include <map>
#include <iostream>
#include <string>
#include <fstream>
#include <vector>
#include <iomanip>
#include <sstream>
#include <cstdint>
#include <limits>
#include <cctype>
#include <algorithm>

using namespace std;

namespace disassem {

    string removeLeadingZeros(const std::string& hexStr);

    bool customSort(const std::string& a, const std::string& b);
    
    string nextAddr(ifstream& objFile, string locStr);
    
    void caps(string& str);

    int hexToInt( string hexStr );

    string intToHex( int num );

    void declareTables( map<string, string>& symTab, map<string, vector<string> >& litTab, string sym_lit );

    
    string getOp( string hexTemp );
    

    
    string hexToBinary( string hexNum );
    
    
    void declareMapping( map< string, pair<string, bool> >& instructionMapping );

    string moveLoc( int disp, string locStr );

    

    
    
    void disassemble(   string data, ofstream& out_file, map< string, pair<string, bool> >& INSTR_MAP, string& locStr, 
                        map<string, string>& symTab,     map<string, vector<string> >& litTab, 
                        map<string, string>& TAcontents, string objLength, ifstream& objFile, bool& litorg_made );
    
}

#endif // DISASSEM_H
