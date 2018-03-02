//
//  os-p1.cpp
//  
//
//  Created by borute on 3/1/18.
//

//#include "os-p1.h"
#include <iostream>

class Process{
private:
    char process_id;
    int arrival_time;
    int cpu_burtst_time;
    int num_bursts;
    int io_time;
    /*
    int remaining_bursts;
    int remaining_burst_time;
    int wait_time;
    int turnaround_time;
     */
public:
    Process(char p_id, int a_time, int b_time, int num_b, int io_t){
        process_id = p_id;
        arrival_time = a_time;
        cpu_burtst_time = b_time;
        num_bursts = num_b;
        io_time = io_t;
    }
    //test
    void Printp(){
        std::cout << process_id <<"|"<< arrival_time << "|" << cpu_burtst_time << "|" << num_bursts
        << "|" << io_time << std::endl;
    }
    
};

//test

