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
    //int wait_time;
    int cbt_remain;
    int io_remain;
    int cs_remain;
    int wait_time;
    int turnaround_begin;
    int turnaround_end;
    bool io_ing;
    bool b_done;
    bool cs_ing;
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
        cs_remain = 0;
        wait_time = 0;
        turnaround_begin = a_time;
        turnaround_end = 0;
        io_ing = false;
        b_done = false;
        cs_ing = false;
        //done = false;
    }
    Process(){
        
    }
    //test
    void Printp(){
        std::cout << process_id <<"|"<< arrival_time << "|" << cpu_burtst_time << "|" << num_bursts
        << "|" << io_time << std::endl;
    }
    //
    void tat_begin(int a){
        turnaround_begin = a;
    }
    void tat_end(int a){
        turnaround_end = a;
    }
    int tat(){
        return (turnaround_end - turnaround_begin);
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
    int iot_r(){
        return io_remain;
    }
    int wait_t(){
        return wait_time;
    }
    int get_cs(){
        return cs_remain;
    }
    int get_cbt(){
        return cbt_remain;
    }
    int get_wait_time(){
        return wait_time;
    }
    void set_cs(int a){
        cs_remain = a;
    }
    void cs_(){
        cs_remain -= 1;
    }
    void wait_(){
        if(!cs_ing){
        wait_time += 1;
        }
    }
    void io_(){
        io_remain -= 1;
    }
    void wait_minus(){
        wait_time -= 1;
    }
    void cs_switch(){
        cs_ing = !cs_ing;
    }
    bool get_b_done(){
        return b_done;
    }
    bool io_done(){
        if(io_remain <= 0){
            io_remain = io_time;
            return true;
        }
        return false;
    }
    bool check_new_burst(){
        if(cbt_remain == cpu_burtst_time){
            return true;
        }
        else{
            return false;
        }
    }
    bool io_done_2(){
        if(io_remain <= 0){
            //io_remain = io_time;
            return true;
        }
        return false;
    }
    char p_id(){
        return process_id;
    }
    bool check_b_done(){
        if(b_done){
            b_done = false;
            return true;
        }
        else{
            return false;
        }
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
            b_done = true;
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

