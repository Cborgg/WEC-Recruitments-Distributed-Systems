
#include "Multi-Thd-model.h"

struct per_thd_info_        per_thd[MAX_COMPUTERS];

map<int, Penta> vector_clocks; // Vector clocks for each "computer"

// Implements casual Algorithm for broadcast messages using vc
bool accept_msg (int my_id, struct my_msg *rcvd)
{
    int     i;

    // Accept messages coming from our own thread
    if (my_id == rcvd->src_thd)
        return true;

    if ((vector_clocks[my_id].val[rcvd->src_thd] +1) != rcvd->val[rcvd->src_thd])
        return false;

    
    
    for (i = 0; i < MAX_COMPUTERS; i++) {

        if (i == rcvd->src_thd)
            continue;

        if (rcvd->val[my_id] > vector_clocks[my_id].val[my_id])
            return false;
    }
    // Update the value from the message
    vector_clocks[my_id].val[rcvd->src_thd] = rcvd->val[rcvd->src_thd];
    return true;
}

struct logger_info_ {
    std::queue<log_msg>   messageQueue;
    std::mutex          mtx;
    std::condition_variable cv;
    bool                 msg_posted = false;
};

struct logger_info_ logger_info;

void logger (void)
{
    ofstream log_file;

    log_file.open("log.txt");

    while (true) {
        {
            std::unique_lock<std::mutex> lock(logger_info.mtx);
            // Wait on a conditional variable to avoid spending cpu cycles while waiting
            // for a message
            logger_info.cv.wait(lock, [] {return logger_info.msg_posted; });
                    
            while (!logger_info.messageQueue.empty()) { 
                struct log_msg    l;
                            
                l = logger_info.messageQueue.front();
                logger_info.messageQueue.pop();

                log_file<<"Logging from Thread "<<l.src_thd << " Operation " << l.oper << " at " << l.log_time << endl;
            }
            logger_info.msg_posted = false;
        }
    }                  
}

static long get_file_size (fstream& fio)
{
    std::streampos fsize = 0;

    fsize = fio.tellg();         // The file pointer is currently at the beginning
    fio.seekg(0, ios::end);      // Place the file pointer at the end of file

    fsize = fio.tellg() - fsize;

    
    return (long) fsize;
}

void perform_read (int thd_id) 
{
    fstream fio;
    char    buf[READ_SIZE+1];

    fio.open(vector_clocks[thd_id].fname, ios::out | ios::in);

    if (!fio) {
        cout << "Unable to open file " << vector_clocks[thd_id].fname << endl;
        return;
    }
                    
    long      fsize = get_file_size(fio);
    if (fsize < READ_SIZE) {
        cout << "-ERROR- File " << vector_clocks[thd_id].fname << " EOF reached " << endl;
        return;
    }
    cout << "size is: " << fsize << " bytes in thd " << thd_id << endl;

    fio.seekg(0, ios::beg);
    fio.read(buf, READ_SIZE);
    buf[READ_SIZE] = '\0';
    cout << "Reading in thd " << thd_id << " data " << buf << endl;

    fio.close();
}

void perform_write (int thd_id, int offset, string buf)
{
    fstream fio;

    fio.open(vector_clocks[thd_id].fname, ios::out | ios::in);

    if (!fio) {
        cout << "Unable to open file in perform_write " << vector_clocks[thd_id].fname << endl;
        return;
    }
    
    // If offset is -1 then write at end of the file, if not then beginining (update case)
    if (offset == -1) {
        fio.seekg(0, ios::end);
    } else {
        fio.seekg(0, ios::beg);
    }
    fio.write(buf.data(), buf.size());
    fio.close();
}

void file_sync (int src_id)
{
    // Take the mutex before sync'ing the files
}

void log_operation(int comp_id, int oper)
{
    struct log_msg   msg;

    msg.src_thd = comp_id;
    msg.oper = oper;
    const auto now = chrono::system_clock::now();
    const time_t t_c = chrono::system_clock::to_time_t(now);
    msg.log_time = ctime(&t_c);

     // Send the message to the target thread
    lock_guard<std::mutex> lock(logger_info.mtx);
    logger_info.messageQueue.push(msg);
    logger_info.msg_posted = true;
    logger_info.cv.notify_one();
}


// Function to perform operations in a client thread
void client_thread(int computer_id) {
    struct my_msg   msg;
    cout << "Entering infinite loop for thd ..." << computer_id << std::endl;
    
    while (true) {
        // Simulate random time intervals, make sure to always sleep hence the +1
        std::this_thread::sleep_for(std::chrono::seconds(rand() % 5 + 1));

        // Generate a random operation (read, write, or update)
        int operation = rand() % 3;

        msg.src_thd = computer_id;
        msg.oper = operation;
        msg.data = str_to_write;
        // Update the string to write a different data every time
        str_to_write[str_to_write.size()  - 1] += 1;
        
        vector_clocks[computer_id].val[computer_id] += 1;

        int i;
        for (i = 0; i < MAX_COMPUTERS; i++)
            msg.val[i] = vector_clocks[computer_id].val[i];

        {
            // Send the message to all threads 
            int i;

            for (i = 0; i < MAX_COMPUTERS; i++) {

        //        if (i == computer_id)
        //            continue;
                //extra braces are put here so that when lock goes out of scope, it ceases to exist and hence the lock gets unlocked
        //if we don't put extra braces the lock will remain for all iterations of i and it won't unlock until it reaches the last thread(i.e. when i=MAX_COMPUTERS-1)
                { 
                std::lock_guard<std::mutex> lock(per_thd[i].mtx);

                per_thd[i].messageQueue.push(msg);
                per_thd[i].cv.notify_one();
                }
            }
        }
        // Check for messages in my queue
        {
                std::unique_lock<std::mutex> lock(per_thd[computer_id].mtx);

                while (per_thd[computer_id].messageQueue.empty())
                    per_thd[computer_id].cv.wait(lock);

                while (!per_thd[computer_id].messageQueue.empty()) { 
                    struct my_msg    rcvd_;
                    
                    rcvd_ = per_thd[computer_id].messageQueue.front();
                    per_thd[computer_id].messageQueue.pop();
                
                    if (!accept_msg(computer_id, &rcvd_)) {
                        // Message arrived too early, push it back into the queue
                        per_thd[computer_id].messageQueue.push(rcvd_);
                        
                        break;
                    }

                    // Perform operation based on operation type
                    if (rcvd_.oper == 0) {
                        perform_read(computer_id);
                    } else if (rcvd_.oper == 1) {
                        // Write at end of the file
                        perform_write(computer_id, -1, msg.data);
                    } else {
                        // Update is read and write at the same location
                        perform_write(computer_id, 0, msg.data);
                    }
                    log_operation(computer_id, rcvd_.oper);
                }
        }
    }
}

int main() {

    //Initialize each "computer" with vector clock and its filename
    for (int i = 0; i < MAX_COMPUTERS; ++i) {
        string  fname = "computer_" + std::to_string(i) + ".txt";
        struct Penta p(fname);

        std::pair< int, Penta > ptr = std::make_pair(i, p);
        vector_clocks.insert(ptr);
    }

    std::vector<std::thread> threads;
    for (int i = 0; i < MAX_COMPUTERS; ++i) {
        threads.emplace_back(client_thread, i);
    }
    thread  logger_thd(logger);

    // Join all threads to prevent the program from exiting prematurely
    for (std::thread& t : threads) {
        t.join();
    }

    logger_thd.join();

    return 0;
}
