# -*- coding: utf-8 -*-

"""
(c) 2019 Andreas Hoenle (der.andi@cern.ch)
    for the benefit of the ATLAS collaboration

    Create
        - yields.13TeV_sorted.txt
        - yields.13TeV_DAOD_sorted.txt
    in one go.

This runs parallelised.
Each subprocess gets its own Counter object and when they are done we join them.
"""

import os
import sys
import re

from argparse import ArgumentParser
from copy import copy
from multiprocessing import Pool

from utils.dirs import get_cxaod_root_subdirs
from utils.spinner import Spinner
from utils.root import ROOT, safeTFile


class Counter(object):
    """
    A counter for the CxAOD and DAOD entries in the CxAODs.

    We need:
        <DSID> <nEvents initial> <nSelectedOut> <nSumOfWeights>
    """

    _re_3tag = re.compile("^[esrp][0-9]+_[esrp][0-9]+_[esrp][0-9]+$")
    _re_4tag = re.compile("^[esrp][0-9]+_[esrp][0-9]+_[esrp][0-9]+_[esrp][0-9]+$")
    _re_5tag = re.compile(
        "^[esrp][0-9]+_[esrp][0-9]+_[esrp][0-9]+_[esrp][0-9]+_[esrp][0-9]+$"
    )
    _re_6tag = re.compile(
        "^[esrp][0-9]+_[esrp][0-9]+_[esrp][0-9]+_[esrp][0-9]+_[esrp][0-9]+_[esrp][0-9]+$"
    )

    def __init__(self, dsid, ami_tag, energy=13):
        self.energy = energy
        self.dsid = dsid
        self.n_initial = 0
        self.n_selout_entries_daod = 0
        self.n_selout_entries_cxaod = 0
        self.n_sum = 0.0
        self.ami_tag = ami_tag

    def cxaod_as_line(self):
        return "{:>10s} {:>15.0f} {:>15.0f} {:>30.6f}\n".format(
            self.dsid, self.n_initial, self.n_selout_entries_cxaod, self.n_sum
        )

    def dxaod_as_line(self):
        return "{:>10s} {:>15.0f} {:>15.0f} {:>30.6f} {:>30s}\n".format(
            self.dsid,
            self.n_initial,
            self.n_selout_entries_daod,
            self.n_sum,
            self.ami_tag,
        )

    def __str__(self):
        return "<Counter: {} TeV - DSID {} - inital {} - daod {} - cxaod {} - sum {}>".format(
            self.energy,
            self.dsid,
            self.n_initial,
            self.n_selout_entries_daod,
            self.n_selout_entries_cxaod,
            self.n_sum,
        )

    def __repr__(self):
        return self.__str__()

    def __add__(self, other):
        """
        Add the entries of all CxAODs in cxaod_dir

        Args:
            cxaod_dir: A directory with CxAOD ROOT files
        """
        if isinstance(other, str) and os.path.isdir(other):
            for cxaod in os.listdir(other):
                with safeTFile(os.path.join(other, cxaod)) as root_file:
                    meta_data = root_file.Get("MetaData_EventCount")
                    if not isinstance(meta_data, ROOT.TH1):
                        self.count_cxaod = -1
                        self.count_daod = -1
                        print(
                            "\rError: Invalid meta_data in {}".format(
                                root_file.GetName()
                            )
                        )
                        return self
                    n_initial = meta_data.GetBinContent(1)
                    n_selout_entries_daod = meta_data.GetBinContent(2)
                    n_selout_entries_cxaod = meta_data.GetBinContent(3)
                    n_sum = meta_data.GetBinContent(4)
                    if n_initial == 0:
                        # Use values from job instead of DAOD
                        n_initial = h.GetBinContent(7)
                        n_sum = h.GetBinContent(8)
                    self.n_initial += n_initial
                    self.n_selout_entries_daod += n_selout_entries_daod
                    self.n_selout_entries_cxaod += n_selout_entries_cxaod
                    self.n_sum += n_sum
        elif isinstance(other, Counter):
            assert self._is_compatible(other)
            print(
                "Merging counters for DSID {}: {}+{}".format(
                    self.dsid, self.ami_tag, other.ami_tag
                )
            )
            self.n_initial += other.n_initial
            self.n_selout_entries_daod += other.n_selout_entries_daod
            self.n_selout_entries_cxaod += other.n_selout_entries_cxaod
            self.n_sum += other.n_sum
            self.ami_tag = "+".join([self.ami_tag, other.ami_tag])
        return self

    def _is_compatible(self, other):
        """
        Check if two Counters are compatible.
        This check is up-to-date as of April 2019, but is not guaranteed to
        make the right checks for all time!
        """
        if self.dsid != other.dsid:
            print(
                "Error: Incompatible DSIDs: self: {}, other: {}".format(
                    self.dsid, other.dsid
                )
            )
            return False
        if self.energy != other.energy:
            print(
                "Error: Incompatible energy for DSID {}: self: {}, other{}".format(
                    self.dsid, self.energy, other.energy
                )
            )
            return False
        # If one of the two counters as already added
        # the two ami tags will be joined by a '+'
        for self_tag in self.ami_tag.split("+"):
            for other_tag in other.ami_tag.split("+"):
                if self_tag == other_tag:
                    print(
                        "Error: Overlappting ami tags for DSID {}: self: '{}', other: '{}'. Potential double counting.".format(
                            self.dsid, self.ami_tag, other.ami_tag
                        )
                    )
                    return False
            # assert that we are not trying to add a 3-tag and a 5-tag AMI tag
            # which would also be double counting
            # we can however add all the other possibilities
            # analyzers have to make sure that they are not double counting
            # this is why we still print a warning for now
            if self._re_3tag.match(self_tag):
                if self._re_5tag.match(other_tag):
                    print(
                        "Error: You're trying to add a 3tag sample {} together with an 5tag sample {} for DSID {}".format(
                            self_tag, other_tag, self.dsid
                        )
                    )
                    return False
            return True


def main(args):
    with Spinner(" Determine directories and subdirectories"):
        all_cxaod_dirs = get_cxaod_root_subdirs(".")

    print("Found {} CxAOD directories".format(len(all_cxaod_dirs)))

    with Spinner(" Waiting for subprocesses to finish"):
        pool = Pool(processes=args.nproc)
        counters = [r[0] for r in pool.imap_unordered(count_entries, all_cxaod_dirs)]
        pool.close()
        pool.join()

    # some sanity checks
    assert len(counters) == len(
        all_cxaod_dirs
    ), "len(counters) (={}) != len(all_cxaod_dirs) (={})".format(
        len(counters), len(all_cxaod_dirs)
    )
    merged_counters = merge_counters(counters)
    if not merged_counters:
        sys.exit(1)

    # write output
    write_output(counters, merged_counters)

    print(u"❤️  All done".encode("utf8"))


def write_output(counters, merged_counters):
    """
    As of April 2019 the yield files are created such that in
        yields.13TeV_sorted.txt
    DSIDs are merged into a single entry, while in
        yields.13TeV_DxAOD_sorted.txt
    they can appear multiple times, but with different AMI tags.
    For that reason we require two arguments:
        counters: The counter objects after, one object per sample
        merged_counters: The counter objects after merging counters with the same DSID
    """
    cxaod_name = "yields.13TeV_sorted.txt"
    dxaod_name = "yields.13TeV_DxAOD_sorted.txt"
    # sort by DSID
    counters = sorted(counters, key=lambda x: x.dsid)
    merged_counters = sorted(merged_counters, key=lambda x: x.dsid)
    # write the files
    with open(cxaod_name, "w") as yieldfile:
        for counter in merged_counters:
            yieldfile.write(counter.cxaod_as_line())
    print("Wrote {}".format(cxaod_name))
    with open(dxaod_name, "w") as yieldfile:
        for counter in counters:
            yieldfile.write(counter.dxaod_as_line())
    print("Wrote {}".format(dxaod_name))


def merge_counters(counters):
    """
    Merge the counters with the same DSID.
    The Counter.__add__ will perform a few sanity checks, but make sure that everything
    you need for your analysis is implement and you do not double count DSIDs.
    """
    merged_counters = []
    all_dsids = sorted(set([counter.dsid for counter in counters]))
    print(
        "merge: Found {} unique DSIDs in {} counters".format(
            len(all_dsids), len(counters)
        )
    )
    for dsid in all_dsids:
        to_merge = [counter for counter in counters if counter.dsid == dsid]
        if len(to_merge) == 1:
            merged_counters.append(to_merge[0])
            continue
        first = copy(to_merge[0])
        for other in to_merge[1:]:
            first += other
        merged_counters.append(first)
    print(
        "merge: Created {} merged counters for {}  unique DSIDs".format(
            len(merged_counters), len(all_dsids)
        )
    )
    assert len(merged_counters) == len(all_dsids), "Wrong number of counters"
    return merged_counters

    for i in range(len(counters)):
        for j in range(i):
            counter1 = counters[i]
            counter2 = counters[j]
            if counter1.dsid == counter2.dsid:
                duplicate_dsids.append(counter1.dsid)
                break
    for dsid in duplicate_dsids:
        print("Error: {} is a duplicate DSID".format(dsid))
    return len(duplicate_dsids) == 0


def count_entries(cxaod_dir):
    counters = []
    if isinstance(cxaod_dir, list):
        for entry in cxaod_dir:
            counters += count_entries(entry)
    dsid, ami_tag = get_info(cxaod_dir)
    counter = Counter(dsid, ami_tag)

    counter += cxaod_dir

    counters.append(counter)  # append is atomic, i.e. thread-safe
    return counters


def get_info(cxaod_dir):
    """Extract the DSID and AMI Tag from a directory name"""
    base = os.path.basename(cxaod_dir)
    try:
        dsid = base.split(".")[3]
        ami_tag = base.split(".")[5]
    except IndexError:
        print("Error: Not a valid CxAOD directory: {}".format(base))
        sys.exit(1)
    return dsid, ami_tag


if __name__ == "__main__":
    parser = ArgumentParser()
    parser.add_argument(
        "--nproc",
        "-j",
        help="Number of processes that will be started [default: 16]",
        default=16,
        type=int,
    )
    main(parser.parse_args())
