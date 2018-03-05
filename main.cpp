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
bool wait = false;

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
void process_io(std::vector<Process> &blocked, std::vector<Process> &finished
                std::vector<Process> &queue){
    //if there is process waiting for io check
    if(blocked.size() != 0){
        //printf("checking blocked list\n");
        for(unsigned int a = 0; a < blocked.size(); a++){
            //if there's no need to halt, process io for all processes
            //processes with 0 io time will also be processed here,
            //  but will be removed in the next if block
            if(blocked.size() != 0 && !halt){
                blocked[a].io_();
                //printf("%d %c\n",time, blocked[a].p_id());
            }
            halt = false;
            //remove any process that has done its io from the blocked list
            if(blocked[a].io_done()){
                //check if this process terminates
                if(blocked[a].check_done()){
                    //if terminates, remove from blocked list and move to finished list
                    printf("%dms : process %c terminated\n", time, blocked[a].p_id());
                    finished.push_back(blocked[a]);
                    blocked.erase(blocked.begin() + a);
                }
                else{
                    //otherwise put it back to ready queue
                    printf("%dms : process %c pushed back to queue\n", time, blocked[a].p_id());
                    queue.push_back(blocked[a]);
                    blocked.erase(blocked.begin() + a);
                }
                a--;
            }
            
        }
    }
}
void check_arrival(std::vector<Process> &p, std::vector<Process> &queue){
    for (unsigned int a = 0; a < p.size(); a++){
        if(p[a].arrival_t() == time){
            //add process to queue
            printf("%dms : added process %c to queue\n", time, p[a].p_id());
            queue.push_back(p[a]);
            p.erase(p.begin()+a);
            //minus one because size of p decrease by 1
            a--;
        }
    }
}
//process moving out of cpu
void cs_first_half(std::vector<Process> &cs_out, int &cs_cd,
                   std::vector<Process> &blocked){
    //if ready queue is empty, cs can actually be skipped
    if(queue.size() == 0){
        //skip cs
        cs_cd = 0;
        //c_swing = false;
    }
    //check if the process has done its burst
    if(cs_out[0].check_b_done()){
        //if burst done, enters block state and process io later
        printf("%dms : process %c enters blocked state\n", time, cs_out[0].p_id());
        blocked.push_back(cs_out[0]);
        halt = true;
    }
    //or it goes back to the queue
    else{
        if(cs_out.size() != 0){
            queue.push_back(cs_out[0]);
        }
    }
    //the process should have gone somewhere (blocked or queue), clear cs_out
    if(cs_out.size() != 0){
        cs_out.erase(cs_out.begin());
    }
}
//process moving into cpu
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
        printf("%dms : process %c begins entering cpu (enters context switch)\n", time, queue[0].p_id());
        //turn cs on
        queue[0].cs_switch();
        cs_in.push_back(queue[0]);
        queue.erase(queue.begin());
    }
}
void check_cs_complete(int &cs_cd, int &t_cs, bool &c_swing, bool &wait){
    if(cs_cd == t_cs - 1){
        printf("%dms : context switch done\n", time);
        cs_cd = 0;
        c_swing = false;
        //the process moving into the cpu needs to wait 1ms upon cs completion to start using cpu
        wait = true;
    }
}
void round_robin(std::vector<Process> p){
    std::vector<Process> p_s = p;
    std::vector<Process> queue;
    std::vector<Process> finished;
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
    int t_cs = 8;
    int time = 0; //time
    int cs_cd = (t_cs/2); //context switch countdown
    int cb_remain = t_slice;
    //default is this because first process takes t_cs/2 ms to get
    //to cpu
    bool cpu_in_use = false;
    bool c_swing = true; //cw status
    bool force_quit = false;
    bool wait = false;
    //bool penalty = false;
    halt = false;
    //bool compensate = true;
    //bool all_finish = false;
    //(while there are unprocessed processes)
    while(finished.size() != process_count){
        check_arrival(p, queue);
        //now newly arrived process(es) are added to queue (if any)
        //check if first half of cs is done
        if(cs_cd == 3){
            cs_first_half(cs_out, cs_cd, blocked);
        }
        //printf("size of queue %lu\n", queue.size());
        //second half begins
        if(cs_cd == 4){
            cs_second_half(cs_in, cpu, queue, cs_cd, c_swing);
        }
        //check if context switch is done, if yes,
        //change context switch status and reset countdown
        check_cs_complete(cs_cd, t_cs, c_swing, wait);
        //wait one more ms before the process actually starts using cpu
        
        //check if cpu is in use first
        //if cpu is in use
        if(cpu_in_use){
            cb_remain -= 1;
            b_finish = cpu.run_cpu_burst(1);
        }
        //if cpu is not in use and context switch not taking place
        else if(!cpu_in_use && !c_swing && !wait){
            cb_remain -= 1;
            printf("%dms : process %c starts cpu burst\n", time, cs_in[0].p_id());
            cpu_in_use = true;
            cpu = cs_in[0];
            b_finish = cpu.run_cpu_burst(1);
            cs_in.erase(cs_in.begin());
            //what else to do...?
        }
        wait = false;
        //if timeout
        if(cb_remain == 0){
            //force finish
            printf("%dms : process %c finishes cpu burst due to timeout\n", time, cpu.p_id());
            force_quit = true;
            //cb_remain = t_slice;
            b_finish = true;
        }
        //if a process has done cpu bursting...?
        if(cpu_in_use && b_finish){
            cb_remain = t_slice;
            if(!force_quit){
                printf("%dms : process %c finished cpu burst normally\n", time, cpu.p_id());
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
            else{
                cs_out.push_back(cpu);
                //context switch begin
                c_swing = true;
                cpu_in_use = false;
            }
            //reset the flag
            force_quit = false;
        }
        //if context switch is taking place
        if(c_swing){
            cs_cd += 1;
        }
        //io time
        process_io(blocked, finished, queue);
        //add wait time of all processes in queue by 1
        if(time != -1){
            for(unsigned int a = 0; a < queue.size(); a++){
                queue[a].wait_();
            }
        }
        time += 1;
    }
    printf("%dms : simulator ended\n", time);
}

//looks like processes are sorted by arrival time and process id - great!

int main(int argc, char* argv[]){
    std::vector<Process> p;
    //std::string input_file = argv[1];
    std::ifstream input(argv[1]);
    read_p(p, input);
    //done reading/creating processes
    //print processes (testing purpose)
    for(unsigned int a = 0; a < p.size(); a++){
        p[a].Printp();
    }
    //algorithms
    round_robin(p);
    return 0;
}
