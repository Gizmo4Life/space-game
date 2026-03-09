import os, re

def check_file(filename):
    with open(filename, 'r') as f:
        content = f.read()
    
    lines = content.split('\n')
    for i, line in enumerate(lines):
        if 'registry.get<' in line:
            # simple heuristic: look for if (registry.all_of<...>(...) or valid
            # in the previous 5 lines
            context = '\n'.join(lines[max(0, i-5):i+1])
            if 'registry.all_of' not in context and 'registry.valid' not in context:
                print(f"Unsafe get in {filename}:{i+1}: {line.strip()}")

for root, dirs, files in os.walk('src'):
    for file in files:
        if file.endswith('.cpp'):
            check_file(os.path.join(root, file))

