"""
(c) 2019 Andreas Hoenle (der.andi@cern.ch) for the benefit of the ATLAS collaboration
Directories for the VHreso project
"""

import os
import sys

from log import get_logger

log = get_logger(os.path.basename(__file__))


def get_project_dir():
    """
    Get the project directory root
    """
    workdir_dir = os.getenv("WorkDir_DIR")
    if not workdir_dir:
        log.error(
            "WorkDir_DIR is not set. Cannot continue. Did you forget to do 'asetup' or 'source x86_64-slc6-gcc62-opt/setup.sh'?"
        )
        sys.exit(1)
    root = os.path.abspath(os.path.join(workdir_dir, "..", ".."))
    return root


def get_build_dir():
    path = os.path.join(get_project_dir(), "build")
    if not os.path.isdir(path):
        log.error("No such file or directory: %s", path)
        return ""
    return path


def get_run_dir():
    path = os.path.join(get_project_dir(), "run")
    if not os.path.isdir(path):
        log.error("No such file or directory: %s", path)
        return ""
    return path


def get_source_dir():
    path = os.path.join(get_project_dir(), "source")
    if not os.path.isdir(path):
        log.error("No such file or directory: %s", path)
        return ""
    return path


def get_cxaod_root_subdirs(path):
    """
    Get all subdirectories until we find the first dir that starts with either
    group* or user* -> those are the CxAOD dirs and we don't want to step into
    them for performance reasons (thousands of subdirectories/files)

    Download prepare directories (dirname starting with prepare_*) are ignored

    Args:
        path: Top directory from which recursive search starts

    Returns:
        A list with all subdirectories that match the pattern
    """
    log.debug("Entering dir %s", path)
    if not os.path.isdir(path):
        log.error("Path %s is not a directory", path)
        return []
    if os.path.basename(path).startswith("prepare_"):
        log.debug("Path %s seems to be a prepare download directory. Ignoring.")
        return []
    cxaod_dirs = []
    subdirs = [d for d in os.listdir(path) if os.path.isdir(os.path.join(path, d))]
    log.debug("Considering subdirs %s", subdirs)
    for dir_entry in subdirs:
        if dir_entry.startswith("user") or dir_entry.startswith("group"):
            log.debug(
                "Found file %s which is a CxAOD .root dir. Returning %s",
                dir_entry,
                str(cxaod_dirs),
            )
            cxaod_dirs.append(os.path.join(path, dir_entry))
        new_path = os.path.join(path, dir_entry)
        cxaod_dirs += get_cxaod_root_subdirs(new_path)
    log.debug("Found CxAOD dirs: %s", str(cxaod_dirs))
    return cxaod_dirs
