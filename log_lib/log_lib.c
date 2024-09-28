#include <stdlib.h>
 #include <stdio.h>
#include <vector>
#include <map>
#include <fstream>
extern "C" {
    void pac_log_function(int type,int id);
    // void pac_log_function(int type,int id,void* ptr);
    void* pac_log_function_ptr(int type,int id,void* ptr);
    void pac_log_start();
    void pac_log_end();
    // void pac_def(int id);
}
int pac_start=0;
int iteration=0;
 std::map<int,std::pair<int,int>> pac_log_map; 
 void* pac_log_function_ptr(int type,int id,void* ptr){
    // printf("pac_log_function %d %d %x\n",type,id,ptr);
    pac_log_function(type,id);
    return ptr;
 }
//  void pac_log_function(int type,int id,void* ptr){
//     pac_log_function(type,id);
//  }
 void pac_log_function(int type,int id){
    // printf("pac_log_function %d %d --\n",type,id);
    if(pac_start==0){
        return;
    }

    // printf("pac_log_function %d %d\n",type,id);
    if(pac_log_map.find(id) == pac_log_map.end()){
        
        pac_log_map.insert(std::make_pair(id,std::make_pair(type,0)));
    }
    
        pac_log_map[id].second++;
    
    
 }

    void pac_log_start(){
        pac_start=1;
        iteration++;
        pac_log_map.clear();
    }

    void pac_log_end(){
        pac_start=0;
        {
         std::ofstream outFile("trace/output.txt"+std::to_string(iteration));
        
        // Check if file is open
        if (!outFile.is_open()) {
            // std::cerr << "Error: Unable to open file!" << std::endl;
            return ;
        }

        // Iterate over the map and write each key-value pair to the file
        for (const auto& entry : pac_log_map) {
            outFile << entry.first << " " << entry.second.first << " " << entry.second.second << std::endl;
        }

        // Close the file
        outFile.close();
        }
        
    }