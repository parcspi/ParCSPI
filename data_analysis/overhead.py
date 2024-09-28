
from itertools import combinations

def parse_pointer_file(file_path):
    pointers = {}
    with open(file_path, 'r') as file:
        for line in file:
            line = line.strip()
            if not line:
                continue
            pointer_id, contents = line.split(' { ', 1)
            contents = contents.rstrip(' }')
            targets = contents.split(' ')
            context_map = {}
            for target in targets:
                if target:
                    loc_id, rest = target.split(':')
                    target_id, ctxs = rest.split('[')
                    ctxs = ctxs.rstrip(']').split(';')
                    ctxs = [ctx for ctx in ctxs if ctx]  # Remove empty contexts
                    for i in range(0, len(ctxs) + 1):
                        for combination in combinations(ctxs, i):
                            context_key = tuple(combination)
                            if context_key not in context_map:
                                context_map[context_key] = set()
                            context_map[context_key].add(target_id)
            pointers[pointer_id] = context_map
    return pointers

def remove_redundancy(pointers):
    # Collect all unique contexts and their corresponding target sets across all pointers
    global_contexts = {}
    for pointer, contexts in pointers.items():
        for context, targets in contexts.items():
            if context not in global_contexts:
                global_contexts[context] = targets.copy()
            else:
                # Intersection to find common targets across all pointers for the same context
                global_contexts[context].intersection_update(targets)
    
    # Determine redundant contexts by checking if the global intersection matches individual pointers
    non_redundant_contexts = {}
    for context, global_targets in global_contexts.items():
        is_redundant = True
        for pointer, contexts in pointers.items():
            pointer_targets = contexts.get(context, set())
            if pointer_targets != global_targets:
                is_redundant = False
                break
        if not is_redundant:
            non_redundant_contexts[context] = global_targets

    # Filter pointers to only include non-redundant contexts
    for pointer in pointers.keys():
        pointers[pointer] = {ctx: targets for ctx, targets in pointers[pointer].items() if ctx in non_redundant_contexts}

    return pointers


def parse_log_file(file_path):
    pointer_logs = {}
    with open(file_path, 'r') as file:
        for line in file:
            line = line.strip()
            if not line:
                continue
            # print(line)
            contents = line.split(' ')
            pointer_logs[contents[0]]={}
            pointer_logs[contents[0]]["type"] = contents[1]
            pointer_logs[contents[0]]["times"] = int(contents[2])
    return pointer_logs


def compute_simulated_costs(pointers, parsed_logs, max_context_level, overhead1, overhead2,overhead3,overhead4):
    costs = {}
    costs["pointer_use"]={}
    costs["context"]={}
    costs["pointer_def"]={}
    # return 
    for pointer_id, context_data in pointers.items():
        # Get the invocation times for the pointer
        # print("EC",pointer_id,context_data)
        times = parsed_logs.get(pointer_id, {}).get('times', 0)
        # print("times",times)    
        # Initialize variables to store aggregated targets and determine max values
        aggregated_targets = set()
        max_context_length = 0
        
        level_contexts = {}
        for ctx, targets in context_data.items():
            trimmed_context = ctx[:max_context_level] if len(ctx) >= max_context_level else ctx
            if trimmed_context not in level_contexts:
                level_contexts[trimmed_context] = set(targets)
            else:
                level_contexts[trimmed_context].update(targets)
        
        # print("level_contexts",level_contexts)
        
        max_context_length=0
        all_ctxs = list(level_contexts.keys())
        if all_ctxs:
            max_context_length = max([len(ctx) for ctx in all_ctxs])
        all_pts = [targets for targets in level_contexts.values()]
        max_equivalence_class_size=0
        if all_pts:
            max_equivalence_class_size = max([len(targets) for targets in all_pts])
        
        # print("all_pts",all_pts)
        # max_context_length=max([len(ctx) for ctx in level_contexts.keys()])
        # print("max_context_length",max_context_length)
        # Calculate the cost based on the given formula
        cost = (times) * (max_context_length * overhead1[0]+overhead1[1] + max_equivalence_class_size * overhead2[0]+overhead2[1])
        costs["pointer_use"][pointer_id] = cost
    
    for id,record in parsed_logs.items():
        if record["type"] == "3": #type context
            # cost = overhead4
            cost = record["times"] * overhead3
            costs["context"][id] = cost
        elif record["type"] == "2": #type pointer def
            cost = record["times"] * overhead4
            costs["pointer_def"][id] = cost
    
    return costs

def get_base_BTI_cost(parsed_logs,bti_overhead):
    cost=0
    for id,record in parsed_logs.items():
        if record["type"] == "4": #type context
            # cost = overhead4
            cost =cost+ record["times"] #* bti_overhead
      

    return cost

def compute_simulated_costs_for_all_ctx_level(pointers, parsed_logs, max_context_level, overhead1, overhead2,overhead3,overhead4):
    total_costs = {}
    for level in range(0, max_context_level + 1):
        costs=compute_simulated_costs(pointers, parsed_logs, level, overhead1, overhead2,overhead3,overhead4)
        total_costs[level] = costs
    return total_costs

def compute_total_cost(costs):
    total_cost = 0
    for per_cost_item in costs.values():
        # print("cost",per_cost_item)
        for cost in per_cost_item.values():
            total_cost += cost
        # total_cost += cost
    return total_cost

def get_dependent_contexts(selected_pointer_id, all_pointers):
    dependent_contexts = set()
    for pointer_id in selected_pointer_id:
        for context in all_pointers[pointer_id].keys():
            for ctx in context:
                tmp = int(ctx)
                if tmp<0:
                    tmp = -tmp
                dependent_contexts.add(str(tmp))
    return dependent_contexts

if __name__ == '__main__':
    CHECK_CTX_OVERHEAD_PER_CTX = 7.3
    CHECK_OVERHEAD_PER_TARGET=2.4
    RECORD_OVERHEAD_PER_CONTEXT=5.1
    POINTER_DEF_OVERHEAD=1.2
    max_context_level=4

    # Use this function with your file path
    log_file_path = '../test/trace/output.txt1'

    pointer_file_path = '../test/pac_log_str_pt.txt'
    parsed_pointers = parse_pointer_file(pointer_file_path)
    non_redundant_pointers = remove_redundancy(parsed_pointers)
   
    parsed_logs=parse_log_file(log_file_path)


    costs=compute_simulated_costs_for_all_ctx_level(non_redundant_pointers, parsed_logs, max_context_level, CHECK_CTX_OVERHEAD_PER_CTX, CHECK_OVERHEAD_PER_TARGET,RECORD_OVERHEAD_PER_CONTEXT,POINTER_DEF_OVERHEAD)
    total_cost = compute_total_cost(costs[1])
    # print("---------------------")
    # print("non_redundant_pointers",non_redundant_pointers)
    # print("---------------------")
    # print("costs",costs)
    # print("---------------------")
    # print("total_cost",total_cost)
    
    dep_context=get_dependent_contexts(list(parsed_pointers.keys())[5:6],parsed_pointers)
    # print("---------------------")
    # print("dep_context",dep_context)