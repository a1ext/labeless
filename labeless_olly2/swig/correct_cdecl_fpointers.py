#!/usr/bin/env python
# -*- coding: utf-8 -*-

#
#    correct_cdecl_fpointer.py - Correct the declaraction of __cdecl function pointers
#    Copyright (C) 2013 Axel "0vercl0k" Souchet - http://www.twitter.com/0vercl0k
#
#    This program is free software: you can redistribute it and/or modify
#    it under the terms of the GNU General Public License as published by
#    the Free Software Foundation, either version 3 of the License, or
#    (at your option) any later version.
#
#    This program is distributed in the hope that it will be useful,
#    but WITHOUT ANY WARRANTY; without even the implied warranty of
#    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#    GNU General Public License for more details.
#
#    You should have received a copy of the GNU General Public License
#    along with this program.  If not, see <http://www.gnu.org/licenses/>.
#

import sys

def main(argc, argv):
    if argc < 2:
        print >> sys.stderr, 'correct_cdecl_fpointers.py <xxx_wrap.c>'
        return 1
    path = argv[1]
    r = open(path).read()
    patterns = [
        ('int (*arg4)(__cdecl *)', 'int (__cdecl *arg4)'),
        ('= (int (*)(__cdecl *)', '= (int (__cdecl *)')
    ]
    for old, new in patterns:
        r = r.replace(old, new)

    # Somehow in Readmemory there is a typo:
    #     arg1 = (char *) buf
    # is generated instead of
    #     arg1 = (char*) buf1
    r = r.replace('arg1 = (char *) buf;', 'arg1 = (char*) buf1;')
    r = r.replace('arg3 = (uchar *) buf;', 'arg3 = (uchar *) buf3;')
    open(path, 'w').write(r)
    return 0

if __name__ == '__main__':
    sys.exit(main(len(sys.argv), sys.argv))