import math
import warnings
from collections import namedtuple

import numpy as np
from numpy import arange, log

# Because this feature is included only in scipy version 0.14.0 up,
# I have taken the function directly from the source at
# https://github.com/scipy/scipy/blob/master/scipy/stats/morestats.py

def _anderson_ksamp_midrank(samples, Z, Zstar, k, n, N):
    """
    Compute A2akN equation 7 of Scholz and Stephens.
    Parameters
    ----------
    samples : sequence of 1-D array_like
        Array of sample arrays.
    Z : array_like
        Sorted array of all observations.
    Zstar : array_like
        Sorted array of unique observations.
    k : int
        Number of samples.
    n : array_like
        Number of observations in each sample.
    N : int
        Total number of observations.
    Returns
    -------
    A2aKN : float
        The A2aKN statistics of Scholz and Stephens 1987.
    """

    A2akN = 0.
    Z_ssorted_left = Z.searchsorted(Zstar, 'left')
    if N == Zstar.size:
        lj = 1.
    else:
        lj = Z.searchsorted(Zstar, 'right') - Z_ssorted_left
    Bj = Z_ssorted_left + lj / 2.
    for i in arange(0, k):
        s = np.sort(samples[i])
        s_ssorted_right = s.searchsorted(Zstar, side='right')
        Mij = s_ssorted_right.astype(float)
        fij = s_ssorted_right - s.searchsorted(Zstar, 'left')
        Mij -= fij / 2.
        inner = lj / float(N) * (N*Mij - Bj*n[i])**2 / (Bj*(N - Bj) - N*lj/4.)
        A2akN += inner.sum() / n[i]
    A2akN *= (N - 1.) / N
    return A2akN


def _anderson_ksamp_right(samples, Z, Zstar, k, n, N):
    """
    Compute A2akN equation 6 of Scholz & Stephens.
    Parameters
    ----------
    samples : sequence of 1-D array_like
        Array of sample arrays.
    Z : array_like
        Sorted array of all observations.
    Zstar : array_like
        Sorted array of unique observations.
    k : int
        Number of samples.
    n : array_like
        Number of observations in each sample.
    N : int
        Total number of observations.
    Returns
    -------
    A2KN : float
        The A2KN statistics of Scholz and Stephens 1987.
    """

    A2kN = 0.
    lj = Z.searchsorted(Zstar[:-1], 'right') - Z.searchsorted(Zstar[:-1],
                                                              'left')
    Bj = lj.cumsum()
    for i in arange(0, k):
        s = np.sort(samples[i])
        Mij = s.searchsorted(Zstar[:-1], side='right')
        inner = lj / float(N) * (N * Mij - Bj * n[i])**2 / (Bj * (N - Bj))
        A2kN += inner.sum() / n[i]
    return A2kN


Anderson_ksampResult = namedtuple('Anderson_ksampResult',
                                  ('statistic', 'critical_values',
                                   'significance_level'))


def anderson_ksamp(samples, midrank=True, full_result=True):
    """The Anderson-Darling test for k-samples.
    The k-sample Anderson-Darling test is a modification of the
    one-sample Anderson-Darling test. It tests the null hypothesis
    that k-samples are drawn from the same population without having
    to specify the distribution function of that population. The
    critical values depend on the number of samples.
    Parameters
    ----------
    samples : sequence of 1-D array_like
        Array of sample data in arrays.
    midrank : bool, optional
        Type of Anderson-Darling test which is computed. Default
        (True) is the midrank test applicable to continuous and
        discrete populations. If False, the right side empirical
        distribution is used.
    Returns
    -------
    statistic : float
        Normalized k-sample Anderson-Darling test statistic.
    critical_values : array
        The critical values for significance levels 25%, 10%, 5%, 2.5%, 1%.
    significance_level : float
        An approximate significance level at which the null hypothesis for the
        provided samples can be rejected.
    Raises
    ------
    ValueError
        If less than 2 samples are provided, a sample is empty, or no
        distinct observations are in the samples.
    See Also
    --------
    ks_2samp : 2 sample Kolmogorov-Smirnov test
    anderson : 1 sample Anderson-Darling test
    Notes
    -----
    [1]_ Defines three versions of the k-sample Anderson-Darling test:
    one for continuous distributions and two for discrete
    distributions, in which ties between samples may occur. The
    default of this routine is to compute the version based on the
    midrank empirical distribution function. This test is applicable
    to continuous and discrete data. If midrank is set to False, the
    right side empirical distribution is used for a test for discrete
    data. According to [1]_, the two discrete test statistics differ
    only slightly if a few collisions due to round-off errors occur in
    the test not adjusted for ties between samples.
    .. versionadded:: 0.14.0
    References
    ----------
    .. [1] Scholz, F. W and Stephens, M. A. (1987), K-Sample
           Anderson-Darling Tests, Journal of the American Statistical
           Association, Vol. 82, pp. 918-924.
    Examples
    --------
    >>> from scipy import stats
    >>> np.random.seed(314159)
    The null hypothesis that the two random samples come from the same
    distribution can be rejected at the 5% level because the returned
    test value is greater than the critical value for 5% (1.961) but
    not at the 2.5% level. The interpolation gives an approximate
    significance level of 3.1%:
    >>> stats.anderson_ksamp([np.random.normal(size=50),
    ... np.random.normal(loc=0.5, size=30)])
    (2.4615796189876105,
      array([ 0.325,  1.226,  1.961,  2.718,  3.752]),
      0.03134990135800783)
    The null hypothesis cannot be rejected for three samples from an
    identical distribution. The approximate p-value (87%) has to be
    computed by extrapolation and may not be very accurate:
    >>> stats.anderson_ksamp([np.random.normal(size=50),
    ... np.random.normal(size=30), np.random.normal(size=20)])
    (-0.73091722665244196,
      array([ 0.44925884,  1.3052767 ,  1.9434184 ,  2.57696569,  3.41634856]),
      0.8789283903979661)
    """
    k = len(samples)
    if (k < 2):
        raise ValueError("anderson_ksamp needs at least two samples")

    samples = list(map(np.asarray, samples))
    Z = np.sort(np.hstack(samples))
    N = Z.size
    Zstar = np.unique(Z)
    if Zstar.size < 2:
        raise ValueError("anderson_ksamp needs more than one distinct "
                         "observation")

    n = np.array([sample.size for sample in samples])
    if any(n == 0):
        raise ValueError("anderson_ksamp encountered sample without "
                         "observations")

    if midrank:
        A2kN = _anderson_ksamp_midrank(samples, Z, Zstar, k, n, N)
    else:
        A2kN = _anderson_ksamp_right(samples, Z, Zstar, k, n, N)

    H = (1. / n).sum()
    hs_cs = (1. / arange(N - 1, 1, -1)).cumsum()
    h = hs_cs[-1] + 1
    g = (hs_cs / arange(2, N)).sum()

    a = (4*g - 6) * (k - 1) + (10 - 6*g)*H
    b = (2*g - 4)*k**2 + 8*h*k + (2*g - 14*h - 4)*H - 8*h + 4*g - 6
    c = (6*h + 2*g - 2)*k**2 + (4*h - 4*g + 6)*k + (2*h - 6)*H + 4*h
    d = (2*h + 6)*k**2 - 4*h*k
    sigmasq = (a*N**3 + b*N**2 + c*N + d) / ((N - 1.) * (N - 2.) * (N - 3.))
    m = k - 1
    A2 = (A2kN - m) / math.sqrt(sigmasq)

    if not full_result:
        return Anderson_ksampResult(A2, None, None)

    # The b_i values are the interpolation coefficients from Table 2
    # of Scholz and Stephens 1987
    b0 = np.array([0.675, 1.281, 1.645, 1.96, 2.326])
    b1 = np.array([-0.245, 0.25, 0.678, 1.149, 1.822])
    b2 = np.array([-0.105, -0.305, -0.362, -0.391, -0.396])
    critical = b0 + b1 / math.sqrt(m) + b2 / m
    pf = np.polyfit(critical, log(np.array([0.25, 0.1, 0.05, 0.025, 0.01])), 2)
    if A2 < critical.min() or A2 > critical.max():
        warnings.warn("approximate p-value will be computed by extrapolation")

    p = math.exp(np.polyval(pf, A2))
    return Anderson_ksampResult(A2, critical, p)