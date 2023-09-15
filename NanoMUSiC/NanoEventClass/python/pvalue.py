import numpy as np
import scipy.stats
from scipy.special import erfinv
import math
import json

NORMAL_PRIOR = 1
LOGNORMAL_PRIOR = 2
DEFAULT_PRIOR = NORMAL_PRIOR

m_noLowStatsTreatment = False


def get_integral_pValue(
    ec_name, n_obs, n_exp, sigma_exp, sigma_stat, yield_per_process_group
):
    p = compute_p_value(n_obs, n_exp, sigma_exp, sigma_stat, yield_per_process_group)

    p_value_data = {
        "Event Class": ec_name,
        "p-value": p,
        "N_MC": n_exp,
        "N_Data": n_obs,
        "Total Uncertainity": sigma_exp,
        "Statistical Uncertainity": sigma_stat,
    }
    json_file_path = f"{ec_name}.json"

    with open(json_file_path, "w") as json_file:
        json.dump(p_value_data, json_file)


def veto_region(n_obs, n_exp, sigma_exp, sigma_stat, yield_per_process_group):
    mc_threshold = 1e-06
    data_threshold = 1e-9
    coverage_threshold = 0.0
    sigma_threshold = 0.6
    threshold_low_stat = 0.6

    if n_exp <= 0:
        return True

    if n_exp < mc_threshold:
        return True

    if (
        n_obs < data_threshold
        and not n_exp == 0
        and abs(n_exp / sigma_stat) < coverage_threshold
    ):
        return True

    adaptive_coverage_threshold = min(1.0, max(1.2 * (n_exp ** (-0.2)), 0.5))
    relative_uncertainity = abs(sigma_exp / n_exp)
    if relative_uncertainity > adaptive_coverage_threshold:
        return True

    if ((n_obs - n_exp) / sigma_exp) < sigma_threshold:
        return True

    if sigma_stat / n_exp > threshold_low_stat:
        return True

    yield_threshold = -0.02 * n_exp
    for Yield in yield_per_process_group:
        if Yield < yield_threshold:
            return True

    if not m_noLowStatsTreatment:
        if sigma_stat / n_exp > threshold_low_stat:
            return True

    return False


def compute_p_value(
    n_obs, n_exp, sigma_exp, sigma_stat, yield_per_process_group, prior=DEFAULT_PRIOR
):
    if veto_region(n_obs, n_exp, sigma_exp, sigma_stat, yield_per_process_group):
        return None

    if prior == NORMAL_PRIOR:
        prior_func = lambda theta: np.exp(
            -np.power((theta - n_exp) / sigma_exp, 2) / 2.0
        )
    elif prior == LOGNORMAL_PRIOR:
        lnk = np.log1p(sigma_exp / n_exp)
        prior_func = lambda theta: scipy.stats.lognorm.pdf(theta, lnk, 0, n_exp)
        # equivalent:
        # prior_func = lambda theta: np.exp( -np.power( np.log( theta / n_exp ) / lnk, 2 )/2. ) / ( math.sqrt( 2.*math.pi ) * lnk * theta )
    else:
        raise ValueError("Invalid prior")

    excess_func = lambda theta: prior_func(theta) * scipy.stats.poisson.sf(
        n_obs - 1, theta
    )
    deficit_func = lambda theta: prior_func(theta) * scipy.stats.poisson.cdf(
        n_obs, theta
    )
    integration_args = dict(
        epsrel=1e-4,
        epsabs=1e-15,
        limit=1000000,
    )

    # 8 sigma gives precision of 1e-16
    lower_bound = max(0, n_exp - 8.0 * sigma_exp)
    upper_bound = n_exp + 8.0 * sigma_exp

    if n_obs > n_exp:
        p = scipy.integrate.quad(
            excess_func, lower_bound, upper_bound, **integration_args
        )[0]
    else:
        p = scipy.integrate.quad(
            deficit_func, lower_bound, upper_bound, **integration_args
        )[0]

    if prior == NORMAL_PRIOR:
        p /= scipy.integrate.quad(
            prior_func, lower_bound, upper_bound, **integration_args
        )[0]

    return p


def pvalue_to_z(p, sided=2):
    if sided == 1:
        return math.sqrt(2.0) * erfinv(1.0 - p)
    elif sided == 2:
        return math.sqrt(2.0) * erfinv(1.0 - 2.0 * p)
    else:
        raise Exception("sided must be either 1 or 2")


def main():
    p = compute_p_value(10.0, 2.0, 0.2)
    print(f"p = {p} : z = {pvalue_to_z(p)}")

    p = compute_p_value(2.0, 10.0, 0.2)
    print(f"p = {p} : z = {pvalue_to_z(p)}")


if _name_ == "__main__":
    main()
