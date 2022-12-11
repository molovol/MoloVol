---
title: macOS Download
filename: macOS.md
---

### Latest version – v1.1.0

Version for macOS El Capitan (10.11) and higher. The app natively supports Macs running on
Apple silicon, as well as older CPU architectures.

<div class="button download" markdown="1">
* <a class="buttons" 
    href="https://github.com/molovol/MoloVol/releases/download/v1.1.0/MoloVol_macOS-10.11+_v1.1.0.dmg">
    Download Installer
  </a>
</div>

#### Changes since v1.0.0

<div id="changelog" markdown="1">

# Added
* Added full support for charged atoms in structures, along with default ionic radii for common ions.

# Improved
* The error message that occurs when an incomplete list of input arguments are provided in the CLI is now more descriptive.
* Chemical formulas no longer contain the subscript "1" when an atom only occurs once.
* The molecular volume is now also displayed in the results summary.
* Reports now include information to find results relevant to porous materials and cavities in cage compounds.

# Fixed
* Fixed an issue where the application would crash, when an invalid elements file was provided while attempting to load a structure file.
* Fixed an issue where some atoms were rarely counted twice inside unit cells, thus generating the wrong chemical formula.

</div>

Previous versions can be downloaded in the [Releases Section](https://github.com/molovol/MoloVol/releases) 
of our GitHub page.

### Installation
The installation file comes in the form of a dmg file. Mac users will likely be familiar with 
this type of installer. Open the file and drag MoloVol into your applications folder.

When opening MoloVol for the first time, a warning may appear saying that the application is 
from an unknown developer (see image). This is because Apple requires a fee to registered as trusted developer. 
To get around the warning, you can navigate to your "Applications" folder and find the MoloVol 
executable. Right-click or control-click the executable and select "Open". You will be prompted 
with a dialog box where you will need to select "Open" and add MoloVol as an exception. 
[Apple's support website](https://support.apple.com/en-ie/guide/mac-help/mh40616/mac) provides a 
more detailed guide.

<img src="/docs/assets/images/macOS_error.png" width="200">