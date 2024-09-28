
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

def compute_equivalence_classes(pointers, max_context_level):
    results = {}
    for pointer_id, contexts in pointers.items():
        pointer_results = {}
        # Handle context-free case (context_level = 0)
        if max_context_level >= 0:
            all_targets = set()
            for targets in contexts.values():
                all_targets.update(targets)
            pointer_results[0] = {
                'equivalence_class_sizes': [len(all_targets)],
                'average_size': len(all_targets),
                'full_points_to':{():all_targets}
            }
            # print("full_points_to at level 0 --  ", pointer_results[0]['full_points_to'])
        # Compute for context levels 1 through max_context_level
        for level in range(1, max_context_level + 1):
            level_contexts = {}
            for ctx, targets in contexts.items():
                trimmed_context = ctx[:level] if len(ctx) >= level else ctx
                if trimmed_context not in level_contexts:
                    level_contexts[trimmed_context] = set(targets)
                else:
                    level_contexts[trimmed_context].update(targets)
                # print("trimmed_context",trimmed_context)    
            # Compute average size of equivalence classes
            equivalence_classes = list(level_contexts.values())
            average_size = sum(len(targets) for targets in equivalence_classes) / len(equivalence_classes) if equivalence_classes else 0
            
            # print("full_points_to at level",level," --  ", level_contexts)
            # Store results for each context level
            pointer_results[level] = {
                'equivalence_class_sizes': [len(targets) for targets in equivalence_classes],
                'average_size': average_size,
                'full_points_to':level_contexts
            }
        results[pointer_id] = pointer_results
    return results

def calculate_ratio_between_levels(results, level1, level2):
    ratios = {}
    for pointer, data in results.items():
        avg_size1 = data[level1]['average_size']
        avg_size2 = data[level2]['average_size']
        ratio = avg_size1 / avg_size2 if avg_size2 != 0 else float('inf')
        ratios[pointer] = ratio
    return ratios

if __name__ == '__main__':
    # Use this function with your file path
    file_path = '../test/pac_log_str_pt.txt'
    parsed_pointers = parse_pointer_file(file_path)
    non_redundant_pointers = remove_redundancy(parsed_pointers)
    equivalence_results = compute_equivalence_classes(non_redundant_pointers, 3)
    ratio_results = calculate_ratio_between_levels(equivalence_results, 2, 0)


    print(parsed_pointers)
    print("---------------------")
    print(non_redundant_pointers)
    print("---------------------")
    print(equivalence_results)
    print("---------------------")
    print(ratio_results)