import sec_score
import overhead
import numpy as np
import sys
import json
BIGNUM=1000000000



def compute_score_cost_ratio(pointers, costs, equivalence_class_results,max_context_level):
    # print('equivalence_class_results',equivalence_class_results)
    # return
    # print('gradual_selection',costs.keys(),equivalence_class_results.keys())
    
    benit_costs_ratios=None
    score_cost_ratio={}
    sec_scores_all_contexts={}
    costs_transformed={}
    for pid in equivalence_class_results.keys():
        sec_scores_all_contexts[pid]={}
        costs_transformed[pid]={}
        for ctx_level in range(max_context_level+1):
            # print('ctx_level',ctx_level)

            if ctx_level==0:
                if equivalence_class_results[pid][max_context_level]["average_size"]==0:
                    sec_scores_all_contexts[pid][ctx_level]=1
                else:
                    sec_scores_all_contexts[pid][ctx_level]=equivalence_class_results[pid][max_context_level]["average_size"]/equivalence_class_results[pid][ctx_level]["average_size"]
                
                costs_transformed[pid][ctx_level]=costs[ctx_level]['pointer_use'][pid]
            else:
                costs_transformed[pid][ctx_level]=costs[ctx_level]['pointer_use'][pid]#-costs[ctx_level-1]['pointer_use'][pid]
                if equivalence_class_results[pid][max_context_level]["average_size"]==0:
                    sec_scores_all_contexts[pid][ctx_level]=1
                else:
                    sec_scores_all_contexts[pid][ctx_level]=equivalence_class_results[pid][max_context_level]["average_size"]/equivalence_class_results[pid][ctx_level]["average_size"]
    score_cost_ratio["score"]=sec_scores_all_contexts        
    score_cost_ratio["cost"]=costs_transformed
    # print('score_cost_ratio',score_cost_ratio)
    
    # return
    score_cost_incremental={}
    # score_cost_incremental["score"]={}  
    # score_cost_incremental["cost"]={}
    for pointers in score_cost_ratio["score"].keys():
        score_cost_incremental[pointers]={}
        for ctx_level in range(max_context_level+1):
            score_cost_incremental[pointers][ctx_level]={}
            score_cost_incremental[pointers][ctx_level]["ori_score"]=score_cost_ratio["score"][pointers][ctx_level]
            score_cost_incremental[pointers][ctx_level]["ori_cost"]=score_cost_ratio["cost"][pointers][ctx_level]
            
            if ctx_level==0:
                
                score_cost_incremental[pointers][ctx_level]["score"]=score_cost_ratio["score"][pointers][ctx_level]
                score_cost_incremental[pointers][ctx_level]["cost"]=score_cost_ratio["cost"][pointers][ctx_level]
                
                # score_cost_incremental[pointers][ctx_level]["score"]=score_cost_ratio["score"][pointers][ctx_level]
            else:
                # print(ctx_level-1,ctx_level,pointers,score_cost_ratio["score"][pointers])
                score_cost_incremental[pointers][ctx_level]={}
                
                
                score_cost_incremental[pointers][ctx_level]["score"]=score_cost_ratio["score"][pointers][ctx_level]-score_cost_ratio["score"][pointers][ctx_level-1]
                score_cost_incremental[pointers][ctx_level]["cost"]=score_cost_ratio["cost"][pointers][ctx_level]-score_cost_ratio["cost"][pointers][ctx_level-1]
                # score_cost_incremental[pointers][ctx_level]["score"]=score_cost_ratio["score"][pointers][ctx_level]
            if score_cost_incremental[pointers][ctx_level]["cost"]==0 and score_cost_incremental[pointers][ctx_level]["score"]==0:
                score_cost_incremental[pointers][ctx_level]["score_cost_rate"]=0    
            elif score_cost_incremental[pointers][ctx_level]["cost"]==0:
                score_cost_incremental[pointers][ctx_level]["score_cost_rate"]=BIGNUM
            else: 
                score_cost_incremental[pointers][ctx_level]["score_cost_rate"]=score_cost_incremental[pointers][ctx_level]["score"]/score_cost_incremental[pointers][ctx_level]["cost"]
            # print("ec keys: ", equivalence_class_results[pointers][ctx_level].keys())
            score_cost_incremental[pointers][ctx_level]['full_points_to']=equivalence_class_results[pointers][ctx_level]['full_points_to']
    # print('score_cost_incremental',score_cost_incremental)
    return score_cost_incremental

def generate_cost_score_line(score_cost_incremental,target_sec_core=0.5):
    
    optimal_policy={}
    optimal_policy['points_to']={}
    optimal_policy['context']=set()
    optimal_policy['pointer_def']=set()
    optimal_policy['pointer_use']=set()

    selected_maps={}
    for pointers in score_cost_incremental.keys():
        selected_maps[pointers]={}
        for ctx_level in score_cost_incremental[pointers].keys():
            selected_maps[pointers][ctx_level]=0
            # print(pointers,ctx_level,score_cost_incremental[pointers][ctx_level]["score"],score_cost_incremental[pointers][ctx_level]["cost"],score_cost_incremental[pointers][ctx_level]["score_cost_rate"])
    
    
    def select_next(score_cost_incremental_,selected_maps_):
        max_score_cost_rate_pointer=None
        max_score_cost_rate_ctx=None
        max_score_cost_rate=-1
        max_points_to=None
       
        for pointers in selected_maps_.keys():
            for ctx_level in sorted(selected_maps_[pointers].keys()):
                if selected_maps_[pointers][ctx_level]==0:
                    if score_cost_incremental_[pointers][ctx_level]["score_cost_rate"]>max_score_cost_rate:
                        max_score_cost_rate=score_cost_incremental_[pointers][ctx_level]["score_cost_rate"]
                        max_score_cost_rate_pointer=pointers
                        max_score_cost_rate_ctx=ctx_level
                        max_points_to=score_cost_incremental_[pointers][ctx_level]['full_points_to']
                    break
        return max_score_cost_rate_pointer,max_score_cost_rate_ctx,max_score_cost_rate,max_points_to
    
    cost_score_pairs=[]
    
    current_cost=0
    current_score=0
    while 1:
        max_score_cost_rate_pointer,max_score_cost_rate_ctx,max_score_cost_rate,max_points_to=select_next(score_cost_incremental,selected_maps)
        if max_score_cost_rate_pointer is None:
            break
        
        if current_score<=target_sec_core*len(selected_maps.keys()):
            
            
            optimal_policy['pointer_use'].add(max_score_cost_rate_pointer)
            optimal_policy['points_to'][max_score_cost_rate_pointer]={}
            optimal_policy['points_to'][max_score_cost_rate_pointer]['check_value']=set()
            optimal_policy['points_to'][max_score_cost_rate_pointer]['ctx_level']=max_score_cost_rate_ctx
            hash_ctx=0
            for _ctxs in max_points_to:
                for _ctx in _ctxs:
                    hash_ctx=hash_ctx^abs(int(_ctx))
                    optimal_policy['context'].add(str(abs(int(_ctx)))) 
                
                for _t in max_points_to[_ctxs]:
                    backup_hash=hash_ctx
                    optimal_policy['pointer_def'].add(_t)
                    backup_hash=backup_hash^abs(int(_t))
                    optimal_policy['points_to'][max_score_cost_rate_pointer]['check_value'].add(backup_hash)
                    
                # optimal_policy['points_to'][max_score_cost_rate_pointer]=list(optimal_policy['points_to'][max_score_cost_rate_pointer])
                
            # optimal_policy['points_to'][max_score_cost_rate_pointer]=max_points_to        
        current_cost+=score_cost_incremental[max_score_cost_rate_pointer][max_score_cost_rate_ctx]['cost']
        current_score+=score_cost_incremental[max_score_cost_rate_pointer][max_score_cost_rate_ctx]['score']
        cost_score_pairs.append((current_cost,current_score))
        selected_maps[max_score_cost_rate_pointer][max_score_cost_rate_ctx]=1
        

       
        # print(max_score_cost_rate_pointer,max_score_cost_rate_ctx,max_score_cost_rate)
    # print('cost_score_pairs',cost_score_pairs)
    # print('optimal_policy',optimal_policy)
    optimal_policy['pointer_use']=list(optimal_policy['pointer_use'])
    optimal_policy['context']=list(optimal_policy['context'])
    optimal_policy['pointer_def']=list(optimal_policy['pointer_def'])
    for _p in optimal_policy['points_to'].keys():
        optimal_policy['points_to'][_p]['check_value']=list(optimal_policy['points_to'][_p]['check_value'])
    return cost_score_pairs,len(selected_maps.keys()),optimal_policy
    
if __name__ == "__main__":
    # print("Hello, World!")
    # if len(sys.argv) != 3:
    #     print("Please provide two arguments.")
    #     exit(0)
   
    pointer_file_path = sys.argv[1]
    log_file_path = sys.argv[2]
    
    target_sec_core=1
    policy_output_path="./optimal_policy.txt"
    if len(sys.argv)>3:
        timepath=sys.argv[3]
        time_data = np.loadtxt(timepath)
        real_runtime = float(time_data)
        if len(sys.argv)>=6:
            target_sec_core=float(sys.argv[4])
            policy_output_path=sys.argv[5]
         
        # print("real_runtime",real_runtime)
    else:
        real_runtime = 1
    # pointer_file_path = '../test/pac_log_str_pt.txt'
    # log_file_path = '../test/trace/output.txt1'    
    max_context_level=4
    
    CHECK_CTX_OVERHEAD_PER_CTX =21/2#59
    CHECK_CTX_OVERHEAD_BASE = CHECK_CTX_OVERHEAD_PER_CTX/3
    
    CHECK_OVERHEAD_PER_TARGET=30/2#32
    CHECK_OVERHEAD_BASE=CHECK_OVERHEAD_PER_TARGET/4
    
    RECORD_OVERHEAD_PER_CONTEXT=CHECK_CTX_OVERHEAD_PER_CTX#CHECK_OVERHEAD_PER_TARGET
    POINTER_DEF_OVERHEAD=CHECK_CTX_OVERHEAD_PER_CTX/4#CHECK_OVERHEAD_PER_TARGET*3
    

    
    parsed_pointers = sec_score.parse_pointer_file(pointer_file_path)
    non_redundant_pointers = sec_score.remove_redundancy(parsed_pointers)
    equivalence_results = sec_score.compute_equivalence_classes(non_redundant_pointers, max_context_level)
    # ratio_results = sec_score.calculate_ratio_between_levels(equivalence_results, 2, 0)
    
    
    parsed_logs=overhead.parse_log_file(log_file_path)
    
    # print('parsed_logs',parsed_logs)
    BTI_CONST_COST=1
    bti_cost=overhead.get_base_BTI_cost(parsed_logs,BTI_CONST_COST)
    # print('bti_cost',bti_cost/real_runtime,BTI_CONST_COST,real_runtime)
    with open('./bti_cost_data.txt','w') as f:
        f.write(f'{bti_cost/real_runtime}\n')
    costs=overhead.compute_simulated_costs_for_all_ctx_level(non_redundant_pointers, parsed_logs, max_context_level, (CHECK_CTX_OVERHEAD_PER_CTX,CHECK_CTX_OVERHEAD_BASE), (CHECK_OVERHEAD_PER_TARGET,CHECK_OVERHEAD_BASE),RECORD_OVERHEAD_PER_CONTEXT,POINTER_DEF_OVERHEAD)
    
    
    # print('parsed_pointers',parsed_pointers)
    # print("---------------------")
    # print('non_redundant_pointers',non_redundant_pointers)
    # print("---------------------")
    # print('equivalence_results',equivalence_results)
    # print("---------------------")
    # print('costs',costs)
    # print("---------------------")
    score_cost_incremental=compute_score_cost_ratio(non_redundant_pointers, costs, equivalence_results,max_context_level)
    cost_score_pairs,num_pointers,optimal_policy=generate_cost_score_line(score_cost_incremental,target_sec_core)
    def save_optimal_policy_to_json(optimal_policy, policy_output_path):
        with open(policy_output_path, 'w') as f:
            json.dump(optimal_policy, f, indent=4)

    save_optimal_policy_to_json(optimal_policy, policy_output_path)
    import matplotlib.pyplot as plt

    def plot_score_cost_increment(_cost_score_pairs,_num_pointers,_real_runtime):
        costs, scores = zip(*_cost_score_pairs)
        plt.plot(np.array(costs)/_real_runtime, np.array(scores)/_num_pointers)
        plt.xlabel('Cost (WCET overhead = cost/origi_runtime) (Note: this is not % number)')
        plt.ylabel('Score')
        plt.title('Score vs Cost')
        plt.savefig('./score_cost_plot.png')
    def save_score_cost_increment(_cost_score_pairs,_num_pointers,_real_runtime):
        costs, scores = zip(*_cost_score_pairs)
        # print(type(costs),costs)
        costs=(0,)+costs
        scores=(0,)+scores
        with open('./cost_score_data.txt','w') as f:
            for i in range(len(costs)):
                f.write(f'{costs[i]/_real_runtime}\t{scores[i]/_num_pointers}\n')
    plot_score_cost_increment(cost_score_pairs,num_pointers,real_runtime)
    save_score_cost_increment(cost_score_pairs,num_pointers,real_runtime)
    
    # print("real_runtime",real_runtime)