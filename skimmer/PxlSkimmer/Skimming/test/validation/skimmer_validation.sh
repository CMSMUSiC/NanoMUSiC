#!/bin/bash

EXPECTED_ARGS=1
if [ $# -ne $EXPECTED_ARGS ]
then
echo "Test"
exit 1
fi


#MiniAOD samples to be used for validation
files=$1


iteration=0
lastfile=`cat $files | wc -l`
echo "There are "$lastfile" Files"

echo "[basic]" > ValidatorConfig.cfg
echo "path = $CMSSW_BASE/src/PxlSkimmer/Skimming/test/validation/" >> ValidatorConfig.cfg
echo "hist_groups = MC, Eles, Muons, Taus, METs" >> ValidatorConfig.cfg
echo "" >> ValidatorConfig.cfg
echo "[samples]" >> ValidatorConfig.cfg
echo "" >> ValidatorConfig.cfg

while [ $iteration -lt $lastfile ];
do
  iteration=$(( iteration + 1 ))
  filename=(`head -n $iteration $files  | tail -1`)
  echo $filename 
  sed 's|testfile|'$filename'|g' mc_SP14_miniAOD_cfg_localRun.py > mc_SP14_miniAOD_cfg_localRun_new.py
  cmsRun mc_SP14_miniAOD_cfg_localRun_new.py
  filename=${filename#*RunIISpring15DR74/}
  filename=${filename%/MINIAODSIM*}
  mv test.pxlio ${filename}.pxlio
  echo "    [["${filename}.pxlio"]]" >> ValidatorConfig.cfg
  echo "        label = \"${filename%%_*}\"" >> ValidatorConfig.cfg
  echo "" >> ValidatorConfig.cfg
done

echo "[histos]

    [[h1_0_Ele_eta_Gen]]
        folder = \"MC\"

    [[h1_0_Ele_num_Gen]]
        folder = \"MC\"

    [[h1_0_Ele_phi_Gen]]
        folder = \"MC\"

    [[h1_0_Ele_pt_Gen]]
        folder = \"MC\"

    [[h1_0_Muon_eta_Gen]]
        folder = \"MC\"

    [[h1_0_Muon_num_Gen]]
        folder = \"MC\"

    [[h1_0_Muon_phi_Gen]]
        folder = \"MC\"

    [[h1_0_Muon_pt_Gen]]
        folder = \"MC\"

    [[h1_0_Tau_eta_Gen]]
        folder = \"MC\"

    [[h1_0_Tau_num_Gen]]
        folder = \"MC\"

    [[h1_0_Tau_phi_Gen]]
        folder = \"MC\"

    [[h1_0_Tau_pt_Gen]]
        folder = \"MC\"

    [[h1_Tau_num]]
        folder = \"Taus\"

    [[h1_0_Tau_pt]]
        folder = \"Taus\"

    [[h1_0_Tau_phi]]
        folder = \"Taus\"

    [[h1_0_Tau_eta]]
        folder = \"Taus\"

    [[h1_1_Tau_pt]]
        folder = \"Taus\"

    [[h1_1_Tau_phi]]
        folder = \"Taus\"

    [[h1_1_Tau_eta]]
        folder = \"Taus\"

    [[h1_2_Tau_pt]]
        folder = \"Taus\"

    [[h1_2_Tau_phi]]
        folder = \"Taus\"

    [[h1_2_Tau_eta]]
        folder = \"Taus\"

    [[h1_Muon_num]]
        folder = \"Muons\"

    [[h1_0_Muon_pt]]
        folder = \"Muons\"

    [[h1_0_Muon_phi]]
        folder = \"Muons\"

    [[h1_0_Muon_eta]]
        folder = \"Muons\"

    [[h1_1_Muon_pt]]
        folder = \"Muons\"

    [[h1_1_Muon_phi]]
        folder = \"Muons\"

    [[h1_1_Muon_eta]]
        folder = \"Muons\"

    [[h1_2_Muon_pt]]
        folder = \"Muons\"

    [[h1_2_Muon_phi]]
        folder = \"Muons\"

    [[h1_2_Muon_eta]]
        folder = \"Muons\"

    [[h1_Ele_num]]
        folder = \"Eles\"

    [[h1_0_Ele_pt]]
        folder = \"Eles\"

    [[h1_0_Ele_phi]]
        folder = \"Eles\"

    [[h1_0_Ele_eta]]
        folder = \"Eles\"

    [[h1_1_Ele_pt]]
        folder = \"Eles\"

    [[h1_1_Ele_phi]]
        folder = \"Eles\"

    [[h1_1_Ele_eta]]
        folder = \"Eles\"

    [[h1_2_Ele_pt]]
        folder = \"Eles\"

    [[h1_2_Ele_phi]]
        folder = \"Eles\"

    [[h1_2_Ele_eta]]
        folder = \"Eles\"

    [[h1_MET_num]]
        folder = \"METs\"

    [[h1_0_MET_pt]]
        folder = \"METs\"

    [[h1_0_MET_phi]]
        folder = \"METs\"

    [[h1_0_MET_eta]]
        folder = \"METs\"

    [[h1_1_MET_pt]]
        folder = \"METs\"

    [[h1_1_MET_phi]]
        folder = \"METs\"

    [[h1_1_MET_eta]]
        folder = \"METs\"

    [[h1_2_MET_pt]]
        folder = \"METs\"

    [[h1_2_MET_phi]]
        folder = \"METs\"

    [[h1_2_MET_eta]]
        folder = \"METs\"" >> ValidatorConfig.cfg
        
$MUSIC_BASE/Validator/./validator.py --nocompilation -nogit --execonfig=/user/esch/TAPAS/PxlButcher/Validator/MC.cfg --cfgfile=$CMSSW_BASE/src/PxlSkimmer/Skimming/test/validation/ValidatorConfig.cfg --debug=DEBUG
