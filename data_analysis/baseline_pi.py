import sec_score
import overhead
import numpy as np
import sys
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
    # print('score_cost_incremental',score_cost_incremental)
    return score_cost_incremental

def generate_cost_score_line(score_cost_incremental):
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
        for pointers in selected_maps_.keys():
            for ctx_level in sorted(selected_maps_[pointers].keys()):
                if selected_maps_[pointers][ctx_level]==0:
                    if score_cost_incremental_[pointers][ctx_level]["score_cost_rate"]>max_score_cost_rate:
                        max_score_cost_rate=score_cost_incremental_[pointers][ctx_level]["score_cost_rate"]
                        max_score_cost_rate_pointer=pointers
                        max_score_cost_rate_ctx=ctx_level
                    break
        return max_score_cost_rate_pointer,max_score_cost_rate_ctx,max_score_cost_rate
    
    cost_score_pairs=[]
    
    current_cost=0
    current_score=0
    while 1:
        max_score_cost_rate_pointer,max_score_cost_rate_ctx,max_score_cost_rate=select_next(score_cost_incremental,selected_maps)
        if max_score_cost_rate_pointer is None:
            break
        current_cost+=score_cost_incremental[max_score_cost_rate_pointer][max_score_cost_rate_ctx]['cost']
        current_score+=score_cost_incremental[max_score_cost_rate_pointer][max_score_cost_rate_ctx]['score']
        cost_score_pairs.append((current_cost,current_score))
        selected_maps[max_score_cost_rate_pointer][max_score_cost_rate_ctx]=1
        # print(max_score_cost_rate_pointer,max_score_cost_rate_ctx,max_score_cost_rate)
    # print('cost_score_pairs',cost_score_pairs)
    return cost_score_pairs,len(selected_maps.keys())
def get_baseline_sec_core(pointers,parsed_logs,equivalence_results,max_context_level):
    # return
    # print("get_baseline_sec_core")
    # print("pointers",pointers)
    # print("parsed_logs",parsed_logs)
    # print("equivalence_results",equivalence_results)
    # return
    num_pointers=len(pointers.keys())
    global_data_pointer_score=0
    global_data_pointer_target=0
    for pointer in pointers.keys():
        if pointer in parsed_logs.keys():
            if parsed_logs[pointer]["type"]=="4":
                # global_data_pointer_score=global_data_pointer_score+1
                # global_data_pointer_score=global_data_pointer_score+1
                continue
        num_target=equivalence_results[pointer][0]["average_size"]
        global_data_pointer_target=global_data_pointer_target+num_target
                # print("pointer",pointer)
    for pointer in pointers.keys():
        if pointer in parsed_logs.keys():
            if parsed_logs[pointer]["type"]=="4":
                global_data_pointer_score=global_data_pointer_score+1
                # print("find return")
                # global_data_pointer_score=global_data_pointer_score+1
                continue
        num_target_fullcontext=equivalence_results[pointer][max_context_level]["average_size"]
        if global_data_pointer_target==0:
            global_data_pointer_score=global_data_pointer_score+0
        else:
            global_data_pointer_score=global_data_pointer_score+num_target_fullcontext/global_data_pointer_target
        
    final_score=0
    if  num_pointers>0:
        final_score=global_data_pointer_score/num_pointers
    else:
        final_score=0
    return final_score
    # print("final score: ",final_score)
        # global_data_pointer_target=global_data_pointer_target+num_target
                # print("pointer",pointer)
    
                # print("parsed_logs[pointer]",parsed_logs[pointer])
    # def get_pointer_score(pointer,pointers, parsed_logs):
    #     if pointer in 
    #     pointer["type"]=="1":
    #         return True
    #     return False
    pass

if __name__ == "__main__":
    # print("Hello, World!")
    # if len(sys.argv) != 3:
    #     print("Please provide two arguments.")
    #     exit(0)
   
    pointer_file_path = sys.argv[1]
    log_file_path = sys.argv[2]
    
    if len(sys.argv)>3:
        timepath=sys.argv[3]
        time_data = np.loadtxt(timepath)
        real_runtime = float(time_data)
        # print("real_runtime",real_runtime)
    else:
        real_runtime = 1
    # pointer_file_path = '../test/pac_log_str_pt.txt'
    # log_file_path = '../test/trace/output.txt1'    
    max_context_level=4
    
    # the timing value obtained from WCET analysis tool ait-timeweaver
    _CHECK_CTX_OVERHEAD_PER_CTX =21/2
    CHECK_CTX_OVERHEAD_PER_CTX =_CHECK_CTX_OVERHEAD_PER_CTX*0#59
    
    CHECK_CTX_OVERHEAD_BASE = CHECK_CTX_OVERHEAD_PER_CTX/3
    
    _CHECK_OVERHEAD_PER_TARGET=30/2#32
    CHECK_OVERHEAD_PER_TARGET=_CHECK_OVERHEAD_PER_TARGET*0#32
    CHECK_OVERHEAD_BASE=_CHECK_OVERHEAD_PER_TARGET/4
    
    RECORD_OVERHEAD_PER_CONTEXT=_CHECK_CTX_OVERHEAD_PER_CTX/3#CHECK_OVERHEAD_PER_TARGET
    POINTER_DEF_OVERHEAD=_CHECK_CTX_OVERHEAD_PER_CTX/4#CHECK_OVERHEAD_PER_TARGET*3
    

    
    parsed_pointers = sec_score.parse_pointer_file(pointer_file_path)
    non_redundant_pointers = sec_score.remove_redundancy(parsed_pointers)
    equivalence_results = sec_score.compute_equivalence_classes(non_redundant_pointers, max_context_level)
    # ratio_results = sec_score.calculate_ratio_between_levels(equivalence_results, 2, 0)
    
    
    parsed_logs=overhead.parse_log_file(log_file_path)


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
    cost_score_pairs,num_pointers=generate_cost_score_line(score_cost_incremental)
    
    import matplotlib.pyplot as plt

    def plot_score_cost_increment(_cost_score_pairs,_num_pointers,_real_runtime):
        costs, scores = zip(*_cost_score_pairs)
        plt.plot(np.array(costs)/_real_runtime, np.array(scores)/_num_pointers)
        # plt.xlabel('Cost (WCET overhead = cost/origi_runtime) (Note: this is not % number)')
        # plt.ylabel('Score')
        # plt.title('Score vs Cost')
        # plt.savefig('./score_cost_plot.png')
    def save_score_cost_increment(_cost_score_pairs,_num_pointers,_real_runtime,_score):
        costs, scores = zip(*_cost_score_pairs)
        print(type(costs),costs)
        costs=(0,)+costs
        scores=(0,)+scores
        with open('./cost_score_data.txt','w') as f:
            # for i in range(len(costs)):
            f.write(f'{costs[-1]/_real_runtime}\t{_score}\n')
    plot_score_cost_increment(cost_score_pairs,num_pointers,real_runtime)
    score=get_baseline_sec_core(non_redundant_pointers,parsed_logs,equivalence_results,max_context_level)
    save_score_cost_increment(cost_score_pairs,num_pointers,real_runtime,score)
    
    # print("real_runtime",real_runtime)
    
