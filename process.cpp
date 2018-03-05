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
    int wait_time;
    int cbt_remain;
    int io_remain;
    bool io_ing;
    //bool done;
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
        cbt_remain = cpu_burtst_time;
        io_remain = io_time;
        wait_time = 0;
        io_ing = false;
        //done = false;
    }
    //test
    void Printp(){
        std::cout << process_id <<"|"<< arrival_time << "|" << cpu_burtst_time << "|" << num_bursts
        << "|" << io_time << std::endl;
    }
    bool check_done(){
        return (num_bursts == 0);
    }
    int arrival_t(){
        return arrival_time;
    }
    int cbt(){
        return cpu_burtst_time;
    }
    int num_b(){
        return num_bursts;
    }
    int iot(){
        return io_time;
    }
    int wait_t(){
        return wait_time;
    }
    void wait_(){
        wait_time += 1;
    }
    void io_start(){
        io_ing = true;
    }
    void io_end(){
        io_ing  = false;
    }
    void io_(){
        io += 1;
    }
    char p_id(){
        return process_id;
    }
    //run cpu burst for the time given (a)
    //hopefully a would be 1
    //return t if a burst is completed
    //return f otherwise
    bool run_cpu_burst(int a){
        //if time given is smaller than or equal to the remaining time of current burst
        //num_bursts - 1;
        if(cbt_remain <= a){
            num_bursts -= 1;
            cbt_remain = cpu_burtst_time;
            return true;
        }
        else{
            cbt_remain -= a;
            return false;
        }
        return false;
    }
    
};

//test

