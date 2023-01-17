import re
import fnmatch
# =====================
# Miscellaneous helpers
# =====================

# http://stackoverflow.com/a/1176023/489345
def camel2lower(name):
    s1 = re.sub('(.)([A-Z][a-z]+)', r'\1_\2', name)
    return re.sub('([a-z0-9])([A-Z])', r'\1_\2', s1).lower()


def rremove(string, needle):
    """Removes the string provided by 'needle' from the end of 'string'.

    This is useful e.g. when removing file extensions:
        >>> rremove("test.root", ".root") == "test"
    If 'needle' is not found, nothing happens.
    """
    if len(needle) > 0 and string.endswith(needle):
        length = len(needle)
        string = string[:-length]
    return string


def multi_filter(names, patterns):
    """Generator function which yields the names that match one or more of the
       patterns."""
    for name in names:
        if any(fnmatch.fnmatch(name, pattern) for pattern in patterns):
            yield name


def super_filter(names, inclusion_patterns=('*',), exclusion_patterns=()):
    """ Enhanced version of fnmatch.filter() that accepts multiple inclusion
        and exclusion patterns.

    Filter the input names by choosing only those that are matched by
    some pattern in inclusion_patterns _and_ not by any in exclusion_patterns.
    taken from:
    http://codereview.stackexchange.com/questions/74713/filtering-with-multiple-inclusion-and-exclusion-patterns
    """
    included = multi_filter(names, inclusion_patterns)
    excluded = multi_filter(names, exclusion_patterns)
    return set(included) - set(excluded)
