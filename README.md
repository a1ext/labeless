Contributed By Check Point Software Technologies LTD.

Description
===========
Labeless is a plugin system for dynamic, seamless and realtime synchronization between IDA Database and Olly. It consists of two parts: IDA plugin and OllyDbg plugin.

Labeless significantly reduces time that researcher spends on transferring already reversed\documented code information from IDA (static) to debugger (dynamic). It saves time, preventing from doing the same job twice. Also, you can document and add data to the IDB on the fly and your changes will be automatically propagated to Olly, even if you will restart the virtual machine or instance of Olly will crash. So, you will never lose your research.

This solution is highly upgradable. You can implement any helper scripts in Python on OllyDbg side and then just call them from IDA with one line of code, parsing the results and automatically propagating changes to IDB.

It features:
* Seamless synchronization of labels, function names, comments and global variables syncing with demangling
* Synchronization modes
    * On demand
    * On rename (update on-the-fly)
* Supports image base-independent synchronization

Also, we provide dynamic dumping of debugged process memory regions functionality. It can be useful in the following cases:

-	When debugged process has extracted/temporary/injected module which doesn't appear in modules list
-	When it doesn't have a valid PE header
-	When it have corrupted import table, etc.

We can take that memory region and put it in the IDB, fixing imports 'on-the-fly', using OllyDbg functionality. No more need in ImpRec or BinScylla, searching for the regions in memory that contain the real IAT, because we get that information dynamically from the debugged process itself.

As a result we have a lot of memory regions that may represent even different modules (if the unpacking process if multistage) with valid references between them, which gives us a possibility to build a full control flow graph of the executable. Basically, we will end up with one big IDB, containing all the info on the specific case.

## Virus Bulletin 2015
* [Presentation](https://www.youtube.com/watch?v=bMQlu-lL6oY)
* [Slides](https://github.com/a1ext/labeless/blob/master/vb2015_presentation/vb2015_labeless.pptx)
* Dumping multiple injections video on [YouTube](https://youtu.be/M5K5Ldaq284)
* Python scripting video on [YouTube](https://youtu.be/SkcM8Hz2dT4)
* Basic labels sync video on [YouTube](https://youtu.be/iqipmqE2Znk)


Installation
===========
## Dependencies

* Python 2.7
* protobuf 2.6.1
* Visual Studio 2010 + Qt 4.8.4 (built with "QT" namespace) - required by IDA-side plugin (to proper use IDA's Qt). You can configure Qt by yourself with the following command:

  ```configure -platform win32-msvc2010 -shared -release -no-webkit -opensource -no-qt3support -no-phonon -no-phonon-backend -opengl desktop  -nomake demos -nomake examples -nomake tools -no-script -no-scripttools -no-declarative -qtnamespace QT```

* Visual Studio 2012 (or newer) to build Olly-side plugin

## IDA part:
 * Copy IDA plugin ```IDA\plugins\labeless_ida.plw``` to IDA's _plugins_ directory, for example ```c:\IDA68\plugins```

## Olly part:
 * Copy both ```Olly\get-pip.py``` and ```Olly\setup_protobuf.bat``` files to guest machine, then run ```setup_protobuf.bat``` and wait for the successful installation
 * Copy ```Olly\Plugins\labeless_olly.dll``` to OllyDbg _plugins_ directory. If you want to use Labeless with Olly FOFF mod (aka DeFixed edition), please use the plugin from the following path: ```Olly\Plugins\labeless_olly_foff.dll```

 * Copy the whole directory ```Olly\python``` to OllyDbg _home_ directory

# Checking if everything works
 * Start Olly and check for _Labeless_ item presence in _Plugins_ menu. If there is any problem, then check Olly's log window for details.
 * Start working with existing IDA database or use '_Labeless -> Load stub database..._' from the menu
 * Open Labeless settings dialog using menu '_Edit -> Plugins -> Labeless_'. You can use main menu '_Labeless -> Settings..._' or using hotkey ```Alt+Shift+E``` as well
 * Enter IP address and port of the guest machine. Click on '_Test connection'_ button.
 * If IDA displays the message '_Successfully connected!_', then configuration is done correctly.

# How to use
 * If you want to sync labels (names) from IDA to Olly you should check '_Enable labels & comments sync_' in Labeless settings dialog in IDA. There is one required field called '_Remote module base_', which should be set to the current module base of the analyzed application. You can find out that information in the debugger (Olly).
 * Select needed features, like _Demangle name_, _Local labels_, _Non-code names_
 * If you want to sync labels right now - press '_Sync now_' button. Labeless will sync all found names in your IDB with Olly. Settings dialog will be automatically closed, while saving all settings
 * If you want to customize settings for IDADump engine, do it in the '_IDADump_' tab.
 * Click on '_Save & Close_'

# Things automatically performed in the background
 * If you enabled '_Enable labels & comments sync_' option, then Labeless will automatically synchronize all the data on any rename operation in IDA

# Download
[Download Labeless 1.0.0.7 (include IDA 6.6 build)](https://github.com/a1ext/labeless/releases/download/v_1_0_0_7/Labeless.v.1.0.0.7_with_IDA66_build.zip)
