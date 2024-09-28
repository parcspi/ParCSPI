
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
            pointer_data = []
            for target in targets:
                if target:
                    loc_id, rest = target.split(':')
                    target_id, ctxs = rest.split('[')
                    ctxs = ctxs.rstrip(']').split(';')
                    # Remove empty contexts
                    ctxs = [ctx for ctx in ctxs if ctx != '']
                    pointer_data.append((loc_id, target_id, ctxs))
            pointers[pointer_id] = pointer_data
    return pointers


def parse_and_analyze(file_path, context_level):
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
                    # Form the context sequence key based on the specified level
                    for i in range(len(ctxs)+1):
                        for combination in combinations(ctxs, i):
                            context_key = tuple(combination)
                            if context_key not in context_map:
                                context_map[context_key] = set()
                            context_map[context_key].add(target_id)
            pointers[pointer_id] = {context_key: (target_set, len(target_set)) for context_key, target_set in context_map.items()}
    
    # Identify redundant contexts
    all_contexts = set()
    for data in pointers.values():
        all_contexts.update(data.keys())

    non_redundant_contexts = {}
    for context in all_contexts:
        common_target_set = None
        is_redundant = True
        for pointer, data in pointers.items():
            target_set = data.get(context, None)
            if common_target_set is None:
                common_target_set = target_set
            elif common_target_set != target_set:
                is_redundant = False
                break
        if not is_redundant:
            non_redundant_contexts[context] = common_target_set
        else:
            print(f"Redundant context: {context}")
    # Remove redundant contexts
    for pointer in pointers:
        pointers[pointer] = {ctx: pointers[pointer][ctx] for ctx in pointers[pointer] if ctx in non_redundant_contexts}

    return pointers

from itertools import combinations

# Use this function with your file path
file_path = '../test/pac_log_str_pt.txt'
parsed_data = parse_pointer_file(file_path)
# print(parsed_data)

context_level = 10  # Change this to desired context level
parsed_data = parse_and_analyze(file_path, context_level)
print(parsed_data)
