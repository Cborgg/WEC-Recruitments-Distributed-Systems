# WEC-Recruitments---Distributed-Systems
Implemented multithreaded system simulating interconnected computers storing data in files in C++. Threads perform random read, write, and update operations on separate files, recording operations and ensuring causal consistency using vector clocks.

Setup
Compile the C++ code, ensure that all the computer_[0-4].txt files are in the same directory. Run the program from the same directory. The program will simulate the 
operations and the files computer_[0-4].txt would be updated. The operations are also logged into log.txt which you can tail to check the progress.
