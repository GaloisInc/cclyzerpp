"""
Used for editing predicates.inc.

Should be deleted after that file is more fully-formed.
"""

from re import search

with open("predicates.inc", mode="r") as f:
    lines = f.readlines()

with open("predicates.inc", mode="w") as f:
    last_group = ""
    for line in lines:
        if line.startswith("GROUP_BEGIN("):
            last_group = search(r"GROUP_BEGIN\((.+)\)", line).group(1)
            f.write(line)
        elif line.startswith("PREDICATE("):
            p = search(r"PREDICATE\((.+)\)", line).group(1)
            f.write(f"PREDICATE({last_group}, {p})\n")
        else:
            f.write(line)
