/** Count Employees
 *  @file orgchart.cpp
 *  @author Bryce Jarboe | RedID 825033151
 *  @date 1/30/2024
 *  @brief Method definitions for OrgChart and EmployeeNode classes
*/
#include "orgchart.h"

void OrgChart::buildOrgChart(char* filename) {
    
    // Parse orgchart.txt file
    ifstream file(filename);
    if (file.is_open()) {
        string line;
        getline(file, line);

        // Obtain managerlevels from first line
        size_t pos = line.find("managerlevels=");
        if (pos != string::npos) {
            string valueStr = line.substr(pos + string("managerlevels=").length());
            this->managerlevels = stoi(valueStr);
        } else {
            cerr << "≡(▔﹏▔)≡    Could not find 'managerlevels' field" << endl << "See line 1 of " << filename << endl;
            exit(EXIT_FAILURE);
        }

        // Obtain sublevels
        getline(file, line);
        istringstream iss(line);
        int sb_lvl;
        while (iss >> sb_lvl) {
            this->sublevels.push_back(sb_lvl);
        }
        if (this->sublevels.size() != this->managerlevels) {
            cerr << "o((⊙﹏⊙))o.    Data mismatch:      managerlevels = " << this->managerlevels
                 << "   sublevels provided = " << this->sublevels.size() << endl;
        }

        // Obtain Employee ID cards (Parsed later)
        while (getline(file, line)) {
            this->toBeAdded.push_back(line);
        }
        


        file.close();
        int count = 0;
        for (string idStr : toBeAdded) {
            const char* employeeIDPath = idStr.c_str();
            if (addEmployees(employeeIDPath)) {
                count++;
            } else {
                cerr << "┌(。Д。)┐  Issue adding ID: " << idStr << endl;
            }
        }
        
    } else {
        cerr << "(っ °Д °;)っ   Could not read file: " << filename << endl;
        exit(EXIT_FAILURE);
    }

} // END: buildOrgChart


bool OrgChart::addEmployees(const char* employeeIDPath) {
    
    // Parse char* into simple integer array
    int arraySize = 0;
    for (int i = 0; employeeIDPath[i] != '\0'; i++) {
        if (employeeIDPath[i] != 'e' && employeeIDPath[i] != '_') {
            arraySize++;
        }
    }

    int eID[arraySize];
    int j = 0;
    for (int i = 0; employeeIDPath[i] != '\0'; i++) {
        char cid = employeeIDPath[i];
        if (cid != 'e' && cid != '_') {
            eID[j] = cid - '0';
            j++;
        }
    }

    if (arraySize == 0 || arraySize > this->sublevels.size()) {
        return false;
    }
    
    // Ensure root initialization
    if (this->root == nullptr) {
        this->root = new EmployeeNode*(new EmployeeNode(sublevels[0]));
        (*this->root)->employeeID = strdup("e"); // root always has ID "e"
    }

    EmployeeNode* current = *this->root;
    bool validPath = true;

    for (int i = 0; i < arraySize; i++) {
        int index = eID[i];

        if (index >= current->maxChildren) {
            validPath = false; // mark invalid paths
        }

        if (validPath && current->children[index] == nullptr) {
            // Initialize next level
            int next_lvl_size = (i + 1 < managerlevels) ? sublevels[i + 1] : 0;
            current->children[index] = new EmployeeNode(next_lvl_size);

            // Construct Partial ID
            int partialIDLength = 2 + 2 * i; // account for 'e' and '_' characters
            char* partialID = new char[partialIDLength + 1]; // +1 for null terminator
            // Copy current ID path
            for (int k = 0; k < partialIDLength; k++) {
                partialID[k] = employeeIDPath[k];
            }
            partialID[partialIDLength] = '\0'; // close id with terminator

            current->children[index]->employeeID = partialID;
            current->children[index]->node_level = i + 1;
        }

        if (validPath) {
            current = current->children[index];
        }
    }

    return validPath;

} // END: addEmployees




EmployeeNode* OrgChart::findEmployee(const char* employeeIDPath) {
    if (this->root == nullptr) return nullptr;
    if (strcmp((*root)->employeeID, employeeIDPath) == 0) return *root;

    // Updated parsing logic to correctly navigate through the tree
    EmployeeNode* current = *this->root;

    for (int i = 1; employeeIDPath[i] != '\0'; i++) {
        if (employeeIDPath[i] == '_') continue; // Skip underscores

        int index = employeeIDPath[i] - '0';
        if (index < 0 || index >= current->maxChildren || current->children[index] == nullptr) {
            return nullptr; // Invalid path or node does not exist
        }
        current = current->children[index];
    }

    return current;
} // END findEmployee


void OrgChart::numEmployees(int &count, EmployeeNode* employee) {
    if (employee == nullptr) {
        return; 
    }

    count++;    // Count every non-null node

    for (int i = 0; i < employee->maxChildren; i++) {
        numEmployees(count, employee->children[i]);
    }
}
void OrgChart::numEmployees(int &count, const char* idPath) {
    if (this->root == nullptr) {
        cerr << "(´･ω･`)?   Tree improperly initialized - No root found" << endl;
        return;
    }

    count = 0; // reset count
    EmployeeNode* startNode = findEmployee(idPath);
    if (startNode != nullptr) {
        numEmployees(count, startNode); // start recursion
    }
    cout << idPath << " " << count << endl;
}// END: numEmployees


void OrgChart::testTree(char* testFilename) {
    // Parse testfile.txt
    ifstream file(testFilename);
    if (file.is_open()) {
        string line;
        while (getline(file, line)) {
            // Run test on current ID path supplied
            int count;
            this->numEmployees(count, line.c_str());
        }
    } else {
        cerr << "(っ °Д °;)っ   Could not read file: " << testFilename << endl;
        exit(EXIT_FAILURE);    
    }
}