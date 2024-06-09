/** @name Scheduler Header
 *  @file schedule.h
 *  @author Bryce Jarboe | RedID 825033151
 *  @date 3/7/2024
 *  @brief Emulate process scheduling with CPU and IO bursts with different strategies
*/

#ifndef SCHEDULE_H
#define SCHEDULE_H

#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include <unistd.h>
#include <vector>
#include <iostream>
#include <fstream>
#include <sstream>
#include <deque>
#include <set>
#include <algorithm>

using namespace std;

enum bt {CPU, IO}; // burst type

struct Burst {
    int id;
    int duration;
    bt burst_type; 
};

struct completionLog {
    int id;
    int completionTime;
    int totalWaitTime;
    // completionLog(int i, int c, int t) : id(i), completionTime(c), totalWaitTime(t) {}
};

vector<deque<Burst>>parse_process(char* filename);

void parse_flag(int argc, char *argv[], string& strategy, int& quantum);
void usage_error();
void fail();
void success();
void sortQ(deque<deque<Burst>>& blockedQ);
void round_robin(int quantum, vector<deque<Burst>> processes);
void fcfs(vector<deque<Burst>> processses);

int main(int argc, char *argv[]) {
    if (argc < 2 || argc > 6 || argc % 2 == 1) {
        // Invalid arguments provided
        usage_error();
    }

    // Parse processes into 2D vector
    vector<deque<Burst>> processes = parse_process(argv[1]);
    
    // Set default strategy and quantum
    string strategy = "fcfs";
    int quantum = 2;

    // Parse any added flags to obtain scheduling strategy and time quantum
    if (argc > 2) {
        parse_flag(argc, argv, strategy, quantum);
    }
    
    // Execute burst procedure based on scheduling strategy
    if (strategy.compare("rr") == 0) {
        round_robin(quantum, processes);
    } else {
        fcfs(processes);
    }

    success();
}


#endif