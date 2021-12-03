
"""
Convert output of parse.py into hierarchical JSON.
"""

import sys
import json
from functools import reduce

#Convert into JSON
def to_hier(df):
	hier = {}
	stack = [{
		"depth": -1000,
		"context": hier
	}]
	
	for call in df: 
		while call["depth"] <= stack[-1]["depth"]:
			stack.pop()
		context = stack[-1]["context"]
		
		depth = call["depth"]
		name = call["name"]
		time = call["time"]
		
		if name not in context.keys():
			context[name] = {
				"name": name,
				"value": time,
				"children": {}
			}
		else:
			context[name]["value"] += time
		stack.append({
			"depth": depth,
			"context": context[name]["children"]
		})
	
	return hier

#Time aggregation
def aggregate_time(df):
	time = 0
	for call in df.values(): time += call["value"]
	df = {
		"name": "root",
		"value": time,
		"children": df
	}
	return df

#######Command line arguments
if(len(sys.argv) < 3):
	print("Usage: to_hier.py [src] [dst]")
	sys.exit()
src = sys.argv[1]
dst = sys.argv[2]

#Open the JSON df from parse.py
print("Loading: " + src)
with open(src, 'r') as fp:
	df = json.load(fp)

#Convert to hier
print("Parsing")
df = to_hier(df)
df = aggregate_time(df)

#Save the JSON df
print("Saving: " + dst)
with open(dst, 'w') as fp:
	json.dump(df, fp)


