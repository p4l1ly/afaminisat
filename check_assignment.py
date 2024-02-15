import sys

asgns = \
    [ -1
    , 1
    , 1
    , 1
    , 1
    , -1
    , 1
    , 1
    , -1
    , 1
    , 1
    , 1
    , 1
    , 1
    , -1
    , -1
    , 1
    , -1
    , 1
    , -1
    , -1
    , 1
    , -1
    , -1
    , 1
    , 0
    , 1
    , 1
    , -1
    , 1
    , 1
    , -1
    , 1
    , 1
    , 1
    , 1
    , 1
    , 1
    , 1
    , 1
    , 1
    , -1
    , 1
    , -1
    , 1
    ]

next(sys.stdin)
for i, line in enumerate(sys.stdin):
    clause = [int(x) for x in line.strip().split()]
    for lit in clause:
        if lit < 0:
            if asgns[-lit - 1] in (-1, 0):
                break
        elif lit > 0:
            if asgns[lit - 1] in (1, 0):
                break
    else:
        print(i, line, end="")