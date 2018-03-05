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
    Process cpu; //the process using the cpu
    std::vector<Process> blocked;
    bool b_finish = 0;
    int process_count = p.size();
    int cb_count = 0; //cpu burst count
    int preemp = 0;
    int c_sw = 0; //count of context switch
    int t_slice = 80;
    int t_cs = 8;
    int time = -1; //time
    int cs_cd = 0; //context switch countdown
    bool cpu_in_use = false;
    bool c_swing = false; //cw status
    //while both queue and processes-on-the-way are not empty
    //(while there are unprocessed processes)
    while(p_s.size() != 0 && queue.size() != 0){
        //check if context switch is done, if yes,
        //change context switch status and reset countdown
        if(cs_cd == t_cs){
            cs_cd = 0;
            c_swing = false;
        }
        //add wait time of all processes in queue by 1
        if(time != -1){
            for(int a = 0; a < queue.size(); a++){
                queue[a].wait_();
            }
        }
        time += 1;
        //check if process(es) arrive
        (for int a = 0; a < p.size; a++){
            if(p[a].arrial_t == t){
                //add process t queue
                queue.push_back(p[a]);
                p.erase(p.begin()+a);
                //minus one because size of p decrease by 1
                a--;
            }
        }
        //now newly arrived process(es) are added to queue (if any)
        //push the first one in queue into CPU...?
        //check if cpu is in use first
        //if cpu is in use
        if(cpu_in_use){
            b_finish = cpu.run_cpu_burst(1);
        }
        //if cpu is not in use and context switch not taking place
        else if(!cpu_in_use && !c_swing){
            cpu_in_use = true;
            //push the first process in queue into cpu
            cpu = queue[0];
            //remove the process from queue
            queue.erase(queue.begin());
            //what else to do...?
        }
        //if a process has done cpu bursting...?
        if(cpu_in_use && b_finish){
            //the process is now blocked on io
            cpu.io_start();
            //move the process to blocked list
            blocked.push_back(cpu);
            //context switch begins
            c_swing = true;
            //cpu not in use
            cpu_in_use = false;
        }
        //if context switch is taking place
        if(c_swing){
            cs_cd += 1;
        }
    }
}

//looks like processes are sorted by arrival time and process id - great!

int main(int argc, char* argv[]){
    std::vector<Process> p;
    std::string input_file = argv[1];
    std::ifstream input(input_file);
    read_p(p, input);
    //done reading/creating processes
    //print processes (testing purpose)
    for(int a = 0; a < p.size(); a++){
        p[a].Printp();
    }
    //algorithms
}
