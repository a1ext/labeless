# -*- coding: utf-8 -*
# Labeless
# by Aliaksandr Trafimchuk
#
# Source code released under
# Creative Commons BY-NC 4.0
# http://creativecommons.org/licenses/by-nc/4.0

__author__ = 'a1ex_t'

import os
import re
import sys
from os import path
from distutils.core import setup


curr_dir = path.abspath(path.dirname(path.realpath(__file__)))
cwd = os.getcwd()
os.chdir(curr_dir)


def _get_version():
    version_py = os.path.join(curr_dir, 'labeless', 'VERSION')

    if not os.path.isfile(version_py):
        version_h = path.join(curr_dir, '..', 'common', 'version.h')
        if not path.isfile(version_h):
            raise Exception('Invalid sources state. Either labeless/VERSION or ../common/version.h file should exists')

        with open(version_h, 'rb') as f:
            raw = f.read()

        res = [
            re.compile(r'^\s*#define\s+VERSION_MAJOR\s+(\d+)\s*$', re.I | re.U | re.M),
            re.compile(r'^\s*#define\s+VERSION_MINOR\s+(\d+)\s*$', re.I | re.U | re.M),
            re.compile(r'^\s*#define\s+VERSION_REVISION\s+(\d+)\s*$', re.I | re.U | re.M),
            re.compile(r'^\s*#define\s+VERSION_BUILD\s+(\d+)\s*$', re.I | re.U | re.M)
        ]
        rv = list()
        for pattern in res:
            m = pattern.search(raw)
            if not m:
                raise Exception('Pattern not found: %r' % pattern)
            rv.append(m.group(1))
        rv = '.'.join(rv)
        with open(version_py, 'wb') as f:
            f.write(rv)

    with open(version_py, 'r') as f:
        return f.read()


requirements = [
    'protobuf==2.6.1'
]


def _get_backends():
    backends_dir = path.join(curr_dir, 'labeless', 'backend')
    rv = list()
    for entry in os.listdir(backends_dir):
        fn = path.join(backends_dir, entry)
        if not path.isdir(fn):
            continue

        rv.append('labeless.backend.%s' % entry)

    return rv


def _get_long_description():
    if not path.isfile('README.rst'):
        import pypandoc
        rst = pypandoc.convert(path.join('..', 'README.md'), 'rst')
        with open('README.rst', 'wb') as f:
            f.write(rst)

    with open(r'README.rst', 'r') as f:
        return f.read()


setup(
    name='labeless',
    version=_get_version(),
    description='Labels/Comments synchronization between IDA PRO and dbg backend (OllyDbg1.10, OllyDbg 2.01, x64dbg)'
                ' , Remote memory dumping tool (including x64-bit), Python scripting tool',
    # long_description=_get_long_description(),
    author="Aliaksandr Trafimchuk",
    author_email='a13x4nd3r.t@gmail.com',
    url='https://github.com/a1ext/labeless',
    packages=[
         'labeless',
         'labeless.backend'
     ] + _get_backends(),
    install_requires=requirements,
    license="CC-BY-NC-4.0",
    keywords=['labeless', 'IDA PRO', 'OllyDbg', 'x64dbg', 'dumping', 'tracing', 'strings decryption', 'API resolving'],
    classifiers=[
        'Operating System :: OS Independent',
        'Programming Language :: Python :: 2.7',
        'Topic :: Security',
        'Topic :: Software Development :: Debuggers',
        'Topic :: Software Development :: Disassemblers',
        'Topic :: Utilities'
    ],
    include_package_data=True,
    package_data={
        'labeless': ['VERSION']
    }
)

os.chdir(cwd)
