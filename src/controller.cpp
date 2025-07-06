#include "controller.h"
#include "atom.h"
#include "model.h"
#include "misc.h"
#include "exception.h"
#include "special_chars.h"
#include "griddata.h"
#include "container3d.h"
#include "voxel.h"
#include <chrono>
#include <utility>
#include <map>

#ifdef MOLOVOL_GUI
#include "base.h"
#endif

///////////////////////
// STATIC ATTRIBUTES //
///////////////////////

Ctrl* Ctrl::s_instance = NULL;
#ifdef MOLOVOL_GUI
MainFrame* Ctrl::s_gui = NULL;
#endif

///////////////////////////
// STATIC MEMBERS ACCESS //
///////////////////////////

std::string Ctrl::getDefaultElemPath(){
#if defined(_WIN32)
    std::string sep = "\\";
#else
    std::string sep = "/";
#endif

  return getResourcesDir() + sep + s_elem_file;
}

std::string Ctrl::getVersion(){
  return s_version;
}

////////////////
// TOGGLE CLI //
////////////////

void Ctrl::hush(const bool quiet){
  _quiet = quiet;
}

void Ctrl::version(){
  notifyUser("Version: " + getVersion() + "\n");
}

////////////////
// TOGGLE GUI //
////////////////

void Ctrl::disableGUI(){
  _to_gui = false;
}

void Ctrl::enableGUI(){
#ifdef MOLOVOL_GUI
  _to_gui = true;
#else
  _to_gui = false;
#endif
}

bool Ctrl::isGUIEnabled(){
  return _to_gui;
}

/////////////
// METHODS //
/////////////

Ctrl* Ctrl::getInstance(){
  if(s_instance == NULL){
    s_instance = new Ctrl();
  }
  return s_instance;
}

#ifdef MOLOVOL_GUI
void Ctrl::registerView(MainFrame* inp_gui){
  s_gui = inp_gui;
}

bool Ctrl::loadElementsFile(){
  // GUI-specific implementation...
  if(_current_calculation == NULL){
    _current_calculation = new Model();
  }

  std::string elements_filepath = s_gui->getElementsFilepath();
  if(!_current_calculation->importElemFile(elements_filepath)){
    displayErrorMessage(101);
  }
  s_gui->displayAtomList(_current_calculation->generateAtomList());
  return true;
}

bool Ctrl::loadAtomFile(){
  // GUI-specific implementation...
  if(_current_calculation == NULL){
    _current_calculation = new Model();
  }

  bool successful_import = _current_calculation->readAtomsFromFile(
    s_gui->getAtomFilepath(), 
    s_gui->getIncludeHetatm()
  );
  s_gui->displayAtomList(_current_calculation->generateAtomList());
  return successful_import;
}

bool Ctrl::runCalculation(){
  // reset abort flag
  setAbortFlag(false);
  s_gui->extSetProgressBar(0);
  // create an instance of the model class
  // ensures, that there is only ever one instance of the model class
  if(_current_calculation == NULL){
    _current_calculation = new Model();
  }

  // PARAMETERS
  // save parameters in model
  try{
    if(!_current_calculation->setParameters(
        s_gui->getAtomFilepath(),
        s_gui->getOutputDir(),
        s_gui->getIncludeHetatm(),
        s_gui->getAnalyzeUnitCell(),
        s_gui->getCalcSurfaceAreas(),
        s_gui->getProbeMode(),
        s_gui->getProbe1Radius(),
        s_gui->getProbe2Radius(),
        s_gui->getGridsize(),
        s_gui->getDepth(),
        s_gui->getMakeReport(),
        s_gui->getMakeSurfaceMap(),
        s_gui->getMakeCavityMaps(),
        s_gui->generateRadiusMap(),
        s_gui->getIncludedElements())){
      return false;
    }
  }
  catch (const std::invalid_argument& e){
    displayErrorMessage(109);
    return false;
  }

  // CALCULATION
  CalcReportBundle data = _current_calculation->generateData();
  calculationDone(data.success);

  updateStatus((data.success && !Ctrl::getInstance()->getAbortFlag())? "Calculation done." : "Calculation aborted.");

  // OUTPUT
  displayResults(data);

  if (data.success){
    renderSurface(_current_calculation->getSurfaceData(), _current_calculation->getCellOrigin(),
        data.grid_step, data.probe_mode, 
        data.cavities.size(), _current_calculation->getAtomTree().getAtomList());
    // export if appropriate option is toggled
    if(data.make_report){exportReport();}
    if(data.make_full_map){exportSurfaceMap(false);}
    if(data.make_cav_maps){exportSurfaceMap(true);}
  }

  return data.success;
}
#endif

// CLI version
bool Ctrl::runCalculation(
    const double probe_radius_s,
    const double probe_radius_l,
    const double grid_resolution,
    const std::string& structure_file_path,
    const std::string& elements_file_path,
    const std::string& output_dir_path,
    const int tree_depth,
    const bool opt_include_hetatm,
    const bool opt_unit_cell,
    const bool opt_surface_area,
    const bool opt_probe_mode,
    const bool exp_report,
    const bool exp_total_map,
    const bool exp_cavity_maps,
    const unsigned display_flag){
  if(_current_calculation == NULL){_current_calculation = new Model();}

  try{_current_calculation->readAtomsFromFile(structure_file_path, opt_include_hetatm);}
  catch (const ExceptInvalidInputFile& e){
    displayErrorMessage(102);
    return false;
  }
  catch (const ExceptIllegalFileExtension& e){
    displayErrorMessage(103);
    return false;
  }
  catch (const ExceptInvalidCellParams& e){
    displayErrorMessage(109);
    return false;
  }

  std::vector<std::string> included_elements = _current_calculation->listElementsInStructure();

  if(!_current_calculation->importElemFile(elements_file_path)){
    displayErrorMessage(903);
    return false;
  }

  _current_calculation->setParameters(
    structure_file_path,
    output_dir_path,
    opt_include_hetatm,
    opt_unit_cell,
    opt_surface_area,
    opt_probe_mode,
    probe_radius_s,
    probe_radius_l,
    grid_resolution,
    tree_depth,
    exp_report,
    exp_total_map,
    exp_cavity_maps,
    _current_calculation->getRadiusMap(),
    _current_calculation->listElementsInStructure());

  CalcReportBundle data = _current_calculation->generateData();

  updateStatus((data.success && !Ctrl::getInstance()->getAbortFlag())? "Calculation done." : "Calculation aborted.");

  displayInput(data, display_flag);
  displayResults(data, display_flag);

  if (data.success){
    // export if appropriate option is toggled
    if(data.make_report){exportReport();}
    if(data.make_full_map){exportSurfaceMap(false);}
    if(data.make_cav_maps){exportSurfaceMap(true);}
  }
  return data.success;
}

////////////////////////
// USER COMMUNICATION //
////////////////////////

void Ctrl::clearOutput(){
#ifdef MOLOVOL_GUI
  if (_to_gui) {
    s_gui->extClearOutputText();
    s_gui->extClearOutputGrid();
  }
#endif
}

void Ctrl::displayInput(CalcReportBundle& data, const unsigned display_flag){
  auto yesno = [](bool b){
    return std::string(b? "yes" : "no");
  };
  if (display_flag & (mvOUT_INP | mvOUT_OPT)){
    if(!_to_gui){
      if (display_flag & mvOUT_INP){
        notifyUser("<INPUT>\n");
        if (display_flag & mvOUT_STRUCTURE){
          notifyUser("Structure file: " + data.atom_file_path + "\n");
        }
        if (display_flag & mvOUT_RESOLUTION){
          notifyUser("Grid resolution: " + std::to_string(data.grid_step) + " ");
          notifyUser(Symbol::angstrom());
          notifyUser("\n");
        }
        if (display_flag & mvOUT_DEPTH){
          notifyUser("Octree depth: " + std::to_string(data.max_depth) + "\n");
        }
        if (data.probe_mode){
          if (display_flag & mvOUT_RADIUS_S){
            notifyUser("Small probe radius: " + std::to_string(data.r_probe1) + " ");
            notifyUser(Symbol::angstrom());
            notifyUser("\n");
          }
          if (display_flag & mvOUT_RADIUS_L){
            notifyUser("Large probe radius: " + std::to_string(data.r_probe2) + " ");
            notifyUser(Symbol::angstrom());
            notifyUser("\n");
          }
        }
        else{
          if (display_flag & mvOUT_RADIUS_S){
            notifyUser("Probe radius: " + std::to_string(data.r_probe1) + " ");
            notifyUser(Symbol::angstrom());
            notifyUser("\n");
          }
        }
      }
      if (display_flag & mvOUT_OPT){
        notifyUser("<OPTIONS>\n");
        if (fileExtension(data.atom_file_path)=="pdb"){
          if (display_flag & mvOUT_OPT_HETATM){
            notifyUser("Include HETATM: " + yesno(data.inc_hetatm) + "\n");
          }
          if (display_flag & mvOUT_OPT_UNITCELL){
            notifyUser("Analyze unit cell: " + yesno(data.analyze_unit_cell) + "\n");
          }
        }
        if (display_flag & mvOUT_OPT_PROBEMODE){
          notifyUser("Enable two-probe mode: " + yesno(data.probe_mode) + "\n");
        }
        if (display_flag & mvOUT_OPT_SURFACE){
          notifyUser("Calculate surface areas: " + yesno(data.calc_surface_areas) + "\n");
        }
      }
    }
  }
}

void Ctrl::displayResults(CalcReportBundle& data, const unsigned display_flag){
  clearOutput();
  if (display_flag & (mvOUT_FORMULA | mvOUT_TIME)){
    if(!_to_gui){notifyUser("<OUTPUT>\n");}
  }
  if(data.success){
    std::wstring vol_unit = Symbol::angstrom() + Symbol::cubed();

    if (display_flag & mvOUT_FORMULA){
      notifyUser("Result summary for: ");
      notifyUser(Symbol::generateChemicalFormulaUnicode(data.chemical_formula));
      notifyUser("\n");
    }
    if (display_flag & mvOUT_TIME){
      notifyUser("Elapsed time: " + std::to_string(data.getTime()) + " s\n");
    }

    if (display_flag & mvOUT_VOL){
      notifyUser("<VOLUME>\n");
    }
    if (display_flag & mvOUT_VOL_VDW){
      notifyUser("Van der Waals volume: " + std::to_string(data.volumes[0b00000011]) + " ");
      notifyUser(vol_unit);
      notifyUser("\n");
    }
    if (display_flag & mvOUT_VOL_INACCESSIBLE){
      notifyUser("Probe excluded void volume: " + std::to_string(data.volumes[0b00000101]) + " ");
      notifyUser(vol_unit);
      notifyUser("\n");
    }
    if (display_flag & mvOUT_VOL_MOL){
      notifyUser("Molecular volume (Vvdw + Vvoid): " + std::to_string(data.volumes[0b00000011] + data.volumes[0b00000101]) + " ");
      notifyUser(vol_unit);
      notifyUser("\n");
    }
    std::string prefix = data.probe_mode? "Small p" : "P";
    if (display_flag & mvOUT_VOL_CORE_S){
      if(!data.probe_mode && !data.analyze_unit_cell){
        notifyUser(prefix +"robe core volume: No physical meaning, contains all volume outside the structure.\n");
      }
      else{
        notifyUser(prefix +"robe core volume: " + std::to_string(data.volumes[0b00001001]) + " ");
        notifyUser(vol_unit);
        notifyUser("\n");
      }
    }
    if (display_flag & mvOUT_VOL_SHELL_S){
      notifyUser(prefix +"robe shell volume: " + std::to_string(data.volumes[0b00010001]) + " ");
      notifyUser(vol_unit);
      notifyUser("\n");
    }
    if(data.probe_mode){
      if (display_flag & mvOUT_VOL_SHELL_L){
        if(!data.analyze_unit_cell){
          notifyUser("Large probe core volume: No physical meaning, contains all volume outside the structure.\n");
        }
        else{
          notifyUser("Large probe core volume: " + std::to_string(data.volumes[0b00100001]) + " ");
          notifyUser(vol_unit);
          notifyUser("\n");
        }
      }
      if (display_flag & mvOUT_VOL_SHELL_L){
        notifyUser("Large probe shell volume: " + std::to_string(data.volumes[0b01000001]) + " ");
        notifyUser(vol_unit);
        notifyUser("\n");
      }
    }
    if(data.calc_surface_areas && !Ctrl::getInstance()->getAbortFlag()){
      std::wstring surf_unit = Symbol::angstrom() + Symbol::squared();
      if (display_flag & mvOUT_SURF){
        notifyUser("<SURFACE>\n");
      }
      if (display_flag & mvOUT_SURF_VDW){
        notifyUser("Van der Waals surface: " + std::to_string(data.getSurfVdw()) + " ");
        notifyUser(surf_unit);
        notifyUser("\n");
      }
      if (display_flag & mvOUT_SURF_MOL){
        if(data.probe_mode){
          notifyUser("Molecular surface (both probes excluded surface): " + std::to_string(data.getSurfMolecular()) + " ");
          notifyUser(surf_unit);
          notifyUser("\n");
        }
        else if(!data.analyze_unit_cell) {
          notifyUser("Molecular open surface (reachable from outside): see outside space surface data in cavities list.\n");
        }
      }
      if (display_flag & mvOUT_SURF_EXCLUDED_S){
        notifyUser(prefix+"robe excluded surface: " + std::to_string(data.getSurfProbeExcluded()) + " ");
        notifyUser(surf_unit);
        notifyUser("\n");
      }
      if (display_flag & mvOUT_SURF_ACCESSIBLE_S){
        notifyUser(prefix+"robe accessible surface: " + std::to_string(data.getSurfProbeAccessible()) + " ");
        notifyUser(surf_unit);
        notifyUser("\n");
      }
    }
    if(_to_gui){
      notifyUser("<INFORMATION>\n");
      notifyUser("For complete results, please export report below.\n");
    }
    if (display_flag & mvOUT_CAVITIES){
      if(!data.cavities.empty()){displayCavityList(data);}
    }
  }
  else{
    if(!getAbortFlag()){displayErrorMessage(200);}
  }
}

void Ctrl::displayCavityList(CalcReportBundle& data, [[maybe_unused]] const unsigned display_flag){
  // store headers and units
  const std::wstring vol_unit = Symbol::angstrom() + Symbol::cubed();
  const std::wstring surf_unit = Symbol::angstrom() + Symbol::squared();

  GridData table({
    GridCol("Cavity\nID", L"", false, mvFORMAT_NUMBER),
    GridCol("Occupied", L"Volume (" + vol_unit + L")", false, mvFORMAT_FLOAT),
    GridCol("Accessible", L"Surface (" + surf_unit + L")", !data.calc_surface_areas, mvFORMAT_FLOAT),
    GridCol("Excluded", L"Surface (" + surf_unit + L")", !data.calc_surface_areas, mvFORMAT_FLOAT),
    GridCol("Cavity\nType", L"", data.analyze_unit_cell, mvFORMAT_STRING),
    GridCol("Center Coord", L"x, y, z (" + Symbol::angstrom() + L")")
  });

  // store data
  for (size_t i = 0; i < data.cavities.size(); ++i){
    std::string volume = (!data.probe_mode && !data.analyze_unit_cell && data.cavities[i].id == 1)? "Outside"
      : std::to_string(data.cavities[i].getVolume());

    const std::vector<std::string> cav_values = {
      std::to_string(i+1),
      volume,
      std::to_string(data.cavities[i].getSurfCore()),
      std::to_string(data.cavities[i].getSurfShell()),
      data.cavities[i].cavTypeDescriptor(data.probe_mode),
      data.cavities[i].getPosition()
    };

    table.storeValues(cav_values);
  }

  // display data

  if (_to_gui){
	#ifdef MOLOVOL_GUI
    s_gui->extDisplayCavityList(table);
	#endif
  }
  else{
    table.print();
  }
}

void Ctrl::notifyUser(std::string str){
#ifdef MOLOVOL_GUI
  if (_to_gui){
    s_gui->extAppendOutput(str);
    return;
  }
#endif
  std::cout << str;
}

void Ctrl::notifyUser(std::wstring wstr){
#ifdef MOLOVOL_GUI
  if (_to_gui){
    s_gui->extAppendOutputW(wstr);
    return;
  }
#endif
  std::wcout << wstr;
}

void Ctrl::updateStatus(const std::string str){
#ifdef MOLOVOL_GUI
  if (_to_gui) {
    s_gui->extSetStatus(str);
    return;
  }
#endif
  if (!_quiet) {
    std::cout << str << std::endl;
  }
}

void Ctrl::updateProgressBar(const int percentage){
  assert (percentage <= 100);
#ifdef MOLOVOL_GUI
  if (_to_gui) {
    s_gui->extSetProgressBar(percentage);
    return;
  }
#endif
  if (!_quiet) {
    std::cout << percentage << "%" << std::endl;
  }
}

void Ctrl::renderSurface(const Container3D<Voxel>& surf_data, const std::array<double,3> origin, 
    const double grid_step, const bool probe_mode, const unsigned char n_cavities, const std::vector<Atom>& atomlist) {
#ifdef MOLOVOL_GUI
  if (_to_gui) {
    s_gui->extRenderSurface(surf_data, origin, grid_step, probe_mode, n_cavities, atomlist);
  }
#else
  // In non-GUI mode, this function does nothing
  (void)surf_data;
  (void)origin;
  (void)grid_step;
  (void)probe_mode;
  (void)n_cavities;
  (void)atomlist;
#endif
}

const Container3D<Voxel>& Ctrl::getSurfaceData() const {
  return _current_calculation->getSurfaceData();
};

////////////
// EXPORT //
////////////

void Ctrl::exportReport(std::string path){
  _current_calculation->createReport(path);
  if(_current_calculation->optionAnalyzeUnitCell()){
    _current_calculation->writeCrystStruct(path);
  }
}

void Ctrl::exportReport(){
  _current_calculation->createReport();
  if(_current_calculation->optionAnalyzeUnitCell()){
    _current_calculation->writeCrystStruct();
  }
}

void Ctrl::exportSurfaceMap(const std::string path, bool cavities){
  if(cavities){
    _current_calculation->writeCavitiesMaps(path);
  }
  else{
    _current_calculation->writeTotalSurfaceMap(path);
  }
}

void Ctrl::exportSurfaceMap(bool cavities){
  if(cavities){
    _current_calculation->writeCavitiesMaps();
  }
  else{
    _current_calculation->writeTotalSurfaceMap();
  }
}

////////////////////////
// CALCULATION STATUS //
////////////////////////

void Ctrl::newCalculation(){
  _calculation_finished = false;
}

void Ctrl::calculationDone(const bool state){
  _calculation_finished = state;
}

bool Ctrl::isCalculationDone(){
  return _calculation_finished;
}

void Ctrl::setAbortFlag(const bool state){
  _abort_calculation = state;
}

bool Ctrl::getAbortFlag(){
  return _abort_calculation;
}

// checks whether worker thread has received a signal to stop the calculation and
// updates the progress of the calculation
void Ctrl::updateCalculationStatus(){
#ifdef MOLOVOL_GUI
  if (_to_gui){
    setAbortFlag(s_gui->receivedAbortCommand());
  }
  #endif
}

////////////////////
// ERROR MESSAGES //
////////////////////

static const std::map<int, std::string> s_error_codes = {
  {0, "Unidentified error code."}, // no user should ever see this
  // 1xx: Invalid Input
  {100, "Import failed!"},
  {101, "Invalid elements file. Please select a valid file or set radii manually. Atomic weights will be set to 0."},
  {102, "Invalid structure file. Please select a valid file. You may need to enable the option HETATM for PDB files."},
  {103, "Invalid file format. Please make sure that the input files have the correct file extensions."},
  {104, "Invalid probe radius input. The large probe must have a larger radius than the small probe."},
  {105, "Invalid entry in structure file encountered. Some atoms have not been imported. Please check the format of the input file."},
  {106, "Invalid element symbol(s) in elements file detected. Some radii may be assigned incorrectly. Please make sure that all element symbols begin with an alphabetic character."},
  {107, "Invalid radius value in elements file detected. Some radii may be set to 0. Please make sure that all radii are numeric."},
  {108, "Invalid atomic weight value in elements file detected. Some atomic weights may be set to 0. Please make sure that all atomic weights are numeric."},
  {109, "Invalid numeric input. Please make sure that all input values have a valid number format."},
  // 11x: unit cell files
  {111, "Space group not found. Check the structure file, or untick the Unit Cell Analysis tickbox."},
  {112, "Invalid unit cell parameters. Check the structure file, or untick the Unit Cell Analysis tickbox."},
  {113, "Space group or symmetry not found. Check the structure and space group files or untick the Unit Cell Analysis tickbox"},
  {114, "Invalid ATOM or HETATM line encountered. Import may be incomplete. Check the structure file."},
  {115, "Invalid option(s). You may have selected an option that is incompatible with the structure file format."},
  // 2xx: Issue during Calculation
  {200, "Calculation failed!"},
  {201, "Total number of cavities (255) exceeded. Consider changing the probe size. Calculation will proceed."},
  // 3xx: Issue with Output
  {300, "Output failed!"},
  {301, "Data missing to export file. Calculation may be still running or has not been started."},
  {302, "Invalid output directory. Please select a valid output directory."},
  {303, "An unidentified issue has been encountered while writing the surface map."},
  // 9xx: Issues with command line arguments
  {900, "Command line interface failed!"},
  {902, "Invalid output display option. At least one parameter belonging to '-o' is invalid and will be ignored."},
  {903, "Elements file import failed. Calculation aborted."},
  // 9xx: Required command line arguments missing
  {910, "Unexpected error. More than three required command line arguments appear to be missing."},
  {911, "One required command line argument missing. Please provide --%s"},
  {912, "Two required command line arguments missing. Please provide --%s and --%s"},
  {913, "Three required command line arguments missing. Please provide --%s, --%s, and --%s"}
};

// Print error message either to GUI or console
void Ctrl::displayErrorMessage(const int error_code, const std::vector<std::string>& str_fill){
  std::string msg = getErrorMessage(error_code);

  // Substitute place holders
  size_t i = 0;
  while (msg.find("%s") != std::string::npos){
    assert(str_fill.size() > i);
    msg.replace(msg.find("%s"), 2, str_fill[i]);
    ++i;
  }

#ifdef MOLOVOL_GUI
  if (_to_gui){
    s_gui->extOpenErrorDialog(error_code, msg);
    return;
  }
#endif
  // Print to console
  std::cout << error_code << ": " << msg << std::endl;
}

std::string Ctrl::getErrorMessage(const int error_code){
  if (s_error_codes.find(error_code) == s_error_codes.end()) {
    return s_error_codes.find(0)->second;
  }
  else {
    return s_error_codes.find(error_code)->second;
  }
}

