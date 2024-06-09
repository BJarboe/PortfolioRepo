/** @name fooddelivery
 *  @file fooddelivery.cpp
 *  @author Bryce Jarboe | RedID 825033151
 *  @date 4/18/2024
 *  @brief Concurrent producing and Consuming via posix thread library
*/

#include "log.h"

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

void success() { 
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
    cerr    << "\n≡(▔﹏▔)≡  Usage: ./fooddelivery" << endl
            << "\t  Options:" << endl
            << "\t\t-n [Total number of delivery requests (production limit)]\tdefault=100" << endl
            << "\t\t-a [time for completing food delivery (service A)]\t\tdefault=0" << endl
            << "\t\t-b [time for completing food delivery (service B)]\t\tdefault=0" << endl
            << "\t\t-p [time for producing and publishing a sandwich request]\tdefault=0" << endl
            << "\t\t-s [time for producing and publishing a pizza request]\t\tdefault=0" << endl << endl;
    fail();
}

void parse_flags(int argc, char *argv[], Flags* flags) {
    int option;
    int arg;
    while ( (option = getopt(argc, argv, "n:a:b:p:s:")) != -1 ) {
        switch (option) {
            case 'n':
                if ((arg = stoi(optarg))>=0) {
                    flags->n = arg;
                } else {
                    usage_error();
                }
                break;
            case 'a':
                if ((arg = stoi(optarg))>=0) {
                    flags->a = arg;
                } else {
                    usage_error();
                }
                break;
            case 'b':
                if ((arg = stoi(optarg))>=0) {
                    flags->b = arg;
                } else {
                    usage_error();
                }
                break;
            case 'p':
                if ((arg = stoi(optarg))>=0) {
                    flags->p = arg;
                } else {
                    usage_error();
                }
                break;
            case 's':
                if ((arg = stoi(optarg))>=0) {
                    flags->s = arg;
                } else {
                    usage_error();
                }
                break;
            case '?':
                cerr    << "\t(´･ω･`)? UNKNOWN OPTION: " << char(option) << endl;
            default:
                usage_error();
        }
    }
} // END parse_flags()


int main(int argc, char *argv[]) {
    Flags flags;
    parse_flags(argc, argv, &flags);

    // Create Mutex
    pthread_mutex_t access_ctrl = PTHREAD_MUTEX_INITIALIZER;

    // Declare Threads: 2 producers, 2 consumers
    pthread_t pizza_producer;
    pthread_t sandwich_producer;
    pthread_t delivery_A;
    pthread_t delivery_B;

    deque<RequestType> brokerQ;
    int sandwich_count = 0;
    atomic<int> production_limit(flags.n);
    sem_t amount_empty;
    sem_t amount_full;
    sem_init(&amount_empty, 0, 20);
    sem_init(&amount_full, 0, 0);
    unsigned int produced[] = {0,0};
    unsigned int** consumed = new unsigned int*[ConsumerTypeN];
    for (int i = 0; i < ConsumerTypeN; ++i) {
        consumed[i] = new unsigned int[RequestTypeN]();
    }


    QueueData thread_args = {   &sandwich_count, &production_limit, &brokerQ, 
                                &access_ctrl, &amount_empty, &amount_full, 
                                produced, consumed, &flags                   };

    // Create threads
    pthread_create(&pizza_producer, NULL, little_caesars, (void*) &thread_args);
    pthread_create(&sandwich_producer, NULL, jersey_mikes, (void*) &thread_args);
    pthread_create(&delivery_A, NULL, uberEats, (void*) &thread_args);
    pthread_create(&delivery_B, NULL, doorDash, (void*) &thread_args);

    // Join threads
    pthread_join(pizza_producer, NULL);
    pthread_join(sandwich_producer, NULL);
    pthread_join(delivery_A, NULL);
    pthread_join(delivery_B, NULL);

    log_production_history(produced, consumed);

    // Clean Up
    pthread_mutex_destroy(&access_ctrl);
    sem_destroy(&amount_empty);
    sem_destroy(&amount_full);

    for (int i = 0; i < ConsumerTypeN; ++i) {
        delete[] consumed[i];
    }
    delete[] consumed;
    
    success();

} // END main()

// Producer threads
void order(RequestType food, QueueData* data) {
    pthread_mutex_lock(data->access_ctrl);

    unsigned int* inQ = new unsigned int[RequestTypeN]();
    for (int r = 0; r < RequestTypeN; r++) {
        inQ[r] = 0;
    }
    for (Requests r : *data->brokerQ) {
        inQ[r]++;
    }
    bool below_sandwich_limit = food != Sandwich || inQ[Sandwich] < 8;
    if (below_sandwich_limit) {
        if (data->production_limit->fetch_sub(1, memory_order_relaxed) > 0) {
            data->brokerQ->push_back(food);
            data->produced[food]++;
        }
    }
    
    

    if (below_sandwich_limit && food < RequestTypeN) {
        log_added_request(RequestAdded {food, data->produced, inQ}); 
    } 
    delete[] inQ;
    pthread_mutex_unlock(data->access_ctrl);
}


void *little_caesars(void *thread_args) { // Pizza
    QueueData *data = (QueueData*) (thread_args);

    // obtain sleep duration
    pthread_mutex_lock(data->access_ctrl);
    int dur = data->flags->s;
    pthread_mutex_unlock(data->access_ctrl);

    while (data->production_limit->load() > 0) {
        sleep(dur);
        sem_wait(data->amount_empty);
        order(Pizza, data);
        sem_post(data->amount_full);
    }
    return NULL;
}


void *jersey_mikes(void *thread_args) { // Sandwiches
    QueueData *data = (QueueData*) (thread_args);
    // obtain sleep duration
    pthread_mutex_lock(data->access_ctrl);
    int dur = data->flags->s;
    pthread_mutex_unlock(data->access_ctrl);

    while (data->production_limit->load() > 0) {
        sleep(dur);
        sem_wait(data->amount_empty);
        order(Sandwich, data);
        sem_post(data->amount_full);
    }
    return NULL;
}


// Consumer threads
void deliver(ConsumerType delivery, QueueData* data) {
    pthread_mutex_lock(data->access_ctrl);
    if (!data->brokerQ->empty()) {
        RequestType rt = data->brokerQ->front();
        data->brokerQ->pop_front();

        unsigned int inQ[] = {0,0};
        for (Requests r : *data->brokerQ) {
            inQ[r]++;
        }
        RequestRemoved rr = {delivery, rt, data->consumed[delivery], inQ};
        data->consumed[delivery][rt]++;
        log_removed_request(rr);

    }
    pthread_mutex_unlock(data->access_ctrl);
}

void *uberEats(void *thread_args) { // Delivery A
    QueueData *data = (QueueData*) (thread_args);
    struct timespec ts;
    int s;
    // obtain sleep duration
    pthread_mutex_lock(data->access_ctrl);
    int dur = data->flags->a;
    while (data->production_limit->load() > 0 || !data->brokerQ->empty()) {
        pthread_mutex_unlock(data->access_ctrl); // unlock after condition check
        sleep(dur);
        // Execute timed wait
        if (clock_gettime(CLOCK_REALTIME, &ts) == -1) {cerr << "Error getting time\n"; return (void*)1;}
        ts.tv_sec += 1;
        s = sem_timedwait(data->amount_full, &ts);

        if (s == 0 /*signal sent within time frame*/) {
            deliver(DeliveryServiceA, data);
            sem_post(data->amount_empty);
        }
        pthread_mutex_lock(data->access_ctrl); // lock for while (condition)
    }
    pthread_mutex_unlock(data->access_ctrl);

    return NULL;
}


void *doorDash(void *thread_args) { // Delivery B
    QueueData *data = (QueueData*) (thread_args);
    struct timespec ts;
    int s;
    // obtain sleep duration
    pthread_mutex_lock(data->access_ctrl);
    int dur = data->flags->b;

    while (data->production_limit->load() > 0 || !data->brokerQ->empty()) {
        pthread_mutex_unlock(data->access_ctrl);
        sleep(dur);
        // Execute timed wait
        if (clock_gettime(CLOCK_REALTIME, &ts) == -1) {cerr << "Error getting time\n"; return (void*)1;}
        ts.tv_sec += 1;
        s = sem_timedwait(data->amount_full, &ts);

        if (s == 0 /*signal sent within time frame*/) {
            deliver(DeliveryServiceB, data);
            sem_post(data->amount_empty);
        }
        pthread_mutex_lock(data->access_ctrl);
    }
    pthread_mutex_unlock(data->access_ctrl);
    return NULL;
}

// sleep function for implementing optional args' time duration
void sleep(int ms) {
    struct timespec ts;
    ts.tv_sec = ms / 1000;
    ts.tv_nsec = (ms % 1000) * 1000000L;
    nanosleep(&ts, NULL);
}
