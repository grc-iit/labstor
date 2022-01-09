
#USAGE: ./parse_libs [lib_folder] [num_cols]

import os, sys
import re

num_cols = 1
library_path = sys.argv[1]
if len(sys.argv) == 3:
    num_cols = int(sys.argv[2])

libs = {}
for lib in os.listdir(library_path):
    grp = re.search("lib([a-zA-Z0-9_]+)\.so", lib)
    if grp:
        libs[grp.group(1)] = True

    grp = re.search("lib([a-zA-Z0-9_]+)\.a", lib)
    if grp:
        libs[grp.group(1)] = True
        if grp.group(1) == "spdk_bdev_delays":
            print(lib)
            exit(1);

i = 0
lib_grid = ""
for lib in libs.keys():
    i += 1
    lib_grid += f"{lib} "
    if (i % num_cols) == 0:
        lib_grid += f"\n"

print(lib_grid)