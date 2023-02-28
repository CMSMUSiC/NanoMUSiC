# Running the analysis

The nominal MUSiC analysis, following the approach of the Run2016 [arXiv:2010.02984](https://arxiv.org/abs/2010.02984), is divided in three stages:

1 - Skimming

Data and MC NanoAOD samples are skimmed in order to:
- Build a **per-event** class model stored in TTrees, with \(p_{T}\), \($m_{inv/T}$\), $MET$ and weights calculated for all systematics variations and classes.
- The skimmed samples, since is per-event based, can be used to explore other methods of p-value estimations.

2 - Classification

- Events sharing the same classes are merged and a Event Class Model (`TEventClass`) is build.
- Data and MC ROOT files are merged in a format compatible with the legacy MUSiC code.

3 - Scan

- Calculates p-values for integrated distributions.
- Perform the RoI (Region of Interest) Scan, per-class, per-distribution.
- Calculates the p-values for selected RoIs, as well as estimates the Look Elsewhere Effect (LEE).
- Estimates the p-value distribution, corrected by the LEE: p-tilde ($\tilde{p}$-value).

## Skim

### Run Config File

Inside `./config`, different run configuration files can by found.

#### Single process

To run a test/debug Skimming:

```
nano_music --run-config <path_to_config_file>
```
The possible options are:

- `--batch` (optional): will run in batch mode. No colors, pretty-printings or flushes.
- `--run-config <path_to_config_file>` (mandatory): a run config should be provided.


