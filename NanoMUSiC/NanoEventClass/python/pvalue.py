import numpy as np
import scipy.stats
from scipy.special import erfinv
import math

NORMAL_PRIOR = 1
LOGNORMAL_PRIOR = 2
DEFAULT_PRIOR = NORMAL_PRIOR


def compute_p_value(n_obs, n_exp, sigma_exp, prior=DEFAULT_PRIOR):
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


if __name__ == "__main__":
    main()
