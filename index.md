<img class="title" src="/docs/assets/images/wordmark.png" alt="MoloVol wordmark" height="72">

MoloVol is a free, cross-plattform, scientific software for volume and surface computations of single molecules and crystallographic unit cells. We aim to combine an intuitive and easy-to-use user interface with powerful computing algorithms. We want the software to serve a broad range of scientist by being easily accessible on all common platforms.

The software is being maintained by Jasmin B. Maglic and Roy Lavendomme and developed on Windows 10, macOS Monterey, and Ubuntu.

## Scientific Reference
MoloVol v1.0.0 features and algorithms are presented in detail in an [open access scientific article](https://doi.org/10.1107/S1600576722004988) published in the *Journal of Applied Crystallography*.

Please use the following or an equivalent citation:

> Maglic, J.B. & Lavendomme, R. (2022). J. Appl. Cryst. 55, 1033-1044. DOI: [10.1107/S1600576722004988](https://doi.org/10.1107/S1600576722004988)

## Download

Compiled binaries of current and previous versions for Windows, macOS and Linux as well as a quick-start guide and the user manual can be found in the [Download Installers](https://github.com/molovol/MoloVol/releases) section.

New features and changes the current and all previous releases can be found in the [Changelog](https://github.com/molovol/MoloVol/blob/master/CHANGELOG.md).

The source code of the current development build can be found in the [Github Repository](https://github.com/molovol/MoloVol).

The source code for each stable release can be found [with the installers](https://github.com/molovol/MoloVol/releases) as .zip or .tar.gz archives.

## Installation
### macOS
You can download the application binary as .dmg file. Opening that file will mount it and open a window containing the application and your system's application folder. To install the application, simply drag it into the application folder.

When opening MoloVol for the first time, a warning may appear saying that the application is from an unknown developer. This is because Apple requires a fee to registered as trusted developer. To get around the warning, you can navigate to your "Applications" folder and find the MoloVol executable. Right-click or control-click the executable and select "Open". You will be prompted with a dialog box where you will need to select "Open" and add MoloVol as an exception. [Apple's support website](https://support.apple.com/en-ie/guide/mac-help/mh40616/mac) provides a more detailed guide.

<img src="/docs/assets/images/macOS_error.png" width="200">

### Linux
You can download an installation file as .deb file. This file format is used on Debian and Ubuntu. Installation for these systems should be as simple as opening the file; however, if you run into issues you can also run the installation from the command line.
```
$ dpkg -i bin/MoloVol_debian_version.deb 
```

Please note that for older versions we only provide the .deb file for arm64 CPU architectures. As such, it is possible that the installer will not work for you. In that case you could compile the application yourself by following [this guide](https://github.com/molovol/MoloVol/wiki/Getting-started-on-Linux).
### Windows
On windows, the application does not require an actual setup. The executable file and input files are all provided in a zip folder. Extract all files in any desired directory (note that "MoloVol.exe" and the "inputfile" folder should remain in the same directory) then start the program by running MoloVol.exe.

Depending on the version of windows and the security settings, a security warning might appear because MoloVol does not have a registered certificat (they are costly). You may proceed safely (unless you obtained the program from another source than here).
## Getting Help
If you encounter a bug or have a request for a feature, you are more than welcome submit an issue to the project's [Issues](https://github.com/jmaglic/MoloVol/issues) page on GitHub.

## Development Team
The development is currently lead by Jasmin B. Maglic and Roy Lavendomme. You may reach us via email through [molovol@outlook.com](mailto:molovol@outlook.com)

