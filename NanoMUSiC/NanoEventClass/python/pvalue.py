import numpy as np
import scipy.stats
from scipy.special import erfinv
import math

NORMAL_PRIOR = 1
LOGNORMAL_PRIOR = 2
DEFAULT_PRIOR = NORMAL_PRIOR

m_noLowStatsTreatment = False


def get_integral_pvalue(n_obs, n_exp, sigma_exp, sigma_stat, yield_per_process_group):
    p, print_veto_reason = compute_p_value(
        n_obs, n_exp, sigma_exp, sigma_stat, yield_per_process_group
    )

    return {
        # "Event Class": ec_name,
        "p-value": p,
        "N_MC": n_exp,
        "N_Data": n_obs,
        "Total Uncertainity": sigma_exp,
        "Statistical Uncertainity": sigma_stat,
        "Veto Reason": print_veto_reason,
    }


def veto_region(n_obs, n_exp, sigma_exp, sigma_stat, yield_per_process_group):
    return [False, "None"]
    mc_threshold = 1e-06
    data_threshold = 1e-9
    coverage_threshold = 0.0
    threshold_low_stat = 0.6
    if n_exp <= 0:
        return [True, "MC Less than 0"]

    if n_exp < mc_threshold:
        return [True, "MC less than threshold"]

    if (
        n_obs < data_threshold
        and not n_exp == 0
        and abs(n_exp / sigma_stat) < coverage_threshold
    ):
        return [True, "Low Coverage"]

    adaptive_coverage_threshold = min(1.0, max(1.2 * (n_exp ** (-0.2)), 0.5))
    relative_uncertainity = abs(sigma_exp / n_exp)
    if relative_uncertainity > adaptive_coverage_threshold:
        return [True, "relative uncertainity greater than adaptive coverage threshold"]

    yield_threshold = -0.02 * n_exp
    for Yield in yield_per_process_group:
        if Yield < yield_threshold:
            return [True, "Low Process Group yield"]

    if not m_noLowStatsTreatment:
        if sigma_stat / n_exp > threshold_low_stat:
            return [True, "Low stat"]

    return [False, "None"]


def compute_p_value(
    n_obs, n_exp, sigma_exp, sigma_stat, yield_per_process_group, prior=DEFAULT_PRIOR
):
    veto, veto_reason = veto_region(
        n_obs, n_exp, sigma_exp, sigma_stat, yield_per_process_group
    )

    if veto:
        return [None, veto_reason]
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

    return p, veto_reason


def pvalue_to_z(p, sided=2):
    if sided == 1:
        return math.sqrt(2.0) * erfinv(1.0 - p)
    elif sided == 2:
        return math.sqrt(2.0) * erfinv(1.0 - 2.0 * p)
    else:
        raise Exception("sided must be either 1 or 2")


def main():
    # yield_per_process_group = [1] * 100

    # event_classes = ["a"] * 10000
    # for i in range(0, 10000):
    #     event_classes[i] = "".join(random.choices(string.ascii_lowercase, k=8))

    get_integral_pValue(
        "EC_2Muon",
        30764504.0,
        30420782.0,
        1521216.9847603259,
        23263.082222437042,
        [
            57.043377,
            121737.76,
            1834.3595,
            0.011711164,
            62.829868,
            39.805325,
            11762.720,
            37319.207,
            4.9203401,
            30125480.0,
            13188.325,
            0.10865737,
            5.2262754,
            32640.074,
            81.760590,
            11.339705,
            514.83551,
            1.1576498,
            26738.008,
            30764504.0,
            14366.714,
            24.995089,
            19070.988,
            15751.234,
            52.172863,
            11.712051,
            7.9483948,
            0.012998534,
            18.934668,
            0.29516774,
        ],
    )
    # with Pool(100) as p:
    #     list(
    #         tqdm.tqdm(
    #             p.imap(
    #                 partial(
    #                     get_integral_pValue,
    #                     n_obs=10.0,
    #                     n_exp=2.0,
    #                     sigma_exp=0.2,
    #                     sigma_stat=0.1,
    #                     yield_per_process_group=yield_per_process_group,
    #                 ),
    #                 event_classes,
    #             ),
    #             total=len(event_classes),
    #         )
    #     )

    # p = compute_p_value(10.0, 2.0, 0.2)
    # print(f"p = {p} : z = {pvalue_to_z(p)}")

    # p = compute_p_value(2.0, 10.0, 0.2)
    # print(f"p = {p} : z = {pvalue_to_z(p)}")


if __name__ == "__main__":
    main()
