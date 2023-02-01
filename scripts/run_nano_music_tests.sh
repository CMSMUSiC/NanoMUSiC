#!/usr/bin/env bash
pkill -9 -f run_configs 2> /dev/null
rm -rf test_* 2> /dev/null
rm -rf Test_Ouputs_* 2> /dev/null

start=$SECONDS
FAIL=0

echo "Starting tests ..."
echo ""

nano_music --run-config ../configs/run_configs/MC_Run2016APV.toml > Test_Ouputs_MC_Run2016APV.txt  2>&1 &
echo \[ $! \] `ps -p $! -o args` | sed -e "s/COMMAND//"

nano_music --run-config ../configs/run_configs/MC_Run2016.toml > Test_Ouputs_MC_Run2016.txt  2>&1 &
echo \[ $! \] `ps -p $! -o args` | sed -e "s/COMMAND//"

nano_music --run-config ../configs/run_configs/MC_Run2017.toml > Test_Ouputs_MC_Run2017.txt  2>&1 &
echo \[ $! \] `ps -p $! -o args` | sed -e "s/COMMAND//"

nano_music --run-config ../configs/run_configs/MC_Run2017_POWHEG.toml > Test_Ouputs_MC_Run2017_POWHEG.txt  2>&1 &
echo \[ $! \] `ps -p $! -o args` | sed -e "s/COMMAND//"

nano_music --run-config ../configs/run_configs/MC_Run2017_JHUGen.toml > Test_Ouputs_MC_Run2017_JHUGen.txt  2>&1 &
echo \[ $! \] `ps -p $! -o args` | sed -e "s/COMMAND//"

nano_music --run-config ../configs/run_configs/MC_Run2018.toml > Test_Ouputs_MC_Run2018.txt  2>&1 &
echo \[ $! \] `ps -p $! -o args` | sed -e "s/COMMAND//"

nano_music --run-config ../configs/run_configs/MC_Run2018_Pythia_only.toml > Test_Ouputs_MC_Run2018_Pythia_only.txt  2>&1 &
echo \[ $! \] `ps -p $! -o args` | sed -e "s/COMMAND//"

nano_music --run-config ../configs/run_configs/MC_Run2018_QCD.toml > Test_Ouputs_MC_Run2018_QCD.txt  2>&1 &
echo \[ $! \] `ps -p $! -o args` | sed -e "s/COMMAND//"

nano_music --run-config ../configs/run_configs/Data_Run2016APV.toml > Test_Ouputs_Data_Run2016APV.txt  2>&1 &
echo \[ $! \] `ps -p $! -o args` | sed -e "s/COMMAND//"

nano_music --run-config ../configs/run_configs/Data_Run2016.toml > Test_Ouputs_Data_Run2016.txt  2>&1 &
echo \[ $! \] `ps -p $! -o args` | sed -e "s/COMMAND//"

nano_music --run-config ../configs/run_configs/Data_Run2017.toml > Test_Ouputs_Data_Run2017.txt  2>&1 &
echo \[ $! \] `ps -p $! -o args` | sed -e "s/COMMAND//"

nano_music --run-config ../configs/run_configs/Data_Run2018.toml > Test_Ouputs_Data_Run2018.txt  2>&1 &
echo \[ $! \] `ps -p $! -o args` | sed -e "s/COMMAND//"


for job in `jobs -p`
do
echo ""
echo "Waiting ..."
echo $job
    wait $job || let "FAIL+=1"
done

echo ""
if [ "$FAIL" == "0" ];
then
echo "Result: YAY!"
else
echo "Result: FAIL! ($FAIL)"
fi

duration=$(( SECONDS - start ))
echo "... done in $duration sec."


echo ""
ls Test_Ouputs_*.txt