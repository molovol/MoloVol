# MoloVol<img src="https://user-images.githubusercontent.com/65410083/99060370-3a6ab980-25a0-11eb-8f39-92e7af993223.png" width="250" ALIGN="right">

MoloVol is a free, cross-platform, scientific software for volume and surface computations of single molecules and
crystallographic unit cells.

You are welcome to visit [the project's website](https://molovol.com)

## Utility

With MoloVol we aim to combine an intuitive and easy-to-use user interface with powerful computing algorithms. We want
the software to serve a broad range of scientist by being easily accessible on all common platforms.

Version 1.0 of MoloVol has now been released. The software will continue to be maintained and actively developed on
Windows 10, macOS, Ubuntu, and web.

## Getting Started

### Desktop application

You can download the current version and older versions under [Releases](https://github.com/molovol/MoloVol/releases).
Installers are available for different operating systems. If you simply want to use MoloVol, then this is the way to go.

For installation on __macOS__ you should download the .dmg file. Opening that file will mount it and open a window
containing the application and your system's application folder. To install the application, simply drag it into the
application folder.

When opening MoloVol for the first time, a warning may appear saying that the application is from an unknown developer.
This is because Apple requires a fee to registered as trusted developer. To get around the warning, you can navigate to
your "Applications" folder and find the MoloVol executable. Right-click or control-click the executable and select "
Open". You will be prompted with a dialog box where you will need to select "Open" and add MoloVol as an
exception. [Apple's support website](https://support.apple.com/en-ie/guide/mac-help/mh40616/mac) provides a more
detailed guide.

On __Debian__ and __Ubuntu__ you should download the .deb file appropriate for your CPU architecture (*x86-64* or *arm64*). Installation should be as simple as opening the file; however, if you run into issues you can also run the
installation from the command line.

```
$ dpkg -i bin/MoloVol_debian_version.deb 
```

### Compiling the source code

You can find the source code for each release under [Releases](https://github.com/molovol/MoloVol/releases) in a .zip or
.tar.gz file. For detailed guides on how to compile the source code yourself, visit
the [MoloVol wiki](https://github.com/molovol/MoloVol/wiki). Compiling the code yourself allows you to modify the
software for your own purposes or propose changes to the developers and take part in development.

Dependencies needed for compilation:

- Any C++ compiler
- [wxWidgets 3.1.5](https://www.wxwidgets.org)

### MoloVol Web

Instead of using the desktop front-end, you can also use a web interface. MoloVol server provides a REST-API with a web
front-end wrapping the molovol CLI interface. To launch, first change the FLASK_APP environment variable by executing 
the command `export FLASK_APP=./webserver/app.py` from the project's root directory. Then execute `flask run`.
For hosting on a web server check out the next section.

### Containerized application

Instead of compiling or running the binaries you can also use a containerized version (using e.g. docker or podman) to
access the CLI or web interface.
In order to create a container you first need to obtain a Docker image for your operating system. A build can be found
at [dockerhub](https://hub.docker.com/r/bsvogler/molovol) or be built locally. Dockerfiles are in the 'container' directory. If you build locally replace the `bsvogler/molovol` with your local image name.

- For a short-lived container: Pass the CLI arguments in the run command:  `docker run -it bsvogler/molovol ./launch_headless.sh <yourMolovolArguments>`
- To run web application http://localhost:5001: run `docker run -dt -p 5001:5000 bsvogler/molovol`. When not otherwise specified the default port is 5000.

## Getting Help

If you wish to report a bug or request a feature go to the project's [GitHub issue tracker](https://github.com/molovol/MoloVol/issues).

## Development Team
The development is currently led by Jasmin B. Maglic and Roy Lavendomme. You may reach us via email through molovol@outlook.com
