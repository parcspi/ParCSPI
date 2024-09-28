/*

The actual runtime impact of PA instructions should be simulated according to different hardware.

*/

#include <stdlib.h>
 #include <stdio.h>
#include <vector>
#include <map>
#include <fstream>
extern "C" {
    void * pac_pointer_check(int id, void* ptr, int* table,int table_length, int ctx_length);
    void * pac_pointer_def(int id, void* ptr);
    void pac_context_pop();
    void pac_context_record(int id);
    void pac_log_start();
    void pac_log_end();
    // void pac_def(int id);
}
int pac_start=0;
int iteration=0;
 std::map<int,std::pair<int,int>> pac_log_map; 
 #define PAC_CONTEXT_SIZE 1024
int pac_context_stack[PAC_CONTEXT_SIZE];
int pac_context_top=-1;
std::map<void *,int> pac_modifier_map; //ptr to modifier

 void * pac_pointer_check(int id, void* ptr, int* table,int table_length, int ctx_length){
    if(pac_start==0){
        return ptr;
    }
    // printf("pac_pointer_check %d %x\n",id,ptr);
    int hash_val=0;
    
    for(int i=0;i<ctx_length &&pac_context_top-i>=0;i++){
        hash_val^=pac_context_stack[pac_context_top-i];
    }
    if(pac_modifier_map.find(ptr) != pac_modifier_map.end()){
        hash_val=pac_modifier_map[ptr];
    }
    for(int i=0;i<table_length;i++){
        int target_hash=table[i];
        
        if(target_hash==hash_val){
             
            return ptr;
        }

    }
   
    return NULL; //Pointer integrity violation )

 }
    void * pac_pointer_def(int id, void* ptr){
        if(pac_start==0){
            return ptr;
        }
       
        pac_modifier_map[ptr]=id;
        
        return ptr;
    }
    void pac_context_pop(){
        if(pac_start==0){
            return ;
        }
        if(pac_context_top>=0){
            pac_context_top--;
        }
       

    }
    void pac_context_record(int id){
        if(pac_start==0){
            return ;
        }
        if(pac_context_top<PAC_CONTEXT_SIZE-1){
            pac_context_top++;
            pac_context_stack[pac_context_top]=id;
        }
 
    }


 


    void pac_log_start(){
        pac_start=1;
        iteration++;
        printf("pac check start\n");
    }

    void pac_log_end(){
        pac_start=0;
        
         printf("pac check end\n");
    }