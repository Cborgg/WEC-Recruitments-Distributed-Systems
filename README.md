# WEC-Recruitments - Distributed Systems
Implemented multithreaded system simulating interconnected computers storing data in files in C++. Threads perform random read, write, and update operations on separate files, recording operations and ensuring causal consistency using vector clocks.

# **Setup**
Compile the C++ code, ensure that all the computer_[0-4].txt files are in the same directory. Run the program from the same directory. The program will simulate the 
operations and the files computer_[0-4].txt would be updated. The operations are also logged into log.txt which you can tail to check the progress.

# **Working**
Each client is simulated as a thread (5 of them in total). The clients randomize the file operation and use message queues to communicate "an event". The message includes
the id of the thread originating the message, the operation (read/write/update) and its own vector clock. For the sake of simplicity reads are always done from the 
beginning of the file and writes are at the end of the file. When the peer threads receive the message, it first extracts the vector clock from the message and checks if it can be accepted by using the causal algorithm. If it can't be accepted the message is queued. If it is accepted then the requested operation is performed. Once the operation is performed the thread sends a notification to the logger thread for logging the operation. The logger thread is a simple implementation which waits for a message and logs it to the file "log.txt".

Note that when a thread sends the broadcast message it is received by itself as well. So causal alogrithm implementation handles this by always accepting messages 
originated by itself. Mutexes are used when queuing messages to maintain consistency. Conditional variables are used instead of a blind while loop when waiting for
messages to avoid wastage of CPU cycles. When update operation is done the code handles the case to only write within the EOF (i.e. it does not write past the end).

# Major Data Structures
**per_thd[MAX_COMPUTERS]:** Array of structures representing thread-specific information for different computers. <br>
<br>
  **vector_clocks:** A map where the key represents computer IDs, and each computer has associated vector clocks for tracking events. <br>
<br>
  **logger_info:** Struct containing a message queue, a mutex for synchronization, a condition variable, and a flag for the logger thread. <br>

