import os

for f in os.listdir("."):
    if ".canonical." in f:
        os.rename(f, f.replace(".canonical.", "."))
