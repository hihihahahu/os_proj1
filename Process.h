#ifndef PROCESS_H
#define PROCESS_H

using namespace std;

class Process{
public:
    //identity variables
    char pid; //process_id
    int arrival_time;
    int cpu_burst_time;
    int num_burst;
    int io_time;
    
    //changing variables
    int remaining_burst_time;
    int remaining_bursts;
    int remaining_block_time;
    
    //constant variables
    int switch_time; //context_switch_time
    int time_slice; //time_slice: used only for round robin
     
    Process(char slot1, int slot2, int slot3, int slot4, int slot5, int switchtime, int timeslice){
        //identity variables
        pid = slot1;
        arrival_time = slot2;
        cpu_burst_time = slot3;
        num_burst = slot4;
        io_time = slot5;
        
        //changing variables
        remaining_burst_time = slot3;
        remaining_bursts = slot4;
        remaining_block_time = slot5;
        
        //constant variables
        switch_time = switchtime;
        time_slice = timeslice;       
    }
    
   
    
       
};

#endif /* pROCESS_H */
