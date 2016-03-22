# -*- coding: utf-8 -*
# Labeless
# by Aliaksandr Trafimchuk
#
# Source code released under
# Creative Commons BY-NC 4.0
# http://creativecommons.org/licenses/by-nc/4.0

__author__ = 'a1ex_t'

import os
from os import path
from distutils.core import setup
import sys

VERSION = None
try:
    with open(path.join(path.dirname(path.realpath(__file__)), 'labeless', 'VERSION'), 'rb') as f:
        VERSION = f.read().strip()
except:
    print >> sys.stderr, '[-] Do you forget to run prepare_version.py before?'
    exit(1)

requirements = [
    'protobuf==2.6.1'
]


def __get_backends():
    backends_dir = path.join(path.dirname(path.realpath(__file__)), 'labeless', 'backend')
    rv = list()
    for entry in os.listdir(backends_dir):
        fn = path.join(backends_dir, entry)
        if not path.isdir(fn):
            continue
        rv.append('labeless.backend.%s' % entry)
    return rv


setup(name='Labeless',
      version=VERSION,
      description=__doc__,
      author="Aliaksandr Trafimchuk",
      author_email='a13x4nd3r.t@gmail.com',
      url='https://github.com/a1ext/labeless',
      packages=[
          'labeless',
          'labeless.backend'] + __get_backends(),
      install_requires=requirements,
      license="CC-BY-NC-4.0",
      keywords='labeless')
