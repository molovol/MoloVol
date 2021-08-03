# Changelog

This changelog keeps track of changes in a user-friendly way. It is based on [keep a changelog](https://keepachangelog.com/en/1.0.0/) by Olivier Lacan.

## Unreleased
### Added
* Allow CIF files for structure input.
* Add variants of selected space groups with rhombohedral/hexagonal or Origin-2 settings in the space_groups definition file.
* In two probe mode, cavity types such as Tunnels, Pockets, and Isolated Cavities can now be distinguished.
* Cavity list output now hides unneeded columns, both in GUI and CLI.

### Changed
* Default grid resolution is now 0.2 A instead of 0.1 A.

## [v0.2.0](https://github.com/jmaglic/MoloVol/releases/tag/v0.2.0) - 2021-07-11

### Added
* The name of the output files from the automatic export option now include the name of the input structure file.
* Fully functional command line interface that allows accessing all functionalities directly form the command line.
* Macroscopic volume and surface area values are now provided in the report.
* Unit cell volume fractions are now provided in the report.
* Add drop down menu with common values to probe radius input text box.

### Fixed
* Copy-pasting file paths can now be done, by pressing enter inside the text box after pasting.

## [v0.1.0](https://github.com/jmaglic/MoloVol/releases/tag/v0.1.0) - 2021-06-17
* First beta release
