---
title: Linux download
filename: linux.md
---

### Latest version – v1.1.1

Installers for Ubuntu/Debian. Download the installer that corresponds to your CPU architecture.
If you are unsure, you can find your machine's architecture by running the following command in your Terminal.

```
$ dpkg-architecture -q DEB_BUILD_ARCH
```

<table class="invisible">
<tr>
<td>
<div class="button download" markdown="1">
* <a class="buttons" 
    href="https://github.com/molovol/MoloVol/releases/download/v1.1.1/MoloVol_debian_arm64_v1.1.1.deb">
    Installer for arm64
  </a>
</div>
</td>
<td>
<div class="button download" markdown="1">
* <a class="buttons" 
    href="https://github.com/molovol/MoloVol/releases/download/v1.1.1/MoloVol_debian_amd64_v1.1.1.deb">
    Installer for x86_64
  </a>
</div>
</td>
<td>
<div class="button download" markdown="1">
* <a class="buttons" 
    href="https://github.com/molovol/MoloVol/releases/download/v1.1.1/MoloVol_debian_amd64_v1.1.1.deb">
    Installer for amd64
  </a>
</div>
</td>
</tr>
</table>

{% include manualdownload.html %}

#### Changes since v1.0.0

{% include latestchanges.html %}

Previous versions can be downloaded in the [Releases Section](https://github.com/molovol/MoloVol/releases) 
of our GitHub page.

### Installation
Installation files are provided in form of a deb file. deb files can be used on Ubuntu and Debian distributions.
Installation for these systems should be as simple as opening the file with Software Center; however, if you run 
into issues you can also run the installation from the command line.
```
$ dpkg -i bin/MoloVol_debian_version.deb 
```

Please note that for older versions we only provide the .deb file for arm64 CPU architectures. As such, it 
is possible that the installer will not work for you. In that case you could compile the application yourself 
by following [this guide](https://github.com/molovol/MoloVol/wiki/Getting-started-on-Linux).

#### Known issues
When attempting an installation you may get a warning about the package being **Bad Quality**. It is unclear
why this happens, as the deb files are checked using Lintian before distribution. If you wish to install
the package regardless you can do so using via the command line as described above.

Older Linux distributions may be using **outdated libraries**. If you cannot upgrade the library version your best
chance is to compile the application yourself.

