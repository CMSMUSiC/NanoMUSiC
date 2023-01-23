# Nano Event Class (NanoEC)

Lighter and faster implementation of MUSiC event class. 

# Rationale

Histograms are nothing but arrays of Counts, Variances and index (bin). If we can avoid, as much as possible reshaping the histogram (rebin and trim) and storing lots of metadata, one can potentially speed up the creation, summation and rescaling of them (implicit SIMD?).