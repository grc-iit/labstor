
"""
Convert output of parse.py into hierarchical JSON.
"""

import sys
import json

#Convert into JSON
def prune(df, thresh, parent):
	for call in list(df.values()):
		if parent:
			if call["value"]/parent["value"] < thresh:
				del parent["children"][call["name"]]
				continue
		if call["children"]:
			prune(call["children"], thresh, call)

#######Command line arguments
if(len(sys.argv) < 4):
	print("Usage: prune.py [src] [dst] [thresh]")
	sys.exit()
src = sys.argv[1]
dst = sys.argv[2]
thresh = float(sys.argv[3])

#Open the JSON df from to_hier.py
print("Loading: " + src)
with open(src, 'r') as fp:
	df = json.load(fp)

#Prune
print("Pruning")
prune(df["children"], thresh, df)

#Save the JSON df
print("Saving: " + dst)
with open(dst, 'w') as fp:
	json.dump(df, fp, indent=4, separators=(',', ': '))


