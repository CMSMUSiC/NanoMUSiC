#include "ConvolutionComputer.hpp"

#include <algorithm>
#include <cmath>
#include <iostream>
#include <limits>
#include <sstream>

#include <gsl/gsl_errno.h>
#include <gsl/gsl_integration.h>
#include <gsl/gsl_sf_erf.h>
#include <gsl/gsl_sf_gamma.h>
#include <gsl/gsl_sf_lambert.h>

#include "TMath.h"

// constants
constexpr double SQRT2PI = std::sqrt(2 * M_PI);

// numeric limits
constexpr double DOUBLE_MIN = std::numeric_limits<double>::min();

// at compile time, evaluate some limits
// we will always add some "safety factor" (last, constant term) here

// minimum value for which exp(x) is still > 0 (~ 700)
constexpr double EXP_MIN = std::log(DOUBLE_MIN) + 5.;

// maximum x where gaus(x,center=0,width=1) != 0 (~ 37)
constexpr double GAUSS_MAX_SIGMAS = std::sqrt(-2. * std::log(SQRT2PI * DOUBLE_MIN)) - 1.;

//--------------- calculate pure Poisson probability by hand ---------------------------
double sum_p_manual(const double N_SM_integration, const double N_SM, const double N_obs)
{
    // Define function for computing sum of pure Poisson-probability
    // Need N_SM_integration for implementation of convolution
    // compute sum for both cases, using 1-Poisson (thus implementing only a single sum) can lead to problems in case of
    // very small numbers

    // cut-off precision
    const double precision = 1e-7;

    // choose the direction in which to run
    const double direction = N_obs >= N_SM ? 1 : -1;
    // choose a good stepsize
    const double step_size = direction * std::max(1., floor(sqrt(N_obs)));

    // poisson sum, later to be returned
    double sum_p = 0;
    // value of one step in sum
    // set to dummy value to go for at least one round
    double one_step = 1;

    // counter variable
    double N_obs_inc = N_obs;

    while (N_obs_inc >= 0)
    { // no Poisson below zero
        // stop if last contribution was below required precision, but only if we already passed the maximum
        if (one_step <= sum_p * precision)
        {
            if (direction > 0)
            {
                if (N_obs_inc > N_SM_integration)
                    break;
            }
            else
            {
                if (N_obs_inc < N_SM_integration)
                    break;
            }
        }

        // calculate next step
        one_step = TMath::Poisson(N_obs_inc, N_SM_integration);
        // and add it up, properly normalized
        sum_p += one_step * fabs(step_size);

        // increment...
        const double N_obs_old = N_obs_inc;
        N_obs_inc += step_size;

        //...and check we actually incremented
        if (N_obs_old == N_obs_inc)
        {
            std::cerr << "Not enough precision in double to in-/decrement N_obs in Poisson-calculation!" << std::endl;
            exit(1);
        }
    }

    return sum_p;
}

// helper functions for calculate_upper_bound and calculate_lower_bound
double first_part(const double x)
{
    return -exp((EXP_MIN + TMath::LnGamma(x + 1)) / x) / x;
}

double second_part(const double x, const double lambert)
{
    return exp((-x * lambert + EXP_MIN + TMath::LnGamma(x + 1)) / x);
}

/* Limits to TMath::Poisson
 * Explanations:
 *
 * TMath::Poisson( x, par ) works like this:
 * if (x == 0.0)
 *    return 1./Exp(par);
 * else {
 *    Double_t lnpoisson = x*log(par)-par-LnGamma(x+1.);
 *    return Exp(lnpoisson);
 * }
 *
 * Especially the exp() induced some bounds on par for a given x.
 * Outside of those bounds the calculation will always return 0.
 *
 * The following 2 functions calculate those bounds.
 *
 * Details of the calculations can be found in doc/poisson.wms
 */
double calculate_upper_bound(const double x)
{
    // special case for x=0, because same special case in TMath::Poisson
    if (x == 0)
        return -EXP_MIN;
    // the calculation fails for x ~< 1
    // so return the smallest value we can still nicely calculate
    else if (x < 1)
        return second_part(1, gsl_sf_lambert_Wm1(first_part(1)));
    // calculation fails if x >~ 1e18, so return the maximum for a double
    else if (x > 1e18)
        return std::numeric_limits<double>::max();
    // all corner cases covered, do the calculation
    else
        return second_part(x, gsl_sf_lambert_Wm1(first_part(x)));
}

double calculate_lower_bound(const double x)
{
    // calculation returns 0 for x < 1.1
    // however TMath::Poisson is 0 for lambda=0
    // so no use to go lower than the minimum for a double
    if (x < 1.1)
        return std::numeric_limits<double>::min();
    // calculation fails for x >~ 1e18, so return the largest value we can still calculate
    if (x > 1e18)
        return second_part(1e18, gsl_sf_lambert_W0(first_part(1e18)));
    // all corner cases covered, do the calculation
    else
        return second_part(x, gsl_sf_lambert_W0(first_part(x)));
}

// This function computes the sum of the poission distribution, either from
// 0 to N_obs or from N_obs to infinity.
// N_SM is only used for the decision between the branches, the actual poission
// mean/parameter is given as N_SM_integration.
// The sum is computed using the regularized incomplete gamma functions P and Q
// as implemented by the Gnu Scientific Library (GSL).
// Q(max, lambda) gives the cumulative poission distribution up to 'max',
// and P = 1 - Q is the cumulative distribution up to infinity.
// More on P and Q here:
// https://en.wikipedia.org/wiki/Incomplete_gamma_function#Regularized_Gamma_functions_and_Poisson_random_variables
double sum_p(const double N_SM_integration, const double N_SM, const double N_obs)
{
    // switch off the GSL error handler, we'll take care about that
    const gsl_error_handler_t *old_handler = gsl_set_error_handler_off();

    double ret;
    gsl_sf_result result;
    int status;
    if (N_obs >= N_SM)
    {
        // excess case: more events observed than predicted
        // => sum from N_obs to infinity

        if (N_SM_integration > N_obs)
        {
            // Edge case: gaussian smearing has set N_SM_integration > N_obs
            // (even though N_SM < N_obs!), so the sum to infinity will be ~1.
            // So the calculation using P might be unstable, thus we use 1-Q.
            status = gsl_sf_gamma_inc_Q_e(N_obs, N_SM_integration, &result);
            ret = 1 - result.val;
        }
        else
        {
            // do normal calculation
            status = gsl_sf_gamma_inc_P_e(N_obs, N_SM_integration, &result);
            ret = result.val;
        }
    }
    else
    {
        // deficit case: less events observed than predicted
        // => sum from 0 to N_obs

        if (N_SM_integration < N_obs)
        {
            // Similar edge case as above:
            // Result will be close to 1 if we use Q, that may be unstable, so use 1-P
            status = gsl_sf_gamma_inc_P_e(N_obs + 1, N_SM_integration, &result);
            ret = 1 - result.val;
        }
        else
        {
            // do normal calculation
            status = gsl_sf_gamma_inc_Q_e(N_obs + 1, N_SM_integration, &result);
            ret = result.val;
        }
    }

    // restore the old error handler
    gsl_set_error_handler(old_handler);

    if (status != GSL_SUCCESS)
    {
        // in case the GSL failed, try it manually
        ret = sum_p_manual(N_SM_integration, N_SM, N_obs);
    }

    return ret;
}

// Integration callback
// Integrand: takes integration variable x as first parameter (ranges from 0 to infinity)
// Other parameters: N_SM, sigma_SM, N_obs
// High precision, using ROOT and manual methode, if necessary (see sum_p function)
double integration_payload_normal(double x, void *par_tmp)
{
    const double *par = (double *)par_tmp;

    const double N_SM = par[0];
    const double sigma_SM = par[1];
    const double N_obs = par[2];

    const double poisson_value = sum_p(x, N_SM, N_obs);
    const double gaus_value = TMath::Gaus(x, N_SM, sigma_SM);

    return gaus_value * poisson_value;
}

double integration_payload_lognormal(double x, void *par_tmp)
{
    const double *par = (double *)par_tmp;

    const double N_SM = par[0];
    const double sigma_SM = par[1];
    const double N_obs = par[2];

    const double poisson_value = sum_p(x, N_SM, N_obs);

    // The ROOT documentation has the things a little mixed up...
    // The original source is http://www.itl.nist.gov/div898/handbook/eda/section3/eda3669.htm
    // As one can see, among the multiple parametrizations, there is one using
    // a parameter commonly called "m" and a different one using the parameter
    // "mu".
    // ROOT::Math::lognormal_pdf implements the latter parametrization
    // LN(X;sigma,theta,mu) but calls mu "m" (and theta "x0")
    // TMath::LogNormal corrects this error by setting "m" = mu = log(m), thus
    // representing the parametrization LN(x;sigma,theta,m)

    // For us:              x=theta, sigma=ln(1+err_rel), theta=0, mu=log(N_SM)
    // Thus for LogNormal:  x=theta, sigma=ln(1+err_rel), theta=0, m=N_SM

    // std::log1p(x) == std::log(1+x) , but with better precision, especially at small x
    const double sigma = std::log1p(sigma_SM / N_SM);
    const double lognormal_value = TMath::LogNormal(x, sigma, 0.0, N_SM);

    return lognormal_value * poisson_value;
}

// "main"-function of this module. computes the MUSiC p-value associated with N_obs, N_SM and sigma_MC
double compute_p_convolution(const double N_obs,
                             const double N_SM,
                             const double sigma_MC,
                             PriorMode prior,
                             const int debugLevel)
{
    if (debugLevel > 2)
    {
        std::stringstream debug;
        debug << "[DEBUG] ConvolutionComputer:" << std::endl;
        debug << "    N_obs = " << N_obs << std::endl;
        debug << "    N_SM  = " << N_SM << std::endl;
        debug << "    sigma_MC = " << sigma_MC << std::endl;

        std::cerr << debug.str();
    }

    // upper and lower limits where the gaus function still returns > 0
    const double g_lower = N_SM - GAUSS_MAX_SIGMAS * sigma_MC;
    const double g_upper = N_SM + GAUSS_MAX_SIGMAS * sigma_MC;

    if (debugLevel > 2)
    {
        std::stringstream debug;
        debug << "[DEBUG] ConvolutionComputer:" << std::endl;
        debug << "    Gaussian integration limits:" << std::endl;
        debug << "    g_lower = " << g_lower << std::endl;
        debug << "    g_upper = " << g_upper << std::endl;

        std::cerr << debug.str();
    }

    // set the integration bounds
    // lower bound never below 0
    double lower = std::max(0., g_lower);
    double upper = g_upper;

    // the Poisson calculation will be zero if N_obs is too far away from lambda
    // hence limit the integral to an interval where the Poisson calculation is non-zero
    if (N_obs < N_SM)
    {
        // here we can only limit the upper bound
        const double p_upper = calculate_upper_bound(N_obs);
        upper = std::min(upper, p_upper);
    }
    else
    {
        const double p_lower = calculate_lower_bound(N_obs);
        // and here only the lower bound
        lower = std::max(lower, p_lower);
    }

    // rough estimate of the peak position
    const double err_sq = sigma_MC * sigma_MC;
    const double peak =
        (N_SM - err_sq + sqrt(N_SM * N_SM - 2 * N_SM * err_sq + err_sq * err_sq + 4 * N_obs * err_sq)) / 2;

    // we need to stay close to the peak or the integration might miss it
    double Nsigma = fabs(N_SM - N_obs) / sigma_MC + 1;
    // should be between 5 and 10
    if (Nsigma < 5)
        Nsigma = 5;
    else if (Nsigma > 10)
        Nsigma = 10;

    const double peak_lower = peak - (sigma_MC * Nsigma);
    const double peak_upper = peak + (sigma_MC * Nsigma);

    // but only move the bounds if there is still some space left
    if (peak_lower < upper)
        lower = std::max(lower, peak_lower);
    if (peak_upper > lower)
        upper = std::min(upper, peak_upper);

    // Check sanity.
    if (lower > upper or lower == upper)
    {
        if (debugLevel > 0)
        {
            std::stringstream warn;
            warn << "[WARNING] ConvolutionComputer_add:" << std::endl;
            warn << "    Lower integration bound >= upper bound!" << std::endl;
            warn << "    lower = " << lower << std::endl;
            warn << "    upper = " << upper << std::endl;

            warn << "General information:" << std::endl;
            warn << "    N_obs = " << N_obs << std::endl;
            warn << "    N_SM  = " << N_SM << std::endl;
            warn << "    sigma_MC = " << sigma_MC << std::endl;

            warn << "Peak information:" << std::endl;
            warn << "    peak       = " << peak << std::endl;
            warn << "    peak_lower = " << peak_lower << std::endl;
            warn << "    peak_upper = " << peak_upper << std::endl;

            warn << "Gaussian integration limits:" << std::endl;
            warn << "    g_lower = " << g_lower << std::endl;
            warn << "    g_upper = " << g_upper << std::endl;

            warn << "p-value set to -1!" << std::endl;

            std::cerr << warn.str();
        }
        return -1.;
    }

    // get a GSL integration workspace good for 1000 sampling points
    const size_t max_points = 1000;
    gsl_integration_workspace *gsl_int_ws = gsl_integration_workspace_alloc(max_points);

    // set the parameters
    double params[3];
    params[0] = N_SM;
    params[1] = sigma_MC;
    params[2] = N_obs;

    // and make a GSL function
    gsl_function gsl_func;
    gsl_func.params = params;

    switch (prior)
    {
    case NORMAL_PRIOR:
        gsl_func.function = integration_payload_normal;
        break;
    case LOGNORMAL_PRIOR:
        gsl_func.function = integration_payload_lognormal;
        break;
    default:
        std::cerr << "[ERROR] Invalid prior!" << std::endl;
        exit(1);
        break;
    }

    // requested integration precisions
    // relative: 1% is enough
    const double rel_precision = 1e-2;

    // absolute: no need to get anything precisely, if it's damn small anyway
    // here damn small equals 1e-30, but needs to be scaled to a un-normalized gaussian
    const double abs_precision = 1e-30 * SQRT2PI * sigma_MC;

    if (debugLevel > 2)
    {
        std::stringstream debug;
        debug << "[DEBUG] ConvoulutionComputer:" << std::endl;
        debug << "    gsl_integration_qag() input:" << std::endl;
        debug << "        lower = " << lower << std::endl;
        debug << "        upper = " << upper << std::endl;
        debug << "        abs_precision = " << abs_precision << std::endl;
        debug << "        rel_precision = " << rel_precision << std::endl;
        debug << "        max_points = " << max_points << std::endl;

        std::cerr << debug.str();
    }

    // and now integrate
    double convolution = 0;
    double conv_error = 0;

    const int ret_code = gsl_integration_qag(&gsl_func,
                                             lower,
                                             upper,
                                             abs_precision,
                                             rel_precision,
                                             max_points,
                                             GSL_INTEG_GAUSS15,
                                             gsl_int_ws,
                                             &convolution,
                                             &conv_error);

    if (debugLevel > 2)
    {
        std::stringstream debug;
        debug << "[DEBUG] ConvolutionComputer:" << std::endl;
        debug << "    gsl_integration_qag() results:" << std::endl;
        debug << "        convolution = " << convolution << std::endl;
        debug << "        conv_error  = " << conv_error << std::endl;

        std::cerr << debug.str();
    }

    // check for errors
    switch (ret_code)
    {
    case GSL_EMAXITER:
        std::cerr << "Integration terminated with max iterations exceeded." << std::endl;
        return -1;
        break;
    case GSL_EROUND:
        std::cerr << "Integration terminated with roundoff error." << std::endl;
        return -1;
        break;
    case GSL_ESING:
        std::cerr << "Integration terminated with non-integrable singularity." << std::endl;
        return -1;
        break;
    case GSL_EDIVERGE:
        std::cerr << "Integration terminated without convergance." << std::endl;
        return -1;
        break;
    }

    // clean the workspace
    gsl_integration_workspace_free(gsl_int_ws);

    // to remove errors due to precision, when p value is very close to 0.
    if (convolution < 0.)
    {
        if (std::abs(convolution) < 1e-12)
        {

            std::stringstream warn;
            warn << "[WARNING] compute_p_convolution:" << std::endl;
            warn << "    Small negative p-value probalby due to precision, setting it to a small positive value!"
                 << std::endl;
            warn << "    convolution = " << convolution << std::endl;
            warn << "    setting convolution to = " << 1e-12 << std::endl;
            std::cerr << warn.str();

            convolution = 1e-12;
        }
    }

    if (convolution < 0.)
    {

        std::cerr << "[ERROR] Convolution < 0 ! " << std::endl;
        std::cerr << "   convolution = " << convolution << std::endl;
        std::cerr << "   N_SM = " << N_SM << std::endl;
        std::cerr << "   N_obs = " << N_obs << std::endl;
        std::cerr << "   sigma_MC = " << sigma_MC << std::endl;
        exit(1);
    }
    else if (convolution > 0)
    {

        if (prior == NORMAL_PRIOR)
        {
            // calculate normalization
            const double sigmas = N_SM / sigma_MC;
            const double normalisation = gsl_sf_erf_Q(-sigmas) * sigma_MC * SQRT2PI;

            if (debugLevel > 2)
            {
                std::stringstream debug;
                debug << "[DEBUG] ConvolutionComputer:" << std::endl;
                debug << "    gsl_sf_erf_Q() normalisation:" << std::endl;
                debug << "        normalisation = " << normalisation << std::endl;

                std::cerr << debug.str();
            }

            convolution /= normalisation;

            if (convolution < 0.)
            {
                std::cerr << "[ERROR] Convolution < 0 (after normalization) ! " << std::endl;
                std::cerr << "   convolution = " << convolution << std::endl;
                std::cerr << "   normalisation = " << normalisation << std::endl;
                std::cerr << "   convolution/normalisation = " << convolution / normalisation << std::endl;
                std::cerr << "   N_SM = " << N_SM << std::endl;
                std::cerr << "   N_obs = " << N_obs << std::endl;
                std::cerr << "   sigma_MC = " << sigma_MC << std::endl;
                exit(1);
            }
        }
    }

    return convolution;
}
