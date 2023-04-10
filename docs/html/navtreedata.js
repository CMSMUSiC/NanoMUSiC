/*
 @licstart  The following is the entire license notice for the JavaScript code in this file.

 The MIT License (MIT)

 Copyright (C) 1997-2020 by Dimitri van Heesch

 Permission is hereby granted, free of charge, to any person obtaining a copy of this software
 and associated documentation files (the "Software"), to deal in the Software without restriction,
 including without limitation the rights to use, copy, modify, merge, publish, distribute,
 sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is
 furnished to do so, subject to the following conditions:

 The above copyright notice and this permission notice shall be included in all copies or
 substantial portions of the Software.

 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING
 BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
 DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

 @licend  The above is the entire license notice for the JavaScript code in this file
*/
var NAVTREE =
[
  [ "NanoMUSiC", "index.html", [
    [ "About", "index.html#autotoc_md1", [
      [ "License", "index.html#autotoc_md2", null ]
    ] ],
    [ "The MUSiC algorithm in a nutsheld", "index.html#autotoc_md3", [
      [ "Kinematical distributions of interest", "index.html#autotoc_md4", [
        [ "Sum pT: $\\sum_{i}|\\vec{p_{i}}_{T}|$", "index.html#autotoc_md5", null ]
      ] ]
    ] ],
    [ "Setup and instructiond", "index.html#autotoc_md6", null ],
    [ "Setup", "index.html#autotoc_md7", [
      [ "Dependencies", "index.html#autotoc_md8", null ],
      [ "Setup the environment (only once)", "index.html#autotoc_md9", null ],
      [ "Configuring the environment (for every new session)", "index.html#autotoc_md10", null ],
      [ "Compilation", "index.html#autotoc_md11", null ],
      [ "Installations directories", "index.html#autotoc_md12", null ]
    ] ],
    [ "Running the analysis", "index.html#autotoc_md13", [
      [ "Skim", "index.html#autotoc_md14", [
        [ "Run Config File", "index.html#autotoc_md15", [
          [ "Single process", "index.html#autotoc_md16", null ]
        ] ],
        [ "Data Samples", "index.html#autotoc_md17", null ]
      ] ],
      [ "References", "index.html#autotoc_md18", [
        [ "NanoAOD file Content", "index.html#autotoc_md19", null ],
        [ "PDF recomendations", "index.html#autotoc_md20", null ],
        [ "HLT Bits", "index.html#autotoc_md21", null ],
        [ "2017", "index.html#autotoc_md22", [
          [ "Muons", "index.html#autotoc_md23", null ]
        ] ],
        [ "Run2 Generator Output", "index.html#autotoc_md24", null ],
        [ "Electron Trigger Scale Factors", "index.html#autotoc_md25", null ]
      ] ]
    ] ],
    [ "Notes on the Legacy Code (German)", "index.html#autotoc_md26", [
      [ "1) <strong>Alle MC & Data Samples auf <a href=\"https://cmsweb.cern.ch/das/request?view=list&limit=50&instance=prod%2Fglobal&input=dataset%3D%2FZToMuMu_*%2FRunIISummer20UL18MiniAODv2*%2FMINIAODSIM*\">DAS</a>, <a href=\"https://cms-pdmv.cern.ch/grasp/samples?dataset_query=ZToMuMu_M-3500To4500_TuneCP5_13TeV-powheg-pythia8&campaign=Run3Winter22*GS,RunIISummer20UL16*GEN,RunIISummer20UL16*GENAPV,RunIISummer20UL17*GEN,RunIISummer20UL18*GEN\">Grasp</a> etc. suchen wie <a href=\"https://docs.google.com/spreadsheets/d/1C3wC3vG5VHEX0-bk-s6qdhGR0Hy7KcVLkG9efDYlN6Q/edit#gid=1654878574\">hier</a></strong>", "index.html#autotoc_md32", null ],
      [ "2) <strong>[[MUSiC einrichten]]</strong>", "index.html#autotoc_md33", null ],
      [ "- <tt>voms-proxy-init --voms cms --voms cms:/cms/dcms --valid 198:0</tt>", "index.html#autotoc_md34", null ],
      [ "- <strong>WICHTIG:</strong> Manchmal entsteht ein Buffer Overflow, wenn das passiert einfach nochmal das Kommando nutzen (solange bis alle Samples einen Eintrag in der Database haben der nicht 0 ist)", "index.html#autotoc_md35", null ],
      [ "- Wenn die Classification zu Beginn Failed dann Fehler suchen und in einem <strong>NEUEM</strong> Ordner alles nochmal starten", "index.html#autotoc_md36", [
        [ "<strong>PxlAnalyzer</strong>", "index.html#autotoc_md28", null ],
        [ "<strong>MUSiC-Configs</strong>", "index.html#autotoc_md29", null ],
        [ "<strong>PxlSkimmer</strong>", "index.html#autotoc_md30", null ],
        [ "MUSiC installieren", "index.html#autotoc_md37", null ],
        [ "PxlSkimmer installieren", "index.html#autotoc_md38", null ],
        [ "Git SSH", "index.html#autotoc_md39", null ],
        [ "Zertifikate einrichten", "index.html#autotoc_md40", null ],
        [ "VOMS", "index.html#autotoc_md41", null ],
        [ "HyperNews", "index.html#autotoc_md42", null ],
        [ "Analyse Vorbereitung:", "index.html#autotoc_md43", null ],
        [ "Zusatz:", "index.html#autotoc_md44", [
          [ "Für PxlAnalyzer/ConfigFiles/Objects/ScaleFactors/ele_sf.cff", "index.html#autotoc_md27", null ],
          [ "ParseSampleList", "index.html#autotoc_md45", null ],
          [ "Data parse...", "index.html#autotoc_md46", null ]
        ] ],
        [ "Wichtige Seiten:", "index.html#autotoc_md69", null ]
      ] ]
    ] ],
    [ "Namespaces", "namespaces.html", [
      [ "Namespace List", "namespaces.html", "namespaces_dup" ],
      [ "Namespace Members", "namespacemembers.html", [
        [ "All", "namespacemembers.html", null ],
        [ "Functions", "namespacemembers_func.html", null ],
        [ "Variables", "namespacemembers_vars.html", null ],
        [ "Typedefs", "namespacemembers_type.html", null ]
      ] ]
    ] ],
    [ "Classes", "annotated.html", [
      [ "Class List", "annotated.html", "annotated_dup" ],
      [ "Class Index", "classes.html", null ],
      [ "Class Hierarchy", "hierarchy.html", "hierarchy" ],
      [ "Class Members", "functions.html", [
        [ "All", "functions.html", "functions_dup" ],
        [ "Functions", "functions_func.html", null ],
        [ "Variables", "functions_vars.html", "functions_vars" ],
        [ "Typedefs", "functions_type.html", null ],
        [ "Enumerations", "functions_enum.html", null ],
        [ "Enumerator", "functions_eval.html", null ],
        [ "Related Functions", "functions_rela.html", null ]
      ] ]
    ] ],
    [ "Files", "files.html", [
      [ "File List", "files.html", "files_dup" ],
      [ "File Members", "globals.html", [
        [ "All", "globals.html", null ],
        [ "Functions", "globals_func.html", null ],
        [ "Variables", "globals_vars.html", null ],
        [ "Typedefs", "globals_type.html", null ],
        [ "Enumerations", "globals_enum.html", null ],
        [ "Enumerator", "globals_eval.html", null ],
        [ "Macros", "globals_defs.html", null ]
      ] ]
    ] ]
  ] ]
];

var NAVTREEINDEX =
[
"Configs_8hpp.html",
"classCorrector.html#aae719be4f4bac28c4f01b66d823f3d0f",
"classOutputs.html#aeabc7fa66306f21aff8eeb634d8671c4",
"nano2pxl__utils_8hpp.html#aedf31ff30ecf8fedd43af0afabee3eab",
"structObjConfig_1_1MuonConfig.html#a5e53dc26624984cfc04e9ba166bfba9c"
];

var SYNCONMSG = 'click to disable panel synchronisation';
var SYNCOFFMSG = 'click to enable panel synchronisation';