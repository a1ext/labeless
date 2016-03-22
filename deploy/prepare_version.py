# -*- coding: utf-8 -*
# Labeless
# by Aliaksandr Trafimchuk
#
# Source code released under
# Creative Commons BY-NC 4.0
# http://creativecommons.org/licenses/by-nc/4.0

__author__ = 'a1ex_t'

from os import path
import re


def prepare_version():
    version_h = path.join(path.dirname(path.realpath(__file__)), '..', 'common', 'version.h')
    version_py = path.join(path.dirname(path.realpath(__file__)), 'labeless', 'VERSION')
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
    return rv

if __name__ == '__main__':
    prepare_version()
    print 'version is file prepared successfully'
