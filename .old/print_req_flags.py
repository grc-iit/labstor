
number = 0x238FF00
for i in range(32):
    if number & (1<<i):
        if i == 8:
            print("__REQ_FAILFAST_DEV")
        if i == 9:
            print("__REQ_FAILFAST_TRANSPORT")
        if i == 10:
            print("__REQ_FAILFAST_DRIVER")
        if i == 11:
            print("__REQ_SYNC")
        if i == 12:
            print("__REQ_META")
        if i == 13:
            print("__REQ_PRIO")
        if i == 14:
            print("__REQ_NOMERGE")
        if i == 15:
            print("__REQ_IDLE")
        if i == 16:
            print("__REQ_INTEGRITY")
        if i == 17:
            print("__REQ_FUA")
        if i == 18:
            print("__REQ_PREFLUSH")
        if i == 19:
            print("__REQ_RAHEAD")
        if i == 20:
            print("__REQ_BACKGROUND")
        if i == 21:
            print("__REQ_NOWAIT")
        if i == 22:
            print("__REQ_NOWAIT_INLINE")
        if i == 23:
            print("__REQ_CGROUP_PUNT")
        if i == 24:
            print("__REQ_NOUNMAP")
        if i == 25:
            print("__REQ_HIPRI")
        if i == 26:
            print("__REQ_HIPRI")
        if i == 27:
            print("__REQ_DRV")
        if i == 28:
            print("__REQ_SWAP")
