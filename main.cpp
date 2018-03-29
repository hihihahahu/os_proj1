//
//  main.cpp
//  
//
//  Created by borute on 3/1/18.
//

#include "process.cpp"
#include <iostream>
#include <fstream>
#include <string>
#include <cstdlib>
#include <vector>
#include <fstream>
//global for rr that controls io halt(for some reason)
bool halt = false;
//global for rr that controls the process of putting processes into the cpu
//(waits 1ms - process doesn't start using cpu as it finishes context switch)
bool wait_ = false;
int time_ = 0;
int t_cs = 8;
bool p_end = false;
bool halt_load = false;
bool rr_add_end = true;
bool rr_sim_begin = true;
//stats
int num_cs = 0; //done
int num_preempt = 0; //done
float cbt_ = 0; //...done
float tat = 0; //done
float wait_time = 0; //done
float num_process = 0; //done
float num_bursts = 0; //done
//
std::vector<Process> wait_for_io;
std::vector<Process> wait_to_q;
std::vector<Process> tie_list;
std::vector<Process> finished;
std::vector<Process> d_finished;
std::vector<Process> queue;
//std::vector<int> tats;
Process preempted;
Process burst_complete;
//show preempt message
bool p_message = false;
bool b_complete_message = false;
void read_p(std::vector<Process> &p, std::ifstream &input){
    std::string line;
    char p_id;
    int iat; //arrival time
    int cbt; //cpu burst time
    int nb; //num of bursts
    int iot; //io time
    //read and create and push back processes
    while(std::getline(input, line)){
        std::string::iterator itr;
        std::string temp ="";
        int count = 1;
        itr = line.begin();
        if(*itr == '#'){
        }
        
        else{
            //first character is pid
            if(count == 1){
                p_id = *itr;
                itr++;
                itr++;//skip the pipe char
                count++;
            }
            if(count == 2){
                while(*itr != '|'){
                    temp.push_back(*itr);
                    //std::cout << *itr << std::endl;
                    itr++;
                }
                itr++;//skip the pipe char
                //std::cout << temp << std::endl;
                iat = std::atoi(temp.c_str());
                //std::cout << iat << std::endl;
                temp.clear();
                count++;
            }
            
            if(count == 3){
                while(*itr != '|'){
                    temp.push_back(*itr);
                    itr++;
                }
                itr++;//skip the pipe char
                cbt = std::atoi(temp.c_str());
                temp.clear();
                count++;
            }
            if(count == 4){
                while(*itr != '|'){
                    temp.push_back(*itr);
                    itr++;
                }
                itr++;//skip the pipe char
                nb = std::atoi(temp.c_str());
                temp.clear();
                count++;
            }
            
            if(count == 5){
                while(itr != line.end()){
                    temp.push_back(*itr);
                    itr++;
                }
                iot = std::atoi(temp.c_str());
            }
            
            p.push_back(Process(p_id, iat, cbt, nb, iot));
            cbt_ += cbt * nb;
            num_bursts += nb;
            num_process += 1;
        }
    }
}
/*
 should print:
 avg cpu burst time
 avg wait time
 #context swtices
 #preemtions
*/
void print_queue(std::vector<Process> &queue){
    std::cout << "[Q";
    for(int a = 0; a < queue.size(); a++){
        std::cout << " " << queue[a].p_id();
    }
    if(queue.size() != 0){
        std::cout << "]" << std::endl;
    }
    else{
        std::cout << " <empty>]" << std::endl;
    }
}
void process_io(std::vector<Process> &blocked, std::vector<Process> &finished,
                std::vector<Process> &queue){
    //if there is process waiting for io check
    if(blocked.size() != 0){
        //printf("checking blocked list\n");
        for(unsigned int a = 0; a < blocked.size(); a++){
            //if there's no need to halt, process io for all processes
            //processes with 0 io time will also be processed here,
            //  but will be removed in the next if block
            if(blocked.size() != 0){
                blocked[a].io_();
                //printf("%d %c\n",time, blocked[a].p_id());
            }
            halt = false;
            if(blocked[a].check_done()){
                //if terminates, remove from blocked list and move to finished list
                //printf("%dms : process %c terminated ", time_, blocked[a].p_id());
                //print_queue(queue);
                finished.push_back(blocked[a]);
                blocked.erase(blocked.begin() + a);
                a--;
            }
            //remove any process that has done its io from the blocked list
            else if(blocked[a].io_done()){
                //otherwise put it back to ready queue
                //printf("%dms : process %c pushed back to ready queue from blocked list", time_, blocked[a].p_id());
                //what if this process would continue to occupy the cpu after finishing this io processing?
                //do not halt load(it should be loaded in the next timeslot
                if(queue.size() == 0){
                    halt_load = false;
                }
                wait_to_q.push_back(blocked[a]);
                //print_queue(queue);
                blocked.erase(blocked.begin() + a);
                a--;
            }
        }
    }
}
void check_arrival(std::vector<Process> &p, std::vector<Process> &queue){
    for (unsigned int a = 0; a < p.size(); a++){
        if(p[a].arrival_t() == time_){
            //add process to queue
            
            //queue.push_back(p[a]);
            tie_list.push_back(p[a]);
            //print_queue(queue);
            p.erase(p.begin()+a);
            //minus one because size of p decrease by 1
            a--;
        }
    }
}
//process moving out of cpu
//loading process from queue
void cs_first_half(std::vector<Process> &cs_out, int &cs_cd,
                   std::vector<Process> &blocked, std::vector<Process> &queue){
    //if ready queue is empty, cs can actually be skipped
    if(queue.size() == 0){
        //skip cs
        cs_cd = 0;
        //c_swing = false;
    }
    //check if the process has done its burst
    if(cs_out[0].check_b_done() && cs_out.size() != 0){
        //if burst done, enters block state and process io later
        num_cs += 1;
        cs_out[0].cs_switch();
        if(!cs_out[0].io_done()){
            //printf("%dms : process %c enters blocked state (first half of context switch ended)", time_, cs_out[0].p_id());
            //print_queue(queue);
            if(cs_out[0].check_done()){
                finished.push_back(cs_out[0]);
            }
            else{
                rr_sim_begin = true;
                wait_for_io.push_back(cs_out[0]);
                halt = true;
            }
        }
        else{
            finished.push_back(cs_out[0]);
        }
    }
    //or it goes back to the queue
    else if(cs_out.size() != 0){
        if(cs_out.size() != 0){
            //if io processing is needed
            /*
            if(!cs_out[0].io_done()){
                cs_out[0].cs_switch();
                wait_for_io.push_back(cs_out[0]);
            }
            else{
             */
                //printf("%dms : process %c returns to ready queue", time_, cs_out[0].p_id());
                cs_out[0].cs_switch();
                //queue.push_back(cs_out[0]);
                //this is actually a hard-coding fix
                //cs_out[0] doesn't enter the ready queue at this timeslice
                //it enters at next time slice
                //but to avoid that 1ms extra wait time
                //wait_minus is executed to get the correct wait time
                cs_out[0].wait_minus();
               //check if the process has completed its own cpu burst
                if(cs_out[0].get_cbt() == cs_out[0].cbt()){
                    cs_out[0].tat_begin(time_ + 1);
                    //printf("begin set at time %d (process %c) (no I/O)\n", time_,cs_out[0].p_id());
                }
            
            if(rr_add_end || cs_out[0].get_cbt() != cs_out[0].cbt()){
                rr_sim_begin = true;
                queue.push_back(cs_out[0]);
            }
            else{
                queue.insert(queue.begin(), cs_out[0]);
            }
            num_cs += 1;
                //print_queue(queue);
            //}
        }
    }
    //wait, cs_out is empty and nothing moved out
    else{
        halt_load = false;
    }
    //the process should have gone somewhere (blocked or queue), clear cs_out
    if(cs_out.size() != 0){
        halt_load = true;
        cs_out.erase(cs_out.begin());
    }
}
//process moving into cpu
//saving process to queue
void cs_second_half(std::vector<Process> &cs_in, Process &cpu, std::vector<Process> &queue,
                    int &cs_cd, bool &c_swing){
    //if there are no processes waiting for context switch
    //skip and label that there is no cs
    if(queue.size() == 0){
        cs_cd = 0;
        c_swing = false;
    }
    //or put the next process on queue into context switch
    else{
        //printf("%dms : process %c begins entering cpu (enters second half of context switch)", time_, queue[0].p_id());
        
        //turn cs on
        if(rr_add_end || queue.size() == 1){
            queue[0].cs_switch();
        //queue[0].wait_minus();
            cs_in.push_back(queue[0]);
            queue.erase(queue.begin());
        }
        else{
            if(!rr_sim_begin){
                queue[1].cs_switch();
                //queue[0].wait_minus();
                cs_in.push_back(queue[1]);
                queue.erase(queue.begin()+1);
            }
            else{
                queue[0].cs_switch();
                //queue[0].wait_minus();
                cs_in.push_back(queue[0]);
                queue.erase(queue.begin());
                rr_sim_begin = false;
            }
        }
        num_cs += 1;
        //print_queue(queue);
    }
}
void check_cs_complete(int &cs_cd, int &t_cs, bool &c_swing, bool &wait_){
    if(cs_cd == t_cs - 1){
        //printf("%dms : (second half of) context switch done\n", time_);
        cs_cd = 0;
        c_swing = false;
        //the process moving into the cpu needs to wait 1ms upon cs completion to start using cpu
        wait_ = true;
    }
}
void tie_breaker(std::vector<Process> &queue){
    while(tie_list.size() != 0){
        int key;
        char id = 'z';
        for(unsigned int a = 0; a < tie_list.size(); a++){
            if(tie_list[a].p_id() < id){
                id = tie_list[a].p_id();
                key = a;
            }
        }
        if(rr_add_end || queue.size() == 0){
            queue.push_back(tie_list[key]);
        }
        else{
            queue.insert(queue.begin(), tie_list[key]);
        }
        std::cout << "time " << time_ << "ms: Process " << tie_list[key].p_id() << " arrived and added to ready queue ";
        print_queue(queue);
        tie_list.erase(tie_list.begin() + key);
    }
}
void preempt_message(){
    if(queue.size() != 0){
        std::cout << "time " << time_ << "ms: Time slice expired; process " << preempted.p_id() << " preempted with " << preempted.get_cbt() << "ms to go ";
        num_preempt += 1;
    }
    else{
        std::cout << "time " << time_ << "ms: Time slice expired; no preemption because ready queue is empty ";
    }
    print_queue(queue);
}
void burst_complete_message(){
    if(burst_complete.num_b() > 1){
        std::cout << "time " << time_ << "ms: Process " << burst_complete.p_id() << " completed a CPU burst; " << burst_complete.num_b() << " bursts to go ";
    }
    else{
        std::cout << "time " << time_ << "ms: Process " << burst_complete.p_id() << " completed a CPU burst; " << burst_complete.num_b() << " burst to go ";
    }
    print_queue(queue);
    burst_complete.tat_end(time_ + t_cs/2);
    printf("Process %c tat end at %d\n", burst_complete.p_id(), time_ + t_cs/2);
    tat += (burst_complete.tat());
     printf("%d\n", burst_complete.tat());
    if(burst_complete.iot() != 0){
        std::cout << "time " << time_ << "ms: Process " << burst_complete.p_id() << " switching out of CPU; will block on I/O until time " << time_ + burst_complete.iot() + t_cs/2 << "ms ";
        print_queue(queue);
    }
}
//check if the process in cs_out actually terminates (i.e. no io needed, and report terminate before it enters cs)
void round_robin(std::vector<Process> p){
    std::vector<Process> p_s = p;
    std::vector<Process> cs_in; //processes undergoing cs
    std::vector<Process> cs_out;
    Process cpu; //the process using the cpu
    std::vector<Process> blocked;
    bool b_finish = 0;
    unsigned int process_count = p.size();
    //int cb_count = 0; //cpu burst count
    //int preemp = 0;
    //int c_sw = 0; //count of context switch
    int t_slice = 80;
   
    time_ = 0; //time
    int cs_cd = 0; //context switch countdown
    int cb_remain = t_slice;
    bool cpu_in_use = false;
    bool c_swing = true; //cw status
    bool force_quit = false;
    wait_ = false;
    halt = false;
    //(while there are unprocessed processes)
    std::cout << "time " << time_ << "ms: Simulator started for RR ";
    print_queue(queue);
    while(finished.size() != process_count){
        //increment wait time
        if(queue.size() != 0){
            for(unsigned int a = 0; a < queue.size(); a++){
                queue[a].wait_();
            //wait_time += 1;
            }
        }
        if(p_message){
            p_message = false;
            preempt_message();
        }
        if(b_complete_message){
            b_complete_message = false;
            burst_complete_message();
        }
        if(d_finished.size() != 0){
            //printf("%dms : process %c terminated\n", time_, d_finished[0].p_id());
            std::cout << "time " << time_ << "ms: Process " << d_finished[0].p_id() << " terminated ";
            print_queue(queue);
            d_finished[0].tat_end(time_ + t_cs/2);
            tat += (d_finished[0].tat());
            printf("Process %c tat end at %d\n", d_finished[0].p_id(), time_ + t_cs/2);
            //printf("%d\n", d_finished[0].tat());
            d_finished.erase(d_finished.begin());
        }
        if(wait_for_io.size() != 0){
            blocked.push_back(wait_for_io[0]);
            wait_for_io.erase(wait_for_io.begin());
        }
        
        if(cs_out.size() != 0){
            c_swing = true;
        }
        
        //check_terminate();
        
        if(wait_to_q.size() != 0){
            //wait_to_q[0].tat_begin(time_);
            //printf("yo\n");
            if(wait_to_q[0].get_cbt() == wait_to_q[0].cbt()){
                wait_to_q[0].tat_begin(time_);
                printf("begin set at time %d (process %c)\n", time_,wait_to_q[0].p_id());
            }
            if(rr_add_end || queue.size() == 0){
                queue.push_back(wait_to_q[0]);
            }
            else{
                queue.insert(queue.begin(), wait_to_q[0]);
            }
            
            
            //tie_list.push_back(wait_to_q[0]);
            //printf("%dms : process %c pushed back to ready queue from blocked list", time_, wait_to_q[0].p_id());
            std::cout << "time " << time_ << "ms: Process " << wait_to_q[0].p_id() << " completed I/O; added to ready queue ";
            wait_to_q.erase(wait_to_q.begin());
            print_queue(queue);
            //process_io(blocked, finished, queue);
        }
        
        process_io(blocked, finished, queue);
        check_arrival(p, queue);
        tie_breaker(queue);
        //now newly arrived process(es) are added to queue (if any)
        //check if first half of cs is done
        if(cs_cd == 3){
            //printf("%dms : cs_out size is %lu\n", time_, cs_out.size());
            cs_first_half(cs_out, cs_cd, blocked, queue);
            //after a process is saved to queue, wait for the next ms to load the process from next queue
            //halt_load = true;
        }
        //printf("size of queue %lu\n", queue.size());
        //force start second half of cs if a process is waiting to enter cpu,
        //and there is no process being loaded
        if(!cpu_in_use && (queue.size() != 0) && (cs_in.size() == 0) && (cs_out.size() == 0) && !halt_load){
            cs_cd = 4;
        }
        halt_load = false;
        //second half begins
        if(cs_cd == 4){
            cs_second_half(cs_in, cpu, queue, cs_cd, c_swing);
        }
        //check if context switch is done, if yes,
        //change context switch status and reset countdown
        check_cs_complete(cs_cd, t_cs, c_swing, wait_);
        //wait one more ms before the process actually starts using cpu
        
        //check if cpu is in use first
        //if cpu is in use
        if(cpu_in_use){
            cb_remain -= 1;
            b_finish = cpu.run_cpu_burst(1);
        }
        //if cpu is not in use and context switch not taking place
        else if(!cpu_in_use && !c_swing && !wait_ && (cs_in.size() != 0)){
            cb_remain -= 1;
            if(cs_in[0].check_new_burst()){
                std::cout << "time " << time_ << "ms: Process " << cs_in[0].p_id() << " started using the CPU ";
                //cs_in[0].tat_begin(time_ - t_cs/2);
            }
            else{
                std::cout << "time " << time_ << "ms: Process " << cs_in[0].p_id() << " started using the CPU with " << cs_in[0].get_cbt() << "ms remaining ";
                //cs_in[0].tat_begin(time_ - t_cs/2);
            }
            print_queue(queue);
            cpu_in_use = true;
            cpu = cs_in[0];
            cpu.cs_switch();
            b_finish = cpu.run_cpu_burst(1);
            cs_in.erase(cs_in.begin());
            //what else to do...?
        }
        wait_ = false;
        //if timeout
        if(cb_remain == 0 && cpu.cbt() > t_slice && !cpu.get_b_done()){
            //force finish
            //printf("%dms : process %c finishes cpu burst due to timeout", time_, cpu.p_id());
            preempted = cpu;
            p_message = true;
            force_quit = true;
            //cb_remain = t_slice;
            b_finish = true;
        }
        //if a process has done cpu bursting...?
        if(cpu_in_use && b_finish){
            //reset the flag
            b_finish = false;
            //reset burst time slice countdown;
            cb_remain = t_slice;
            if(!force_quit){
                //printf("%dms : process %c finished cpu burst normally", time_, cpu.p_id());
                if(cpu.num_b() != 0){
                    burst_complete = cpu;
                    b_complete_message = true;
                }
            }
            //if time slice expires and no process waiting to enter cpu,
            //no context switch and the process remains where it was
            if(force_quit && (queue.size() == 0)){
                //no context switch
                c_swing = false;
                cpu_in_use = true;
            }
            //if there is another process waiting, this process should be moved out
            //(or if the process ends normally)
            //!!!
            
            //process should go to blocked for io check first before entering cs
            //necessarily?
            
            //!!!
            //preempted
            else{
                cpu.cs_switch();
                cs_out.push_back(cpu);
                //if the process has actually terminated
                if(cpu.check_done()){
                    //it will be declared terminated at next time slice
                    //though cs is still taking place
                    d_finished.push_back(cpu);
                }
                
                //c_swing = true;
                cpu_in_use = false;
                p_end = true;
                //cs_cd = -1;
            }
            //reset the flag
            force_quit = false;
        }
        //if context switch is taking place
        if(c_swing && !p_end){
            cs_cd += 1;
        }
        p_end = false;
        //io time
        
        //add wait time of all processes in ready queue by 1
        /*
        if(time_ != -1){
            for(unsigned int a = 0; a < queue.size(); a++){
                queue[a].wait_();
            }
        }
         */
        //printf("%d\n", time_);
        time_ += 1;
    }
    //printf("%dms : simulator ended\n", time_);
    std::cout << "time " << time_ << "ms: Simulator ended for RR" << std::endl;
    
    std::cout << "-- avg cpu burst time: " << (float)cbt_/(float)num_bursts << "ms" << std::endl;
    //find total wait time
    
    for(unsigned int a = 0; a < finished.size(); a++){
        wait_time += finished[a].get_wait_time();
        //printf("wait time for process %c is %d\n", finished[a].p_id(), finished[a].get_wait_time() - 1);
    }
    
    std::cout << "-- avg wait time: " << (float)(wait_time)/(float)(num_bursts) << "ms" << std::endl;
    std::cout << "avg turnaround time: " << (float)(tat)/(float)(num_bursts) << "ms" << std::endl;
    std::cout << "total number of cs: " << num_cs/2 << std::endl;
    std::cout << "total number of preemptions: " << num_preempt << std::endl;
}

//looks like processes are sorted by arrival time and process id - great!

int main(int argc, char* argv[]){
    std::vector<Process> p;
    //std::string input_file = argv[1];
    std::ifstream input(argv[1]);
    read_p(p, input);
    if(argc == 3){
        std::string arg(argv[2]);
        if(arg.compare("END") == 0){
            rr_add_end = true;
        }
        else if(arg.compare("BEGINNING") == 0){
            rr_add_end = false;
        }
    }
    //done reading/creating processes
    //print processes (testing purpose)
    /*
    for(unsigned int a = 0; a < p.size(); a++){
        p[a].Printp();
    }
     */
    //algorithms
    round_robin(p);
    return 0;
}
