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
    int cs_cd = (t_cs/2) + 1; //context switch countdown
    int cb_remain = t_slice;
    //default is this because first process takes t_cs/2 ms to get
    //to cpu
    bool cpu_in_use = false;
    bool c_swing = true; //cw status
    bool force_quit = false;
    //(while there are unprocessed processes)
    while(finished.size() != process_count){
	if(time == 200){break;}
        //check if process(es) arrive
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
        //now newly arrived process(es) are added to queue (if any)
        //check if first half of cs is done
        if(cs_cd == 4){
            //check if queue is empty
            if(queue.size() == 0){
                //skip cs
                cs_cd = 0;
                c_swing = false;
            }
            printf("%dms : process %c enters blocked state\n", time, cs_out[0].p_id());
            blocked.push_back(cs_out[0]);
            cs_out.erase(cs_out.begin());
        }
        //second half begins
        if(cs_cd == 5){
            printf("%dms : process %c begins entering cpu (enters context switch)\n", time, queue[0].p_id());
            cs_in.push_back(queue[0]);
            queue.erase(queue.begin());
        }
        //check if context switch is done, if yes,
        //change context switch status and reset countdown
        if(cs_cd == t_cs + 1){
            printf("%dms : context switch done\n", time);
            cs_cd = 0;
            c_swing = false;
        }
        
        //push the first one in queue into CPU...?
        //check if cpu is in use first
        //if cpu is in use
        if(cpu_in_use){
            cb_remain -= 1;
            b_finish = cpu.run_cpu_burst(1);
        }
        //if cpu is not in use and context switch not taking place
        else if(!cpu_in_use && !c_swing){
            printf("%dms : process %c starts cpu burst\n", time, cs_in[0].p_id());
	    b_finish = false;
            cpu_in_use = true;
            cpu = cs_in[0];
            cs_in.erase(cs_in.begin());
            //what else to do...?
        }
        //if timeout
        if(cb_remain == 0){
            //force finish
            printf("%dms : process %c finishes cpu burst due to timeout\n", time, cpu.p_id());
            force_quit = true;
            cb_remain = t_slice;
            b_finish = true;
        }
        //if a process has done cpu bursting...?
        if(cpu_in_use && b_finish){
            if(!force_quit){
                printf("%dms : process %c finished cpu burst normally\n", time, cpu.p_id());
                force_quit = false;
            }
            //move the process to cs list
            //cpu.set_cs_remain(t_cs/2);
            printf("here1?\n");
            cs_out.push_back(cpu);
            //context switch begins
            printf("here2?\n");
            c_swing = true;
            //cpu not in use
            cpu_in_use = false;
            
        }
        //if context switch is taking place
        if(c_swing){
            cs_cd += 1;
        }
        //io time
        if(blocked.size() != 0){
            for(unsigned int a = 0; a < blocked.size(); a++){
                //do io
                blocked[a].io_();
                //remove any process that has done its io from the blocked list
                if(blocked[a].io_done()){
                    if(blocked[a].check_done()){
                        //if this process is completely done
                        finished.push_back(blocked[a]);
                        blocked.erase(blocked.begin() + a);
                    }
                    else{
                        //else push back to queue
                        queue.push_back(blocked[a]);
                        blocked.erase(blocked.begin() + a);
                    }
                    a--;
                }
            }
        }
        //add wait time of all processes in queue by 1
        if(time != -1){
            for(unsigned int a = 0; a < queue.size(); a++){
                queue[a].wait_();
            }
        }
        time += 1;
    }
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
