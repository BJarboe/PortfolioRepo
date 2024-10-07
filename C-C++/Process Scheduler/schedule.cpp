/** @name Scheduler
 *  @file schedule.cpp
 *  @author Bryce Jarboe | RedID 825033151
 *  @date 3/7/2024
 *  @brief main file to emulate process scheduling with CPU and IO bursts with different strategies
*/
#include "schedule.h"
#include "log.h"

using namespace std;

void fail() {
    cerr << R"(
         ______
      .-"      "-.        ###########       #########         #########         ###########      #########
     /            \       ###########       ###########       ###########       ###########      ###########
    |              |      ###               ###     ###       ###     ###       ###     ###      ###     ###
    |,  .-.  .-.  ,|      ###               ###     ###       ###     ###       ###     ###      ###     ###
    | )(__/  \__)( |      ###               ###     ###       ###     ###       ###     ###      ###     ###
    |/     /\     \|      ##########        #########         #########         ###     ###      #########
    (_     ^^     _)      ###               ###     ###       ###     ###       ###     ###      ###     ###
     \__|IIIIII|__/       ###               ###     ###       ###     ###       ###     ###      ###     ###
      | \IIIIII/ |        ###               ###     ###       ###     ###       ###     ###      ###     ###
      \          /        ###########       ###     ###       ###     ###       ###########      ###     ###
       `--------`         ###########       ###     ###       ###     ###       ###########      ###     ###
    )" << endl;
    exit(EXIT_FAILURE);
}

void success() { // 
    cerr << R"(
        
 yay                       ##########    ###     ###    ###########    ###########    ###########     ##########     ##########
   \                      ###########    ###     ###    ###########    ###########    ###########    ###########    ###########
    \  /\      /\         ###            ###     ###    ###            ###            ###            ###            ###
      /  \____/  \        ###            ###     ###    ###            ###            ###            ###            ###
     /            \       ###            ###     ###    ###            ###            ###            ###            ###
    /              \      ##########     ###     ###    ###            ###            ###########    ##########     ##########
    \   >      <   /       ##########    ###     ###    ###            ###            ###             ##########     ##########
     \    \/\/    /               ###    ###     ###    ###            ###            ###                    ###            ###
      \          /                ###    ###     ###    ###            ###            ###                    ###            ###
  /\   ` ------ `   /\    ###########    ###########    ###########    ###########    ###########    ###########    ###########
 /  \              /  \   ##########     ###########    ###########    ###########    ###########    ##########     ##########
    )" << endl;
    exit(EXIT_SUCCESS);
}

void usage_error() {
    cerr    << "\n≡(▔﹏▔)≡  Usage: ./schedule [Burst file]" << endl
            << "\t  Options:" << endl
            << "\t\t-s [Scheduling strategy]" << endl
            << "\t\t\tfcfs - First Come First Served," << endl
            << "\t\t\trr - Round Robin." << endl
            << "\t\t-q [Time Quantum]" << endl
            << "\t\t\tMust be larger than 0" << endl << endl;
    fail();
}


vector<deque<Burst>> parse_process(char* filename) {
    vector<deque<Burst>> processes;
    fstream burst_stream;
    burst_stream.open(filename);

    // Ensure file opened successfully
    if (!burst_stream.is_open()) {
        cerr << endl << "┌(。Д。)┐\nUnable to open " << filename << endl
                     << "Verify file name and directory contents" << endl;
        fail();
    }

    string line;
    for (int i = 0; getline(burst_stream, line); i++) {
        deque<Burst> process;
        stringstream ss(line);
        int val; // Temporary value to extract burst duration
        for (int j = 0; ss >> val; j++) {
            if (val <= 0) {
                cerr << "\n(╯▔皿▔)╯\nA burst number must be bigger than 0\n";
                fail();
            }
            Burst burst;
            burst.id = i;
            burst.burst_type = (j % 2 == 1) ? IO : CPU; // odd j -> IO burst
            burst.duration = val;
            
            if (ss.peek() == ',' || ss.peek() == ' ') {
                ss.ignore();
            }
            process.push_back(burst);
        }
        // Ensure process ends with CPU burst
        if (process.size() % 2 == 0) {
            cerr << endl << "(ಥ _ ಥ)\nThere must be an odd number of bursts for each process" << endl;
            fail();
        }
        processes.push_back(process);
        cout << line << endl;
    }

    burst_stream.close();

    // Ensure processes were parsed
    if (processes.empty()) {
        cerr << "o((⊙﹏⊙))o. No bursts found.  Verify " << filename << "'s contents." << endl;
        fail();
    }
    
    return processes;
}


void sortQ(deque<deque<Burst>>& blockedQ) {
    vector<deque<Burst>> tempVec(blockedQ.begin(), blockedQ.end());
    std::stable_sort(tempVec.begin(), tempVec.end(), 
        [](const std::deque<Burst>& a, const std::deque<Burst>& b) -> bool {
            if (!a.empty() && !b.empty()) {
                return a.front().duration < b.front().duration;
            } else if (a.empty()) {
                cerr << "\t＞︿＜     Compared burst deque at " << &a <<" is empty\n";
                return true;     
            } else {
                cerr << "\t＞︿＜     Compared burst deque at " << &b <<" is empty\n";
                return false;
            }
        }
    );
    copy(tempVec.begin(), tempVec.end(), blockedQ.begin());
}


void parse_flag(int argc, char *argv[], string& strategy, int& quantum) {
    int option;
    string arg;
    while ( (option = getopt(argc, argv, "s:q:")) != -1 ) {
        switch (option) {
            // Scheduling Strategy
            case 's':
                arg = optarg;
                if (arg.compare("rr") == 0 || arg.compare("fcfs") == 0) {
                    strategy = arg;
                }
                else {
                    cerr << "\n╚(•⌂•)╝    " << optarg << " is an invalid scheduling strategy!" << endl;
                    usage_error();
                }
                break;
            // Time Quantum
            case 'q':
                if (stoi(optarg) > 0) {
                    quantum = stoi(optarg);
                }
                else {
                    cerr << "\n(σ｀д′)σ\nTime quantum must be a number and bigger than 0" << endl;
                    usage_error();
                }
                break;
            case '?':
                cerr    << "** UNKNOWN OPTION: " << char(option) << endl;
            default:
                usage_error();
        }
    }
}

void round_robin(int quantum, vector<deque<Burst>> processes) {

    // Initialize queues
    deque<deque<Burst>> blockedQ;

    deque<deque<Burst>> readyQ;
    for (deque<Burst> process : processes) {
        readyQ.push_back(process);  
    }


    // CPU and IO Execution times mapped by process ID
    int ioExecutedTimes[processes.size()];
    int cpuExecutedTimes[processes.size()];
    for (int i = 0; i < processes.size(); i++) {ioExecutedTimes[i] = 0; cpuExecutedTimes[i] = 0;}

    int elapsed_time = 0;
    int total_time = 0;

    int numIO;

    Burst* cpu_burst;
    Burst* io_burst;
    
    ExecutionStopReasonType stopReason;

    completionLog finished[processes.size()];
    int f = 0;

    while (!readyQ.empty() || !blockedQ.empty()) {

        // Default state: quantum exhausted
        stopReason = QUANTUM_EXPIRED;
        elapsed_time = quantum;

        // Processes all waiting for IO, handle as separate burst
        if (readyQ.empty()) {
            numIO = 0;
            io_burst = &(blockedQ.front().front());
            elapsed_time = io_burst->duration;
            // Exhaust IO based on shortest burst
            for (deque<Burst>& process : blockedQ) {
                io_burst = &(process.front());
                if (io_burst->duration <= elapsed_time) { // IO finishes within elapsed time
                    ioExecutedTimes[io_burst->id] += io_burst->duration;
                    io_burst->duration = 0;
                    numIO++;
                }
                else {
                    ioExecutedTimes[io_burst->id] += elapsed_time;
                    io_burst->duration -= elapsed_time;
                }
            }
            // Enqueue any processes that finished IO
            for (int i = 0; i < numIO; i++) {
                blockedQ.front().pop_front();
                readyQ.push_back(blockedQ.front());
                blockedQ.pop_front();
            }
            total_time += elapsed_time;
            continue; // Move to next burst
        } // END if

        // Handle CPU bursts
        cpu_burst = &(readyQ.front().front());
        if (cpu_burst->duration <= elapsed_time) { // CPU burst finishes before quantum
            cpuExecutedTimes[cpu_burst->id] += cpu_burst->duration;
            elapsed_time = cpu_burst->duration;
            cpu_burst->duration = 0;
            stopReason = (readyQ.front().size() != 1) ? ENTER_IO : COMPLETED;
        }
        else { // Quantum Expires before CPU burst finishes
            cpuExecutedTimes[cpu_burst->id] += elapsed_time;
            cpu_burst->duration -= elapsed_time;
        }

        // Handle any IO bursts
        if (!blockedQ.empty()) {
            numIO = 0;
            // Exhaust IO based on elapsed time
            for (deque<Burst>& process : blockedQ) {
                io_burst = &(process.front());
                if (io_burst->duration <= elapsed_time) { // IO finishes within elapsed time
                    ioExecutedTimes[io_burst->id] += io_burst->duration;
                    io_burst->duration = 0;
                    numIO++;
                }
                else {
                    ioExecutedTimes[io_burst->id] += elapsed_time;
                    io_burst->duration -= elapsed_time;
                }
            }
            // Enqueue any processes that finished IO
            for (int i = 0; i < numIO; i++) {
                blockedQ.front().pop_front();
                readyQ.push_back(blockedQ.front());
                blockedQ.pop_front();
            }
        } // END if

        total_time += elapsed_time;

        log_cpuburst_execution( cpu_burst->id, 
                                cpuExecutedTimes[cpu_burst->id],
                                ioExecutedTimes[cpu_burst->id],
                                total_time,
                                stopReason );

        // Handle Queue 
        switch (stopReason)
        {
        case QUANTUM_EXPIRED:
            readyQ.push_back(readyQ.front());
            readyQ.pop_front();
            break;
        case ENTER_IO:
            // Enqueue process that finished cpu burst
            readyQ.front().pop_front();
            blockedQ.push_back(readyQ.front());
            sortQ(blockedQ); // Stable sort blocked queue
            readyQ.pop_front();
            break;
        case COMPLETED:
            // Store log information for finished process
            finished[f].id = cpu_burst->id;
            finished[f].completionTime = total_time;
            finished[f].totalWaitTime = total_time - cpuExecutedTimes[cpu_burst->id] - ioExecutedTimes[cpu_burst->id];
            f++;
            // Dequeue process
            readyQ.pop_front();
            break;
        }
    } // END while

    for (int i = 0; i < processes.size(); i++) {
        log_process_completion(finished[i].id, finished[i].completionTime, finished[i].totalWaitTime);
    }
}

void fcfs(vector<deque<Burst>> processes) {

    // Initialize queues
    deque<deque<Burst>> blockedQ;

    deque<deque<Burst>> readyQ;
    for (deque<Burst> process : processes) {
        readyQ.push_back(process);  
    }


    // CPU and IO Execution times mapped by process ID
    int ioExecutedTimes[processes.size()];
    int cpuExecutedTimes[processes.size()];
    for (int i = 0; i < processes.size(); i++) {ioExecutedTimes[i] = 0; cpuExecutedTimes[i] = 0;}

    int elapsed_time = 0;
    int total_time = 0;

    int numIO;

    Burst* cpu_burst;
    Burst* io_burst;
    
    ExecutionStopReasonType stopReason;

    completionLog finished[processes.size()];
    int f = 0;

    while (!readyQ.empty() || !blockedQ.empty()) {

        // Default state: CPU burst exhausted
        stopReason = ENTER_IO;
        // Processes all waiting for IO, handle as separate burst
        if (readyQ.empty()) {
            numIO = 0;
            io_burst = &(blockedQ.front().front());
            elapsed_time = io_burst->duration;
            // Exhaust IO based on shortest burst
            for (deque<Burst>& process : blockedQ) {
                io_burst = &(process.front());
                if (io_burst->duration <= elapsed_time) { // IO finishes within elapsed time
                    ioExecutedTimes[io_burst->id] += io_burst->duration;
                    io_burst->duration = 0;
                    numIO++;
                }
                else {
                    ioExecutedTimes[io_burst->id] += elapsed_time;
                    io_burst->duration -= elapsed_time;
                }
            }
            // Enqueue any processes that finished IO
            for (int i = 0; i < numIO; i++) {
                blockedQ.front().pop_front();
                readyQ.push_back(blockedQ.front());
                blockedQ.pop_front();
            }
            total_time += elapsed_time;
            continue; // Move to next burst
        } // END if

        // Handle CPU bursts
        cpu_burst = &(readyQ.front().front());
        cpuExecutedTimes[cpu_burst->id] += cpu_burst->duration;
        elapsed_time = cpu_burst->duration;
        cpu_burst->duration = 0;
        stopReason = (readyQ.front().size() != 1) ? ENTER_IO : COMPLETED;
        
       

        // Handle any IO bursts
        if (!blockedQ.empty()) {
            numIO = 0;
            // Exhaust IO based on elapsed time
            for (deque<Burst>& process : blockedQ) {
                io_burst = &(process.front());
                if (io_burst->duration <= elapsed_time) { // IO finishes within elapsed time
                    ioExecutedTimes[io_burst->id] += io_burst->duration;
                    io_burst->duration = 0;
                    numIO++;
                }
                else {
                    ioExecutedTimes[io_burst->id] += elapsed_time;
                    io_burst->duration -= elapsed_time;
                }
            }
            // Enqueue any processes that finished IO
            for (int i = 0; i < numIO; i++) {
                blockedQ.front().pop_front();
                readyQ.push_back(blockedQ.front());
                blockedQ.pop_front();
            }
        } // END if

        total_time += elapsed_time;

        log_cpuburst_execution( cpu_burst->id, 
                                cpuExecutedTimes[cpu_burst->id],
                                ioExecutedTimes[cpu_burst->id],
                                total_time,
                                stopReason );

        // Handle Queue 
        switch (stopReason)
        {
        case QUANTUM_EXPIRED:
            readyQ.push_back(readyQ.front());
            readyQ.pop_front();
            break;
        case ENTER_IO:
            // Enqueue process that finished cpu burst
            readyQ.front().pop_front();
            blockedQ.push_back(readyQ.front());
            sortQ(blockedQ); // Stable sort blocked queue
            readyQ.pop_front();
            break;
        case COMPLETED:
            // Store log information for finished process
            finished[f].id = cpu_burst->id;
            finished[f].completionTime = total_time;
            finished[f].totalWaitTime = total_time - cpuExecutedTimes[cpu_burst->id] - ioExecutedTimes[cpu_burst->id];
            f++;
            // Dequeue process
            readyQ.pop_front();
            break;
        }
    } // END while

    for (int i = 0; i < processes.size(); i++) {
        log_process_completion(finished[i].id, finished[i].completionTime, finished[i].totalWaitTime);
    }
}

// Debugging (insert before while END)
// cout << "Iteration: " << i << endl;
// cout << "Elapsed Time: " << elapsed_time << endl;
// cout << "\tReady Queue:\t";
// for (deque<Burst> process : readyQ) {
//     cout << "P" << process.front().id << "[" << process.front().duration << "] ";
// }
// cout << "\n\tBlocked Queue:\t";
// for (deque<Burst> process : blockedQ) {
//     cout << "P" << process.front().id << "[" << process.front().duration << "] ";
// } cout << endl;