# -*- coding: utf-8 -*-
"""
(c) 2019 Andreas Hoenle (der.andi@cern.ch) for the benefit of the ATLAS collaboration
Taken from stackoverflow and slightly modified.
"""

import itertools
import sys
import time
import multiprocessing

HIDE_CURSOR = "\x1b[?25l"
SHOW_CURSOR = "\x1b[?25h"

active_spinner = None


def get_spinner():
    """
    Return the current active spinner
    Will only work as long there is only a single spinner
    """
    global active_spinner
    return active_spinner


class Spinner(object):
    start_symbol = u"→️‍".encode("utf8")
    # spinner_cycle = itertools.cycle(['-', '/', '|', '\\'])
    cycle = [u"⣾", u"⣷", u"⣯", u"⣟", u"⡿", u"⢿", u"⣻", u"⣽"]
    success_symbol = u"✔".encode("utf8")
    fail_symbol = u"✖".encode("utf8")
    spinner_cycle = itertools.cycle([f.encode("utf8") for f in cycle])

    def __init__(self, text=""):
        self.stop_running = multiprocessing.Event()
        self.text = text
        self.success = None
        self.spin_process = multiprocessing.Process(target=self.init_spin)
        self.printed_header = False
        global active_spinner
        active_spinner = self

    def print_header(self):
        """
        To be called externally only in case of additional print statements
        This way all the additional print statements will be between the header
        and the final line that includes the success/fail symbol
        """
        if not self.printed_header:
            sys.stdout.write(HIDE_CURSOR + self.start_symbol + self.text + "\n")
            sys.stdout.flush()
            self.printed_header = True

    def start(self):
        self.spin_process.start()

    def stop(self):
        self.stop_running.set()
        self.spin_process.join()
        global active_spinner
        active_spinner = None

    def __enter__(self):
        self.start()
        return self

    def __exit__(self, type, value, traceback):
        self.stop()

    def init_spin(self):
        while not self.stop_running.is_set():
            sys.stdout.write(HIDE_CURSOR + "\r")
            sys.stdout.write(self.spinner_cycle.next() + self.text)
            sys.stdout.flush()
            time.sleep(0.1)
        if self.success is None:
            # default assumes success
            self.success = True
        if self.success:
            sys.stdout.write(
                "\r" + self.success_symbol + self.text + SHOW_CURSOR + "\n"
            )
        else:
            sys.stdout.write("\r" + self.fail_symbol + self.text + SHOW_CURSOR + "\n")
