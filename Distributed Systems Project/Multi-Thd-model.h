#include <iostream>
#include <thread>
#include <map>
#include <mutex>
#include <condition_variable>
#include <queue>
#include <vector>
#include <fstream>
#include <chrono>
#include <ctime>

using namespace std;

#define MAX_COMPUTERS   5
#define MAX_LINES_IN_FILE 27   // Maximum lines in sample file. Should not read beyond this  
#define READ_SIZE       5
#define WRITE_SIZE      READ_SIZE

string str_to_write("ABCDEF");

struct Penta {
    int val[MAX_COMPUTERS]; 
    string fname;

    Penta(string name) {
        int i;
        for ( i =0 ; i < MAX_COMPUTERS; i++)
            val[i] = 0;

        fname = name;
    }
    Penta() {
        int i;
        for ( i =0 ; i < MAX_COMPUTERS; i++)
            val[i] = 0;

        fname = "noname.txt";      
    }
};

// Message structure passed across threads
struct my_msg
{
     /* data */
    int             val[MAX_COMPUTERS]; 
    int             src_thd;
    int             oper;
    string          data; 
};

struct log_msg
{
    int             src_thd;
    int             oper;
    string          log_time;
};

struct per_thd_info_ {
    queue<my_msg>   messageQueue;
    mutex          mtx;
    condition_variable cv;
};

mutex    file_sync_mtx;