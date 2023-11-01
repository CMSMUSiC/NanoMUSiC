#ifndef ConvolutionComputer_hh
#define ConvolutionComputer_hh

// debugging levels:
// 0: ERROR
// 1: WARNING
// 2: INFO (not used)
// 3: DEBUG

enum PriorMode
{
    NORMAL_PRIOR = 1,
    LOGNORMAL_PRIOR = 2
};

// use 'extern "C"' to disable name mangling (useful for loading shared object library from Python)
extern "C" double compute_p_convolution(const double N_obs,
                                        const double N_SM,
                                        const double error_parameter,
                                        PriorMode prior,
                                        const int debugLevel = 1);

#endif
