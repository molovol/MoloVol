---
title: Linux download
filename: linux.md
---

### Latest version – v1.1.0

Installers for Ubuntu/Debian. Download the installer that corresponds to your CPU architecture.

<div class="button download" markdown="1">
* <a class="buttons" 
    href="https://github.com/molovol/MoloVol/releases/download/v1.1.0/MoloVol_debian_arm64_v1.1.0.deb">
    Installer for arm64
  </a>
</div>

{% include manualdownload.html %}

#### Changes since v1.0.0

{% include latestchanges.html %}

Previous versions can be downloaded in the [Releases Section](https://github.com/molovol/MoloVol/releases) 
of our GitHub page.

### Installation
Installation files are provided in form of a deb file. deb files can be used on Ubuntu and Debian distributions.
Installation for these systems should be as simple as opening the file; however, if you run into issues you 
can also run the installation from the command line.
```
$ dpkg -i bin/MoloVol_debian_version.deb 
```

Please note that for older versions we only provide the .deb file for arm64 CPU architectures. As such, it 
is possible that the installer will not work for you. In that case you could compile the application yourself 
by following [this guide](https://github.com/molovol/MoloVol/wiki/Getting-started-on-Linux).
