
import subprocess
import argparse
parser = argparse.ArgumentParser()
parser.add_argument('parts',metavar='parts',type=str,nargs='+',
        help='list of parts. Valid parts: (zcu104,pynq_z2)')

OPERATIONS = [(0,"add"),(1,"mul"),(2,"div")]
TYPES = [(0,"double"),(1,"float"),(2,"int32")]

args = parser.parse_args()
PARTs = args.parts
print("User parts:",PARTs)

OP_CODE = 0
TYPE_CODE = 0

for ID_FACTOR in [1,2]:
    for PART in PARTs:
        if PART == "zcu104":
            if ID_FACTOR == 1:
                DD_configs = [(1.75,7),(1.5,8),(1.5,12),(1.5,16),(1.65,32)]
            else:
                DD_configs = [(1.75,7),(1.5,8),(1.5,12),(1.65,16),(1.7,32)]
        elif PART == "pynq_z2":
            if ID_FACTOR == 1:
                DD_configs = [(5,8),(5.2,12),(5.5,16),(6,32)]
            else:
                DD_configs =  [(5.5,8),(5.5,12),(5.5,16),(6.4,32)]
        else:
            assert False, "PART(%s) not supported" %PART

        for target_period,DD in DD_configs:
            logic_latency = DD-1

            cmd = ["make"]
            cmd += ["profiling_steps"]
            cmd += ["PART=" + PART]
            cmd += ["HLS_PERIOD=%4.2f" % target_period]
            cmd += ["IMPL_PERIOD=%4.2f" % target_period]
            cmd += ["DEP_DIST=%d" % DD]
            cmd += ["OP_CODE=%d" % OP_CODE]
            cmd += ["DATATYPE=%d" % TYPE_CODE]
            cmd += ["ID_FACTOR=%d" % ID_FACTOR]

            print("Running command:", cmd)
            process = subprocess.run(cmd,stderr=subprocess.PIPE,stdout=subprocess.PIPE)

            if process.returncode != 0:
                    print("*****************************")
                    print("Returncode != 0 ")
                    print("\nSTDOUT:")
                    print(process.stdout.decode("utf-8"))
                    print("\nSTDERR:")
                    print(process.stderr.decode("utf-8"))
                    print("*****************************")
