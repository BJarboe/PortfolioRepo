/**
 * @author Bryce Jarboe | RedID 825033151
 * @date 11/27/2023
*/

#include "disassem.h"

using namespace std;
using namespace disassem;




namespace disassem {
    
    string removeLeadingZeros(const string& hexStr) {
        size_t nonZeroPos = hexStr.find_first_not_of('0');

        if (nonZeroPos != string::npos) {
            // Erase leading zeros
            return hexStr.substr(nonZeroPos);
        }

        // If the string is all zeros, return a string with a single zero
        return "0";
    }

    bool customSort(const std::string& a, const std::string& b) {
        return lexicographical_compare(a.begin(), a.end(), b.begin(), b.end());
    }
    
    string nextAddr( ifstream& objFile, string locStr ) {
        string line;
        streampos currentPos = objFile.tellg();
        getline(objFile, line);

        if (line.front() == 'T'){
            line = line.substr(3, 4); // next address
        }
        else {
            return locStr;
        }

        objFile.seekg(currentPos); // return file pointer to original position

        return line;
    }
    
    void caps(string& str){
        for ( char& c : str ) {
            c = toupper(c);
        }
    }


    int hexToInt( string hexStr ){
        if (hexStr.size() == 3 && hexStr.front() != 'F') {
            hexStr.insert(hexStr.begin(), '0');
        }
        
        int64_t decimalValue = stoll(hexStr, nullptr, 16);
        if (decimalValue > numeric_limits<int>::max() || decimalValue < numeric_limits<int>::min()) {
            throw out_of_range("Value out of range for int type.");
        }
        if (decimalValue & (1LL << (hexStr.size() * 4 - 1))) { 
            decimalValue -= 1LL << (hexStr.size() * 4); 
        }
        return static_cast<int>(decimalValue);
    }

    string intToHex( int num ){
        stringstream stream;
        stream << hex << setw(4) << setfill('0') << num;        
        string hexStr = stream.str();
        caps(hexStr);
        return hexStr;
    }


    void declareTables( map<string, string>& symTab, map<string, vector<string>>& litTab, string sim_lit ) {
        string temp;
        string addr;
        string symbol;
        string litName;
        string litConst;
        string lit_size; // parse into integer within main function
        

        // Open file with symbols and literals
        ifstream symFile( sim_lit );
        if ( getline( symFile, temp ) && temp.at(0) == 'S' ) {
            getline( symFile, temp ); // move file pointer
        }
        else {
            cerr << "Issue reading .sym" << endl;
            exit(1);
        }

        while (getline( symFile, temp ) && !temp.empty() ) {
            istringstream iss(temp);
            iss >> symbol >> addr;
            addr = addr.substr(2);
            symTab[addr] = symbol;
        }
        getline( symFile, temp );
        getline( symFile, temp );

        while ( getline( symFile, temp ) ) {
            istringstream iss(temp);
            if (temp.front() != ' ') { //"        =X'000007' 6      0002E3"
                iss >> litName >> litConst >> lit_size >> addr;
            }
            else{
                iss >> litConst >> lit_size >> addr;
                litName = "";
            }
            addr = addr.substr(2);
            litTab[addr] = {litName, litConst, lit_size};
        }
        symFile.close();
        
    }
    /*  Reference:
        Symbol  Address Flags:
        ----------------------
        FIRST   000000  R
        [--8---][---8---]
        Name    Lit_Const  Length Address:
        ----------------------------------
        VDEV    X'F1'      2      0002C9
        WDEV    X'000001'  6      0002D0
        [--8---][---11----]
    */
    
    string getOp( string hexStr ){
        
        char hexNum = hexStr.back();
        hexStr.pop_back();
        int hexValue;

        // Convert hex char to int
        if (isdigit(hexNum)) {
        hexValue = hexNum - '0';
        } else if (hexNum >= 'A' && hexNum <= 'F') {
            hexValue = hexNum - 'A' + 10;
        } else if (hexNum >= 'a' && hexNum <= 'f') {
            hexValue = hexNum - 'a' + 10;
        } else {
            cerr << "Error obtaining opcode: invalid hexadecimal character." << endl;
            exit(1);
        }

        // Round down to nearest multiple of 4
        hexValue -= (hexValue % 4);

        if (hexValue < 10) {
            hexNum = '0' + hexValue;
        }
        else {
            hexNum = 'A' + (hexValue - 10);
        }

        hexStr.push_back(hexNum);
        return hexStr;
    }
    
    string hexToBinary( string hexNum ) {
        string binNum;
        for ( char hexDigit : hexNum ) {
            switch ( hexDigit ) {
            case '0': binNum.append("0000"); break;
            case '1': binNum.append("0001"); break;
            case '2': binNum.append("0010"); break;
            case '3': binNum.append("0011"); break;
            case '4': binNum.append("0100"); break;
            case '5': binNum.append("0101"); break;
            case '6': binNum.append("0110"); break;
            case '7': binNum.append("0111"); break;
            case '8': binNum.append("1000"); break;
            case '9': binNum.append("1001"); break;
            case 'A': case 'a': binNum.append("1010"); break;
            case 'B': case 'b': binNum.append("1011"); break;
            case 'C': case 'c': binNum.append("1100"); break;
            case 'D': case 'd': binNum.append("1101"); break;
            case 'E': case 'e': binNum.append("1110"); break;
            case 'F': case 'f': binNum.append("1111"); break;
            
            default: 
                cerr << "Error: failed to convert Hex to Binary" << endl;
                exit(1);
            }
        }
        return binNum;
    }

    void declareMapping( map< string, pair<string, bool> >& instructionMapping ) {
        
        // Number of Instructions
        const int NUM_INSTR = 59;

        // Array of 2-byte machine code designations
        const static string ops[] = {
        "18", "58", "90", "40", "B4", "28",
        "88", "A0", "24", "64", "9C", "C4",
        "C0", "F4", "3C", "30", "34", "38",
        "48", "00", "68", "50", "70", "08",
        "6C", "74", "04", "D0", "20", "60",
        "98", "C8", "44", "D8", "AC", "4C",
        "A4", "A8", "F0", "EC", "0C", "78",
        "54", "80", "D4", "14", "7C", "E8",
        "84", "10", "1C", "5C", "94", "B0",
        "E0", "F8", "2C", "B8", "DC"
        };

        // Array of Instruction Names
        const static string mnemonics[] = {
        "ADD",    "ADDF",   "ADDR", "AND",  "CLEAR", "COMP",
        "COMPF",  "COMPR",  "DIV",  "DIVF", "DIVR",  "FIX",
        "FLOAT",  "HIO",    "J",    "JEQ",  "JGT",   "JLT",
        "JSUB",   "LDA",    "LDB",  "LDCH", "LDF",   "LDL",
        "LDS",    "LDT",    "LDX",  "LPS",  "MUL",   "MULF",
        "MULR",   "NORM",   "OR",   "RD",   "RMO",   "RSUB",
        "SHIFTL", "SHIFTR", "SIO",  "SSK",  "STA",   "STB",
        "STCH",   "STF",    "STI",  "STL",  "STS",   "STSW",
        "STT",    "STX",    "SUB",  "SUBF", "SUBR",  "SVC",
        "TD",     "TIO",    "TIX",  "TIXR", "WD"
        };

        // Array of booleans determining if the corresponding
        // machine code falls under format 2
        const static bool format2[] = {
            false,false,true, false,true, false,
            false,true, false,false,true, false,
            false,false,false,false,false,false,
            false,false,false,false,false,false,
            false,false,false,false,false,false,
            true, false,false,false,true, false,
            true, true, false,false,false,false,
            false,false,false,false,false,false,
            false,false,false,false,true, true,
            false,false,false,true, false
        };


        /*  Fill the map
            Use the machine code (2 digit hex) as keys
            Map the pairs to each key:
                the first contains the instruction name
                the second determines if it falls under format 2
        */
        for (int i = 0; i < NUM_INSTR; i++)
        {
            instructionMapping[ ops[i] ].first = mnemonics[i];
            instructionMapping[ ops[i] ].second = format2[i];
        }
        
        return;
    }

    string moveLoc( int disp, string locStr ) {
        // convert locStr to int
        int locCount = hexToInt(locStr);

        // add desired displacement
        locCount += disp;

        // convert back to string
        return intToHex(locCount);
        
    }





    void disassemble (  string data, ofstream& out_file, map< string, pair<string, bool> >& INSTR_MAP, 
                        string& locStr,  map<string, string>& symTab,     map<string, vector<string>>& litTab,
                        map<string, string>& TAcontents, string objLength, ifstream& objFile, bool& litorg_made ) {
        unsigned int i = 0;
        string opcode;
        string binInstr;
        string INSTR;
        char FORMAT;
        string OAT;
        string TAAM;
        string OBJ;
        string operand;
        string symbol;
        string litConst;
        int lit_size;
        static string BASE;
        int objL = data.size();
        
        // Read through data and generate instruction info
        //      Use a While loop since the indexing will vary per instruciton
        while ( i < data.size() ) {
            
            // Reset symbol to default value
            symbol = "";

            // Symbol check
            if (symTab.find(locStr) != symTab.end()) { // if symbol table is not empty at current location counter
                symbol = symTab[locStr];
            }

            // Literal check
            if (litTab.find(locStr) != litTab.end()) { // if symbol table is not empty at current location counter
                
                symbol = litTab[locStr].at(0);
                litConst = litTab[locStr].at(1);
                lit_size = stoi(litTab[locStr].at(2));
                OBJ = data.substr(i, lit_size);

                if (!symbol.empty()) {
                    out_file    << left << setw(5)  << locStr
                                << left << setw(8)  << symbol
                                << left << setw(8)  << "BYTE"
                                << left << setw(14) << litConst
                                << left << setw(13) << OBJ      << "\n";
                }
                else {
                    if (!litorg_made) {
                        out_file << "             LTORG" << endl;
                        litorg_made = true;
                    }
                    out_file    << left << setw(5)  << locStr
                                << left << setw(8)  << symbol
                                << left << setw(8)  << "*"
                                << left << setw(14) << litConst
                                << left << setw(13) << OBJ      << "\n";
                }

                TAcontents[locStr] = symbol;
                
                // Move location counter
                locStr = moveLoc( lit_size / 2, locStr );

                i += lit_size;
                continue;
            }

            // Write Location
            out_file    << left << setw(5) << locStr 
                        << left << setw(8) << symbol;

            // Extract OpCode
            opcode = getOp( data.substr(i, 2) );

            // Get instruction name
            INSTR = INSTR_MAP[opcode].first;

        /* Determine if instruction is of format 2
                Format 2:   [ op 8bits ] [r1 4bits] [r2 4bits]
                            i
        */

            // Format 2
            if (INSTR_MAP[opcode].second) {
                FORMAT = '2';

                // Extract 2 byte instruction in hex
                OBJ = data.substr(i, 4);

                // Determine Register to set operand
                switch ( OBJ.at(2) ) {
                    case '0':
                        operand = "A";
                        break;
                    case '1':
                        operand = "X";
                        break;
                    case '2':
                        operand = "L";
                        break;
                    case '3':
                        operand = "B";
                        break;
                    case '4':
                        operand = "S";
                        break;
                    case '5':
                        operand = "T";
                        break;
                    case '6':
                        operand = "F";
                        break;
                    case '8':
                        operand = "PC";
                        break;
                    case '9':
                        operand = "SW";
                        break;
                    default:
                        cerr << "Issue determining register contents for obj: " << OBJ << endl;
                        exit(1);
                }

                TAcontents[locStr] = operand;

                out_file    << left << setw(8)  << INSTR
                            << left << setw(14) << operand
                            << left << setw(13) << OBJ << "\n";

                // Continue to next instruction
                i += 4;

                // MODIFY LOCATION COUNTER
                locStr = moveLoc( 2, locStr );

                continue;
            }

        /* NOT Format 2
                Either Format 3 or Format 4

                Format 3:   [op 6] [n] [i] [x] [b] [p] [e] [disp 12]
                            i

                Format 4:   [op 6] [n] [i] [x] [b] [p] [e] [ address 20 ]
                            i

            Since both Formats are the same for the first 12 bits,
            we can unconditionally examine them first
        */ 

            // Convert first 3 digits of the hex instruction into binary
            OBJ = data.substr(i, 3);
            binInstr = hexToBinary(OBJ);

            // the binary instruction should now be in this format:
            //      [op 6] [n] [i] [x] [b] [p] [e]
            char n = binInstr.at(6);
            char iB = binInstr.at(7);
            char x = binInstr.at(8);
            char b = binInstr.at(9);
            char p = binInstr.at(10);
            char e = binInstr.back();

        // Determine Format 3 or 4 through [e] bit
        //              e = 0 or 1

            // FORMAT 3
            if (e == '0') {
                FORMAT = '3';
                // Reassign OBJ and binary s_to_int
                OBJ = data.substr(i, 6);
                binInstr = hexToBinary(OBJ);
            }

            // FORMAT 4
            else if (e == '1') {
                FORMAT = '4';

                // Reassign OBJ and binary s_to_int
                OBJ = data.substr(i, 8);
                binInstr = hexToBinary(OBJ);

                // Format 4 Instruction call: prepend '+'
                INSTR.insert(0, "+");
            }

            // INVALID FORMAT
            else {
                cerr << "Instruction has invalid format" << endl;
                exit(1);
            }

            string prevLoc = locStr;
            string TA;
            vector<string> symbolHandler;
            string nextRecordAddr;

            locStr = moveLoc( OBJ.size() / 2, locStr );
            if (i + OBJ.size() >= objL) {
                nextRecordAddr = nextAddr(objFile, locStr);
                int range_begin = hexToInt( locStr );
                int range_end = hexToInt(nextRecordAddr);
                int cmpAddr;
                for (const auto& sym : symTab) {
                    cmpAddr = hexToInt(sym.first);
                    if (range_begin <= cmpAddr && cmpAddr < range_end) {    // Symbol address falls in range
                        symbolHandler.push_back(sym.first);
                    }
                }
                for (const auto& lit : litTab) {
                    cmpAddr = hexToInt(lit.first);
                    if (range_begin <= cmpAddr && cmpAddr < range_end) {    // Symbol address falls in range
                        symbolHandler.push_back(lit.first);
                    }
                }
            }
            
        /*  Determine Operand Addressing Type - OAT
            simple:     [i] = [n]
            immediate:  [i] = 1     [n] = 0
            indirect:   [i] = 0     [n] = 1
        */
            if (iB == n) {
                OAT = ""; // simple
            }
            else { 
                switch (n) {
                    case '0':
                        OAT = "#";  //immediate
                        break;
                    case '1':
                        OAT = "@"; //indirect
                        break;
                    default:
                        cerr << "Error analyzing [n], check binInstr" << endl;
                        exit(1);
                }
            }

            // Prepend Addressing Type Symbol
            operand = OAT;

        /*  Determine Target-Address Addressing Mode
            absolute:   [b] = [p]      (this only works assuming valid input)
            base:       [b] = 1     [p] = 0
            PC:         [b] = 0     [p] = 1
        */
            if ( (b == p) || (FORMAT == '4') ) {
                TAAM = "absolute";
                if (FORMAT == '4'){
                    TA = OBJ.substr(4);
                }
                else{
                    TA = "0" + OBJ.substr(3);
                }
            }
            else {
                switch (p) {
                    case '0':
                        TAAM = "base";
                        TA = moveLoc( hexToInt(OBJ.substr(3)), BASE );
                        break;
                    case '1':
                        TAAM = "pc";
                        TA = moveLoc( hexToInt(OBJ.substr(3)), locStr );
                        break;
                    default:
                        cerr << "Error analyzing [p], check binInstr" << endl;
                        exit(1);
                        break;
                }
            }

            // Store Contents of TA in case of indirect calls later
            TAcontents[prevLoc] = TA;

            
            if (!OAT.empty() && OAT.at(0) == '#' && FORMAT == '3') {
                string opTemp = OBJ.substr(3);
                if (opTemp.front() == '0') {
                    opTemp = removeLeadingZeros(opTemp);
                }
                operand.append(opTemp);
            }

            // Check if operand has a corresponding symbol / literal
            else if (symTab.find(TA) != symTab.end()) {
                operand.append(symTab[TA]);
            }
            else if (litTab.find(TA) != litTab.end()) {
                if (!litTab[TA].at(0).empty()){
                    operand.append(litTab[TA].at(0));
                }
                else{
                    operand.append(litTab[TA].at(1));
                }
            }
            

            if (x == '1') {
                operand.append(",X");
            }

            out_file    << left << setw(8)  << INSTR
                        << left << setw(14) << operand
                        << left << setw(13) << OBJ << "\n";

            // Run separate check for LDB
            if (INSTR.compare("+LDB") == 0) {
                BASE = TA;
                out_file    << left << "             BASE    "
                            << left << operand.substr(1) << "\n";
            }

            if (!symbolHandler.empty()) {
                sort(symbolHandler.begin(),symbolHandler.end(),customSort);
                reverse(symbolHandler.begin(), symbolHandler.end());
                locStr = nextAddr(objFile, locStr);
            }
            while (!symbolHandler.empty()) {
                int disp;
                int pointA;
                int pointB;
                if (symbolHandler.size() > 1) {
                    pointA = hexToInt( *(symbolHandler.end() - 2) );
                    pointB = hexToInt(symbolHandler.back());
                    disp = pointA - pointB;
                }
                else {
                    disp = hexToInt(locStr) - hexToInt(symbolHandler.back());
                }

                // Write intermediary symbols to file
                out_file    << left << setw(5) << symbolHandler.back()
                            << left << setw(8) << symTab[symbolHandler.back()]
                            << left << setw(8) << "RESB"
                            << left << disp << "\n";

                symbolHandler.pop_back();
            }

            /*  Reference
                0007         LDA     @RETADR       0222BC      
                000A BADR    RESB    700                       
                02C6 RETADR  RESB    1  
                
                out_file    << left << setw(5) << locStr 
                            << left << setw(8) << symbol;
            */

            // increment index based on FORMAT
            i += OBJ.size();
        } // END WHILE LOOP

        /*  Reference: Column Format
        0000 FIRST   +LDB    #02C6         691002C6
        Widths
        5    8       8       14            13
        */

        return;
    }
}


int main( int argc, char* argv[] ) {
    
    // Declare map of machine code and corresponding instructions
    map< string, pair<string, bool> > instructionMapping;
    declareMapping( instructionMapping );

    // Obtain Symbols
    map<string, string> symTab;
    map<string, vector<string>> litTab;

    string sym_lit = argv[2];
    declareTables( symTab, litTab, sym_lit );
    
    // Validate Commandline argument
    if (argc != 3) {
        cerr << "Usage: " << argv[0] << " <obj_file_name.obj>" << endl;
        return 1;
    }

    // Open the obj file
    const string objFileName = argv[1];
    ifstream objFile(objFileName);

    // Verify the obj file was successfully opened
    if ( !objFile.is_open() ) {
        cerr << "Error: Unable to open obj file" << endl;
        return 1;
    }

    // Create Text file to write in
    ofstream outputFile("out.lst");

    // Verify the newly created file is opened
    if ( !outputFile.is_open() ) {
        cerr << "Error: Unable to open out.lst file" << endl;
        return 1;
    }

    // Parse Header
    string header;
    getline(objFile, header);
    if (header.at(0) != 'H') {
        cerr << "Issue parsing Header.  Check object file." << endl;
        exit(1);
    }
    if (!header.empty() && header.back() == '\r') {
        header.pop_back();
    }

    string progName = header.substr(1, 6);
    string startAddr = header.substr(10, 4);
    int progEnd = hexToInt(header.substr(13, 6)) + hexToInt(startAddr);

    // Add Header
    outputFile  << left << setw(5) << startAddr
                << left << setw(8) << progName
                << "START   0\n"; 
    
    string locStr;
    string line;
    string objLength;
    map<string, string> TAcontents;
    bool litorg_made = false;

    // Parse Text Records
    while (getline(objFile, line) && line.at(0) == 'T') {
        locStr = line.substr(3, 4);
        objLength = line.substr(7, 2);
        line = line.substr(9);
        disassemble( line, outputFile, instructionMapping, locStr, symTab, litTab, TAcontents, objLength, objFile, litorg_made );
    }

    // Handle End of Program
    int range_in = hexToInt(locStr);
    int cmpAddr;
    vector<string> endHandler;
    for (const auto& sym : symTab) {
        cmpAddr = hexToInt(sym.first);
        if (range_in <= cmpAddr && cmpAddr < progEnd) {   
            endHandler.push_back(sym.first);
        }
    }
    if (!endHandler.empty()) {
        sort(endHandler.begin(),endHandler.end(),customSort);
        reverse(endHandler.begin(), endHandler.end());
    }
    while (!endHandler.empty()) {
        int disp;
        int pointA;
        int pointB;
        if (endHandler.size() > 1) {
            pointA = hexToInt( *(endHandler.end() - 2) );
            pointB = hexToInt(endHandler.back());
            disp = pointA - pointB;
        }
        else {
            disp = progEnd - hexToInt(endHandler.back());
        }

        // Write intermediary symbols to file
        outputFile  << left << setw(5) << endHandler.back()
                    << left << setw(8) << symTab[endHandler.back()]
                    << left << setw(8) << "RESB"
                    << left << disp << "\n";

        endHandler.pop_back();
    }

    // Add Footer
    outputFile << "             END     " << progName << "\n";

    /*  Reference: Column Format
        0000 FIRST   +LDB    #02C6         691002C6
        Widths
        5    8       8       14
    */

    // Close files
    objFile.close();
    outputFile.close();

    return 0;
}