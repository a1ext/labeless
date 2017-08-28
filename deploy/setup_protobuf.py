import sys
import platform
import subprocess
from os import path
import re


REQUIRED_PROTOBUF_VER = '2.6.1'
# url = 'https://bootstrap.pypa.io/get-pip.py' # TODO
ver = platform.python_version_tuple()
if ver[0] != '2' or ver[1] != '7':
    print >> sys.stderr, 'Please use Python 2.7'
    exit(1)

curr_script_path = path.dirname(path.realpath(__file__))
get_pip_script = path.join(curr_script_path, 'get-pip.py')
pip_executable = path.join(sys.exec_prefix, 'Scripts', 'pip.exe')
easy_install = path.join(sys.exec_prefix, 'Scripts', 'easy_install.exe')
ver_re = re.compile(r'^Version: ([\d\.]+)', re.I | re.M)

try:
    import pip
except ImportError:
    print 'Insatlling pip...'
    cmd = [sys.executable, get_pip_script]
    p_get_pip = subprocess.Popen(cmd, shell=True, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
    out, err = p_get_pip.communicate()
    if p_get_pip.returncode != 0:
        print >> sys.stderr, 'Error: unable to install pip, stderr:', err
        exit(2)
    if not path.isfile(pip_executable):
        print >> sys.stderr, 'PIP installed, but pip.exe is not found in following location: %s' % pip_executable
        exit(3)

cmd_show_protobuf = [pip_executable, 'show', 'protobuf']
p_show_protobuf = subprocess.Popen(cmd_show_protobuf, shell=True, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
out, err = p_show_protobuf.communicate()
if p_show_protobuf.returncode == 0 and out.strip():
    m = ver_re.search(out)
    if m and m.group(1) == REQUIRED_PROTOBUF_VER:
        print 'Protobuf check is OK'
        exit(0)

egg_file = path.join(path.abspath(path.dirname(path.realpath(__file__))), 'protobuf-2.6.1-py2.7.egg')
p_install_protobuf = subprocess.Popen([easy_install, egg_file], shell=True)
p_install_protobuf.communicate()
if p_install_protobuf.returncode != 0:
    print >> sys.stderr, 'pip install failed, stderr: %s' % err
    exit(4)

p_show_protobuf = subprocess.Popen(cmd_show_protobuf, shell=True, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
out, err = p_show_protobuf.communicate()
print out
print >> sys.stderr, err
if p_show_protobuf.returncode == 0 and out.strip():
    m = ver_re.search(out.strip().replace('\r', ''))
    print 'Protobuf check is %s' % ('OK' if m and m.group(1) == REQUIRED_PROTOBUF_VER else 'FAILED')
else:
    print >> sys.stderr, 'pip exited with error status: %u' % p_show_protobuf.returncode


