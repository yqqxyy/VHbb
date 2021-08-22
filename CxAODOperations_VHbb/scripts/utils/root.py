"""
ROOT overloads / extensions
"""

import os
import sys

argv = sys.argv  # SO
sys.argv = []  # UNNECESSARY
import ROOT

ROOT.gROOT.SetBatch(1)
ROOT.PyConfig.IgnoreCommandLineOptions = True
sys.argv = argv  # ROOT
# https://root-forum.cern.ch/t/pyroot-hijacks-help/15207


class safeTFile(ROOT.TFile):
    """
    Exactly like TFile, but now you can use it the pythonic way:

    Example:
        with safeTFile("myfile.root", "recreate") as rf:
            rf.mkdir("mydir")
            my_histogram.Write()
            # other stuff that happens in the file

    ROOT.TFile.Write() will be executed automatically if the file was opened
    with the "recreate" attribute.
    """

    def __init__(self, fname, option="", ftitle="", compress=1, dirtyclose=False):
        super(safeTFile, self).__init__(fname, option, ftitle, compress)
        self.writeOnExit = "recreate" in option.lower() or "update" in option.lower()
        # Closing the file the dirty way means removing it from the list of files
        # Only use this at the very end of every program and never re-use a file
        # that was closed "dirty"
        self.dirtyclose = dirtyclose

    def __enter__(self):
        if not self or self.IsZombie():
            path = os.path.dirname(self.GetName())
            if not os.path.isdir(path):
                raise IOError("Path does not exist: {}".format(path))
            elif not os.path.isfile(self.GetName()):
                raise IOError("File does not exist: {}".format(self.GetName()))
            else:
                raise IOError("Problems with ROOT file.")
        return self

    def __exit__(self, type, value, traceback):
        if self.writeOnExit:
            print("Writing " + self.GetName())
            self.Write()
        if self.dirtyclose:
            ROOT.gROOT.GetListOfFiles().Remove(self)
        else:
            self.Close()
