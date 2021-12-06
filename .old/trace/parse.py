
"""
Converts trace-cmd output data into a machine-readable format

[depth][name][time][start_trace]

oc_v_fs-11709 [003] 59054.145040: print:                tracing_mark_write: START_TRACE
oc_v_fs-11709 [003] 59054.142383: funcgraph_entry:                   |         security_file_open() {
oc_v_fs-11709 [003] 59054.145048: funcgraph_exit:       + 13.517 us  |         }
oc_v_fs-11709 [003] 59054.145048: funcgraph_exit:         9.444 us   |         }
oc_v_fs-11709 [003] 59054.145044: funcgraph_entry:        0.534 us   |         fsnotify();
oc_v_fs-11709 [003] 59054.145044: funcgraph_entry:      + 11.534 us  |         fsnotify();
"""

import sys
import re
import json

fname_pattern = re.compile("^([a-zA-Z0-9_]+)")

#Get the system call name and execution time when entering
def funcgraph_entry(df, tokens, i0, stack, start_trace):
	time = 0
	for i in range(i0, len(tokens)):
		token = tokens[i]
		
        #Function name
		if token[0] == "|":
			fname = re.search(fname_pattern, tokens[i+1][0])
			fname = fname.group(0)
			
			df.append({
				"depth": int(len(token[1])/2 - 1),
				"name": fname,
				"time": time,
				"start_trace": start_trace
			})
			stack = stack[0:df[-1]["depth"]]
			stack += [None for i in range(len(stack), df[-1]["depth"])]
			if i+2 < len(tokens) and tokens[i+2][0] == "{":
				stack.append(df[-1]) 
			return (i+1, stack)

        #Function time
		else:
			try:
				time = float(token[0])
			except:
				pass

	return (len(tokens), stack)

#Get the execution time when exiting
def funcgraph_exit(tokens, i0, stack):
	if len(stack) == 0:
		return i0
	time = 0
	for i in range(i0, len(tokens)):
		token = tokens[i]

        #Function name
		if token[0] == "|":
			depth = int(len(token[1])/2 - 1)
			if depth >= len(stack):
				return i
			call = stack[depth]
			stack = stack[0:depth]
			if call:
				call["time"] = time
			return i

        #Function time
		if time == 0:
			try:
				time = float(token[0])
			except:
				pass

	return len(tokens)

#Parse function graph
def parse_syscalls(tokens):
	df = []
	stack = []
	i = 0
	start_trace=True ####If I actually define critical regions, mark False

	while i < len(tokens):
		token = tokens[i] 
		if token[0] == "funcgraph_entry:":
			(i, stack) = funcgraph_entry(df, tokens, i+1, stack, start_trace)
		elif token[0] == "funcgraph_exit:":
			i = funcgraph_exit(tokens, i+1, stack)
			if len(stack) == 0 and in_syscall:
				in_syscall=False
		elif token[0] == "tracing_mark_write:":
			start_trace = not start_trace
		i += 1
	
	return df

#Remove any calls that are not apart of the trace
def filter_calls(df):
	i = 0
	outer_call = 0
	while i < len(df):
		call = df[i]
		if call["depth"] == 0:
			outer_call = i
		if not call["start_trace"]:
			df.pop(outer_call)
			while outer_call < len(df) and df[outer_call]["depth"] > 0:
				df.pop(outer_call)
			i -= 1
		i += 1
	return df

#######Command line arguments
if(len(sys.argv) < 3):
	print("Usage: parse.py [src] [dst]")
	sys.exit()
src = sys.argv[1]
dst = sys.argv[2]

#Load the data
print("Loading: " + src)
file = open(src, mode="r")
data = file.read()
file.close()

#Tokenize the data
print("Tokenizing")
data = re.findall("([^\s]+)(\s+)", data) 

#Parse system calls
print("Parsing")
df = parse_syscalls(data) 
#df = filter_calls(df)

#Save the JSON df
print("Saving: " + dst)
with open(dst, 'w') as fp:
	json.dump(df, fp, indent=4, separators=(',', ': '))


