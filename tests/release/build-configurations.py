#!/usr/bin/env python3

# Copyright 2019 The Imaging Source Europe GmbH
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
# http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

import sys
import os
import shutil
import argparse
import subprocess
import re
import logging


log = logging.getLogger(__name__)


# variables

builddir = os.path.curdir
srcdir = None

# add options here,
# rest does not have to be touched
build_options = [
    "-DBUILD_ARAVIS",
    "-DBUILD_V4L2",
    "-DBUILD_LIBUSB",
    "-DBUILD_TOOLS"
]

on = '=ON'
off = '=OFF'


def find_git_root():
    """
    Returns a string containing the absolut path to the tiscamera directory.
    Assumes a git checkout has been done.
    """
    repo_dir = subprocess.Popen(['git', 'rev-parse', '--show-toplevel'],
                                stdout=subprocess.PIPE).communicate()[0].rstrip().decode('utf-8')

    return repo_dir


def create_folder_name(args):
    """
    args - list of strings describing the selected cmake options
    returns: string containing a unique folder name based on the given args
    """
    name = ""
    for arg in args:

        if on in arg:
            res = re.search("_(.*)=", arg)
            name += res.group(1).lower()
            name += "_"

    if name == "":
        name = "no_options"

    return name


def delete_folder_content(folder):
    """
    folder that shall be emptied
    """
    for the_file in os.listdir(folder):
        file_path = os.path.join(folder, the_file)
        try:
            if os.path.isfile(file_path):
                os.unlink(file_path)
            elif os.path.isdir(file_path):
                shutil.rmtree(file_path)
        except Exception as e:
            print(e)


def setup_logger(name, log_file, level=logging.INFO):
    """Function setup as many loggers as you want"""

    handler = logging.FileHandler(log_file)
    formatter = logging.Formatter('%(message)s')
    handler.setFormatter(formatter)

    logger = logging.getLogger(name)
    logger.setLevel(level)
    logger.addHandler(handler)

    return logger


def execute_build(args):
    """
    args - list of strings containing the cmake options
    """

    print(args)
    # check and create build dir

    folder_name = create_folder_name(args)
    folder = os.path.join(os.path.abspath(builddir), folder_name)

    if os.path.isdir(os.path.abspath(folder)):
        delete_folder_content(os.path.abspath(folder))
    else:
        os.makedirs(os.path.abspath(folder))

    # change to dir

    os.chdir(os.path.abspath(folder))

    log_file = os.path.abspath(os.path.join(folder, "build.log"))

    logger = setup_logger(folder_name, log_file)

    def log_subprocess_output(pipe):
        for line in iter(pipe.readline, b''):  # b'\n'-separated lines
            logger.info('%s', line.decode('utf-8').replace("\n", ""))

    # call cmake with args

    cmake_process = subprocess.Popen(["cmake"] + args + [srcdir],
                                     stdout=subprocess.PIPE, stderr=subprocess.STDOUT)
    with cmake_process.stdout:
        log_subprocess_output(cmake_process.stdout)

    # check cmake result
    ret = cmake_process.wait()
    if ret != 0:
        print("CMake returned {}. Not building".format(ret))
        os.chdir(builddir)

        return

    # call make

    make_process = subprocess.Popen("make -j4", shell=True,
                                    stdout=subprocess.PIPE, stderr=subprocess.STDOUT)
    with make_process.stdout:
        log_subprocess_output(make_process.stdout)

    # check make result
    ret = make_process.wait()
    if ret != 0:
        print("Make returned {}. This is considered an error.".format(ret))

    os.chdir(builddir)


def toggle_option(args, current_index):
    """
    Toggles the build_options[current_index]
    between ON/OFF. If all options are included a build will be triggered
    if not the next option will be selected
    Arguments:

    args - list of current arguments for cmake
    current_index - int containing the list index that shall be used
    """

    arg = build_options[current_index]

    base_list = args

    if current_index == len(build_options) - 1:
        execute_build(base_list + [arg + on])
        execute_build(base_list + [arg + off])
    else:
        toggle_option(base_list + [arg + on], current_index+1)
        toggle_option(base_list + [arg + off], current_index+1)


def main():

    parser = argparse.ArgumentParser(description="Helper script to build all possible build configurations. ")
    parser.add_argument("--builddir",
                        help="Base dir in which configurations shall be built. Default=current dir",
                        action="store", dest="builddir", default=os.path.curdir)
    parser.add_argument("--srcdir",
                        help="tiscamera root dir that shall be used. Default=git root",
                        action="store", dest="srcdir", default=find_git_root())

    arguments = parser.parse_args()

    if not arguments.srcdir:
        print("Unable to find tiscamera root. Use --srcdir to manually specify it.")
        return 1

    global srcdir
    srcdir = arguments.srcdir

    global builddir
    builddir = os.path.abspath(arguments.builddir)

    index = 0

    toggle_option([], index)

    return 0


if __name__ == "__main__":
    sys.exit(main())
