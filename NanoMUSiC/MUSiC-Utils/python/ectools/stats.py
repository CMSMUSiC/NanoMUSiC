import numpy as np
from .misc import camel2lower
from .base import ECStyleHelper


class ECStatHelper(ECStyleHelper):

    @classmethod
    def ec_result_meta_dump(cls, result_dict):
        '''
            create a dump of metaiformation e.g. for import in viewer apps
        '''
        ec_keys = ("Name",
                   "ClassType",
                   "ObjectCounts",
                   "MCEventsClass",
                   "DataEventsClass",
                   "SkipReason")
        for key in ec_keys:
            ec_dict[ectools.misc.camel2lower(key)] = result_dict.get(key, 0)

        dist_dict = {}
        dist_dict["event_class"] = result_dict["Name"]
        dist_dict["n_signal_rounds"] = result_dict["DataCount"]
        dist_keys = ("PTildeValue",
                     "PValue",
                     "DataEventsRegion",
                     "LowerEdge",
                     "Width")
        for key in dist_keys:
            camel_key = ectools.misc.camel2lower(key)
            dist_dict[camel_key] = result_dict["DataRounds"].get(key, 0)

        return{"eventclass": ec_dict, "eventclasses_distributions": dist_dict}

    @classmethod
    def extend_metadata(cls, dist_list, meta_object_key, options):
        ''' Add a list of distributions to update and extend the existing
            metadata objects.
        '''
        run_id = os.path.basename(os.path.normpath(options.out))
        meta_file = os.path.join(options.out, run_id + ".runinfo")
        with open(meta_file, "r") as meta_file:
            meta_json = json.load(meta_file)
        meta_json["run_id"] = run_id
        meta_json[meta_object_key] = dist_list
        with open(meta_file, "w") as meta_file:
            json.dump(meta_file)

    @classmethod
    def calc_p_tilde(cls,
                     pseudo_p_value_list,
                     data_p_value,
                     correct_if_zero=False):
        """ Calculate a p-tilde value from a p-value and a list of pseudo
            p-values.
        """

        # Make sure we are dealing with numpy arrays (for comparison).
        pseudo_p_value_list = np.array(pseudo_p_value_list)

        # Make sure we actually have a list.
        if len(pseudo_p_value_list) == 0:
            return 1.0

        # Straightforward: count the instances where the pseudo p-value is
        # *smaller* than the data p.
        ptilde = float(np.count_nonzero(pseudo_p_value_list <= data_p_value))
        ptilde /= len(pseudo_p_value_list)
        # If correct_if_zero is set and no pseudo value was smaller than
        # the data p, return an upper bound on the p-tilde value.
        if ptilde == 0 and correct_if_zero:
            ptilde = 1.0 / (len(pseudo_p_value_list) + 1)

        return ptilde

    @classmethod
    def get_pseudo_p_tilde_list(cls, p_value_list, correct_if_zero=False):
        """ Calculate a p-tilde value by comparing each p-value in the list to
        all other p-values from the same list.

        The idea of the algorithm is to take a linspace object and assign each
        p-value a p-tilde value from this set. This naive approach could be
        implemented in one line, as follows:
        ptilde = linspace(0, 1, len(p))[p.argsort().argsort()]
        This approach fails when values are duplicated in the list. Because of
        the less-than-or-equals sign in our p-tilde equation:
        ptilde = #(p_pseudo <= p_data)/#p_pseudo,
        all duplicated values have to be assigned the same p-tilde value, which
        is the *last* one from the linspace.
        """

        # Make sure we are dealing with numpy arrays (for argsort)
        p_value_list = np.array(p_value_list)

        # Make sure we actually have a list.
        if len(p_value_list) == 0:
            return []

        # Determine duplicate p-values in the list. The actual unique p-value
        # list is never used, but the info unique_indices and unique_inverse.
        # 'unique_inverse' is an array that takes unique_list and converts it
        # back into the original list (by repeating the same index over).
        unique_inverse = np.unique(p_value_list, return_inverse=True)[1]

        # The return value with 'return_index=True' is the index of the first
        # appearance of each value. Since we are interested in the last
        # occurence, we call 'np.unique' again with the reversed array and then
        # "invert" the indices by subrating them from len(p_value_list) - 1.
        unique_indices = np.unique(p_value_list[::-1], return_index=True)[1]
        unique_indices = len(p_value_list) - unique_indices - 1

        # 'unique_indices' now contains the *last* appearance of each value,
        # as index relative to the p_value_list (thus has as many indices as
        # there are unique p-values).

        # Combining the two return values gives you a ranking where all
        # duplicate
        # indices are replaced by their first occurence index.
        # Example: p_value_list = [4, 2, 2, 3, 1, 5, 1]
        # --> unique_list = [1, 2, 3, 4, 5]
        # --> unique_indices = [4, 1, 3, 0, 5] (unique_list[0] = p_value_list[4], unique_list[1] = p_value_list[1], ...)
        # --> unique_inverse = [3, 1, 1, 2, 0, 4, 0] (p_value_list[0] = unique_list[3], p_value_list[1] = unique_list[1], ...)
        # ---> combination: [0, 1, 1, 3, 4, 5, 4]
        duplication_indices = unique_indices[unique_inverse]

        # Now we build the output domain: Assuming no special duplicate
        # handling, each value in the range of 0, 1 will occur once.
        n_pvals = len(p_value_list)
        segmented_range = np.linspace(0.0, 1.0, n_pvals, endpoint=True)

        # Apply the correction beforehand: the first entry (index=0) will be
        # given as upper limit (if correct_if_zero is set). Here we divide by N
        # instead of N+1, because one value is taken out of the array:
        # (N-1)+1 = N
        if correct_if_zero:
            segmented_range[0] = 1.0 / len(p_value_list)

        # argsort() gives the indices that would be needed to sort the
        # Because we are interested in the indices that turn a sorted array
        # into the original, we need an "inverse" of argsort.
        # So far, I have not found a better solution than calling 'argsort' on
        # the result again. This has been verified to work.
        # The result is a list of indices which have to be applied to the
        # correctly segmented range between 0 and 1, to obtain p-tilde-values.
        inverse_sorting_indices = p_value_list.argsort().argsort()

        # Apply first the inverse sorting on the output range, then apply the
        # special handling of duplicate entries.
        ptilde = segmented_range[inverse_sorting_indices][duplication_indices]

        # The result is exactly the same one would obtain when calling
        # calc_p_tilde on every single entry.
        return ptilde.tolist()
