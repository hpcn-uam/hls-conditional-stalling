
import subprocess

OPERATIONS = [(0,"add"),(1,"mul"),(2,"div")]
TYPES = [(0,"double"),(1,"float"),(2,"int32")]

DD_configs = [(1.5,8),(1.25,12),(1.25,16),(1.25,32)]
OP_CODE = 0
TYPE_CODE = 0


for PART in ["zcu104","pynq_z2"]:
    if PART == "zcu104":
        DD_configs = [(1.5,8),(1.25,12),(1.2,16),(1.2,32)]
    elif PART == "pynq_z2":
        DD_configs = [(5.5,8),(5,12),(5,16),(5,32)]
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
