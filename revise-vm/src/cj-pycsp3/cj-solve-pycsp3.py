#!/usr/bin/env python3
import argparse
import json
import math
import sys
import time
from pycsp3 import *


parser = argparse.ArgumentParser(
                    prog='cj-solve-pycsp3',
                    description='Solve CSP JSON instances.')
parser.add_argument('--csp', type=str, required=True)


if __name__ == '__main__':
    args = parser.parse_args()
    csp_json = None
    with open(args.csp) as f:
        csp_json = json.loads(f.read())

    def domfn(i):
        return csp_json['domains'][csp_json['vars'][i]]['values']
    x = VarArray(size=len(csp_json['vars']), dom=domfn)

    tables = []
    for constraintDef in csp_json['constraintDefs']:
        tables.append(constraintDef['noGoods'])

    for constraint in csp_json['constraints']:
        cid = constraint['id']
        cvars = constraint['vars']
        satisfy([x[i] for i in cvars] not in tables[cid]) # TODO in/not-in depends on constraintDef

    time_start = time.time_ns()
    instance = compile()
    ace = solver(ACE)
    ace_opts = '-varh=Lexico -valh=First -ng=NO -prepro=false -branching=NON -r_c=99999999999'
    #ace_opts = '-varh=DdegOnDom -valh=First -ng=NO -prepro=false -branching=NON -r_c=99999999999'
    result = ace.solve(instance, dict_options={"args": ace_opts}, verbose=0)
    time_end = time.time_ns()
    time_ms = math.floor((time_end - time_start) / 1000000)
    solution = None
    if result is SAT:
        solution = values(x)
    print(json.dumps({"solution": solution, "timeMs": time_ms}))
    sys.exit(0)
