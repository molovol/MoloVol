# MoloVol<img src="https://user-images.githubusercontent.com/65410083/99060370-3a6ab980-25a0-11eb-8f39-92e7af993223.png" width="250" ALIGN="right">

MoloVol is a free, cross-platform, scientific software for volume and surface computations of single molecules and crystallographic unit cells.

## Utility
With the MoloVol software we aim to combine an intuitive and easy-to-use user interface with powerful computing algorithms. We want the software to serve a broad range of scientist by being easily accessible on all common platforms.

The MoloVol software, along with many of its features, is currently in pre-release. The software is being actively developed on Windows 10, macOS, Ubuntu, and web.

## Getting Started

### Desktop application

You can find compiled binaries and installation files for different operating systems under [Releases](https://github.com/molovol/MoloVol/releases).

For detailed guides on compiling the source code visit the MoloVol wiki (https://github.com/molovol/MoloVol/wiki).

Dependencies:
- wxWidgets 3.1.5 (https://www.wxwidgets.org)

### Web application
MoloVol also provides a REST-API with a web front-end built on-top of the desktop application. Instead of using the desktop front-end, you can deploy it using docker or podman. 

To start the container use docker or podman on port 5001 `podman run -dt -p 5001:5000/tcp <image>`. The web front-end is then available at http://localhost:5001. The default port is 5000. Internally flask is used as an interface to the CLI. You can also pass the CLI arguments in the run command for a short-lived container.


## Getting Help
If you wish to report a bug or request a feature go to the project's GitHub:
https://github.com/molovol/MoloVol/issues

## Development Team
The development is currently lead by Jasmin B. Maglic and Roy Lavendomme. You may reach us via email through molovol@outlook.com


