# -*- coding: utf-8 -*
__author__ = 'AlexWMF'

import json
import os
import subprocess
import sys


LL_LOCAL_DIR = r'd:\!my\labeless_olly2'


class DeployLabelessToVM(object):
    CFG_PATH = os.path.join(os.path.dirname(os.path.realpath(__file__)), 'deploy.conf')

    __CFG_LOCAL_DIR = 'local_dir'
    __CFG_USERNAME = 'username'
    __CFG_PASSWORD = 'password'
    __CFG_FILES = 'files'
    __CFG_SRC_NAME = 'src_name'
    __CFG_DST = 'dst'
    __CFG_RUN = 'run'
    __CFG_NAME = 'name'
    __CFG_CMDLINE = 'cmdline'
    __CFG_SHELL = 'shell'

    __CFG_DEFAULT_USERNAME = 'Administrator'
    # vmrun -T ws -gu Administrator -gp %pass% CopyFileFromGuestToHost %VMX% %FILE_FROM% %FILE_TO%

    def __init__(self, vmx):
        super(DeployLabelessToVM, self).__init__()
        self.cfg = dict()
        self.local_dir = None
        self.bin_dir = None
        self.vmrun = None
        self.vmx = vmx

        self.load_config()
        self.find_vmrun()

    def load_config(self):
        if os.path.isfile(self.CFG_PATH):
            with open(self.CFG_PATH, 'rb') as f:
                self.cfg = json.load(f)

        self.local_dir = self.cfg.get(self.__CFG_LOCAL_DIR, LL_LOCAL_DIR)
        self.bin_dir = os.path.join(self.local_dir, 'bin')

    def find_vmrun(self):
        for program_files in ('Program Files', 'Program Files (x86)'):
            pth = os.path.join('c:\\', program_files, 'VMware', 'VMware VIX', 'vmrun.exe')
            if os.path.exists(pth):
                self.vmrun = pth
                break
        if not self.vmrun:
            raise Exception('vmrun not found :(')

    def delete_file(self, dst):
        """vmrun -T ws -gu Administrator -gp %pass% deleteFileInGuest %VMX% %FILE_PATH%"""

        cmd = [self.vmrun, '-T', 'ws', '-gu', self.cfg[self.__CFG_USERNAME], '-gp', self.cfg[self.__CFG_PASSWORD],
               'deleteFileInGuest', vmx, dst]
        p = subprocess.Popen(cmd, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
        out, err = p.communicate()
        if p.returncode != 0:
            # print >> sys.stderr, '[-] deleteFileInGuest failed, process exited with error code: %u' % p.returncode
            # print >> sys.stderr, '[-] stdout: %s\n[-] stderr: %s' % (out.strip(), err.strip())
            return

        return True

    def delete_dir(self, dst):
        """vmrun -T ws -gu Administrator -gp %pass% deleteDirectoryInGuest %VMX% %DIR_PATH%"""

        cmd = [self.vmrun, '-T', 'ws', '-gu', self.cfg[self.__CFG_USERNAME], '-gp', self.cfg[self.__CFG_PASSWORD],
               'deleteDirectoryInGuest', vmx, dst]
        p = subprocess.Popen(cmd, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
        _, _ = p.communicate()
        if p.returncode != 0:
            # print >> sys.stderr, '[-] deleteFileInGuest failed, process exited with error code: %u' % p.returncode
            # print >> sys.stderr, '[-] stdout: %s\n[-] stderr: %s' % (out.strip(), err.strip())
            return

        return True

    def deploy_file(self, src, dst):
        """vmrun -T ws -gu Administrator -gp %pass% CopyFileFromHostToGuest %VMX% %FILE_FROM% %FILE_TO%"""
        cmd = [self.vmrun, '-T', 'ws', '-gu', self.cfg[self.__CFG_USERNAME], '-gp', self.cfg[self.__CFG_PASSWORD],
               'CopyFileFromHostToGuest', vmx, src, dst]

        if os.path.isdir(src):
            self.delete_dir(dst)
        else:
            self.delete_file(dst)

        p = subprocess.Popen(cmd, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
        out, err = p.communicate()
        if p.returncode != 0:
            print >> sys.stderr, '[-] Process exited with error code: %u' % p.returncode
            print >> sys.stderr, '[-] stdout: %s\n[-] stderr: %s' % (out.strip(), err.strip())
            return

        return True

    def start_process_in_vm(self, name, cmdline, shell, wait=False):
        """vmrun -T ws -gu Administrator -gp %pass% RunProgramInGuest %VMX% -noWait %FILE_FROM%"""

        cmd = [self.vmrun, '-T', 'ws', '-gu', self.cfg[self.__CFG_USERNAME], '-gp', self.cfg[self.__CFG_PASSWORD],
               'RunProgramInGuest', vmx]

        if not wait:
            cmd.append('-noWait')
        cmd += cmdline

        p = subprocess.Popen(cmd, stdout=subprocess.PIPE, stderr=subprocess.PIPE, shell=shell)
        out, err = p.communicate()
        if p.returncode != 0:
            print >> sys.stderr, '[-] Process exited with error code: %u' % p.returncode
            print >> sys.stderr, '[-] stdout: %s\n[-] stderr: %s' % (out, err)
            return

        print '[+] process "%s" exited normally, out: %s' % (name, out)
        return True

    def deploy(self):
        for item in self.cfg[self.__CFG_FILES]:
            src_name = os.path.normpath(os.path.join(self.local_dir, item[self.__CFG_SRC_NAME]))
            dst = item[self.__CFG_DST]
            if not self.deploy_file(src_name, dst):
                print >> sys.stderr, '[-] deploy_file("%s", "%s") failed' % (src_name, dst)
            else:
                print '[+] OK deploy_file("%s", "%s")' % (src_name, dst)

        for item in self.cfg[self.__CFG_RUN]:
            name = item[self.__CFG_NAME]
            cmdline = item[self.__CFG_CMDLINE]
            shell = item[self.__CFG_SHELL] != 0
            if not self.start_process_in_vm(name, cmdline, shell, wait=True):
                print >> sys.stderr, '[-] start_process_in_vm("%s", %r) failed' % (name, cmdline)
            else:
                print >> sys.stderr, '[+] OK, start_process_in_vm("%s", %r)' % (name, cmdline)

        print '[*] Finished'


if __name__ == '__main__':
    if len(sys.argv) < 2:
        print >> sys.stderr, '[-] Usage: deploy_labeless_to_vm.py <vmx_path>'
        exit(1)

    vmx = sys.argv[1]
    deploy_to_vm = DeployLabelessToVM(vmx)
    deploy_to_vm.deploy()
    raw_input()
