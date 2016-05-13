
Deployment automation
=====================

Automation script that can be found here ```deploy\deploy_labeless_to_vm.py```, may be used in order to deploy Labeless to the VMware virtual machine.

This script gets path to the ```.vmx``` file of target virtual machine, like ```m:\!vm\labeless_test\labeless_test.vmx```. To easily run the script, you can create a shortcut at Desktop or in TotalCommander bar and pass .vmx file as a parameter (you just have to drag .vmx file to the created shortcut).

Configuration for the script is located in ```deploy\deploy.conf``` JSON-like file.

The sample configuration is displayed below:

```json
{
  "local_dir": "d:\\!my\\labeless_olly2",
  "username": "Administrator",
  "password": "",
  "files": [
    {"src_name": "bin/labeless_olly.dll", "dst": "e:\\odbg110\\labeless_olly.dll"},
    {"src_name": "bin/labeless_olly_foff.dll", "dst": "e:\\DeFixed_Edition\\Plugins\\labeless_olly_foff.dll"},
    {"src_name": "bin/labeless_olly2.dll", "dst": "e:\\2.01\\plugins\\labeless_olly2.dll"},
    {"src_name": "deploy", "dst": "e:\\deploy"}
  ],
  "run": [
    {"name": "setup protobuf", "cmdline": ["c:\\python27\\python.exe", "e:\\deploy\\setup_protobuf.py"], "shell": 1},
    {"name": "setup labeless", "cmdline": ["c:\\python27\\python.exe", "e:\\deploy\\setup.py", "install"], "shell": 1}
  ]
}
```

```local_dir``` - local foder with Labeless release (or root sources dir). It used as base directory for ```files.src_name``` field.

```username``` and ```password``` is credentials of local user in your guest machine (**required!**)

```files``` - list of deploy operations, each has the following fields:

    * ```src_name``` - relative path from ```local_dir``` to the source file/directory
    * ```dst``` - absolute path of file/directory destination in your guest machine

```run``` - list of commands that will be executed after deployment of files/directories specified in ```files``` list. Each ```run``` command has the following fields:

    * ```name``` - name of command (just for displaying at the output)
    * ```cmdline``` - commandline, list of strings. Note: use the absolute paths here
    * ```shell``` - should always be ```1```



Troubleshooting
===============

* Check is you have ```vmrun.exe``` in one of the following locations:
```
c:\Program Files (x86)\VMware\VMware VIX\vmrun.exe
c:\Program Files (x86)\VMware\VMware Workstation\vmrun.exe
c:\Program Files\VMware\VMware VIX\vmrun.exe
c:\Program Files\VMware\VMware Workstation\vmrun.exe
```

Dependencies
============

* Python 2.7
* ```vmrun.exe``` from VMware VIX or VMWare Workstation
