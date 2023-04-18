
from computation import summer
from condor_scheduler import CondorScheduler

def main():
    condor_scheduler = CondorScheduler(output_folder = "Test_Outputs", input_folder="../CondorScheduler", prologue=["source /cvmfs/sft.cern.ch/lcg/views/LCG_102b/x86_64-centos7-gcc12-opt/setup.sh", "cd CondorScheduler","ls","pwd","hostname"])
    for i in range(1,100):
        condor_scheduler.submit_task("./computation.py", [i])
    condor_scheduler.finalise()
        
if __name__=="__main__":
    main()
