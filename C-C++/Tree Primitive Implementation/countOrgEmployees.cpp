/** Count Employees
 *  @file countOrgEmployees.cpp
 *  @author Bryce Jarboe | RedID 825033151
 *  @date 1/30/2024
 *  @brief main file for compilation and execution
*/

#include "orgchart.h"


using namespace std;


int main(int argc, char **argv) {
    if (argc != 3) {
        cerr << "(╬▔皿▔)╯   Usage: countEmployees <path_to_orgchart> <path_to_testfile>" << endl;
        exit(EXIT_FAILURE);
    }

    OrgChart tree;

    tree.buildOrgChart(argv[1]);

    int count = 0;
    tree.numEmployees(count, "e");

    tree.testTree(argv[2]);

    exit(EXIT_SUCCESS);
}
