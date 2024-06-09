/** Count Employees
 *  @file orgchart.h
 *  @author Bryce Jarboe | RedID 825033151
 *  @date 1/30/2024
 *  @brief Header file for EmployeeNode and OrgChart classes
*/

#ifndef ORGCHART_H
#define ORGCHART_H

#include <vector>
#include <iostream>
#include <cstring>
#include <string.h>
#include <string>
#include <fstream>
#include <sstream>

using namespace std;


class EmployeeNode {
    public:
        EmployeeNode** children;
        int node_level;
        int maxChildren;
        const char* employeeID;

        EmployeeNode() {
            children = nullptr;
            node_level = -1;
        }
        EmployeeNode(int numChildren) {
            maxChildren = numChildren;
            children = new EmployeeNode*[numChildren];
            for (int i = 0; i < numChildren; i++) {
                children[i] = nullptr;
            }
        }
        ~EmployeeNode() {
            for (int i = 0; i < maxChildren; i++) {
                delete children[i];
            }
            delete[] children;
        }
};

class OrgChart {
    public:
        EmployeeNode** root;
        vector<int> sublevels;
        int managerlevels;
        vector<string> toBeAdded;


        OrgChart() {
            sublevels = {};
            managerlevels = 0;
            root = nullptr;
        }

        ~OrgChart() {
            if (root != nullptr) {
                delete *root;
                delete root;
            }
        }

        void buildOrgChart(char* filename);

        bool addEmployees(const char* employeeIDPath);

        EmployeeNode* findEmployee(const char* employeeIDPath);

        void numEmployees(int &count, const char* idPath);
        void numEmployees(int &count, EmployeeNode* employee);
        
        void testTree(char* testFilename);


};

#endif // ORGCHART_H