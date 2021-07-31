#include "griddata.h"
#include "controller.h"
#include "misc.h"

// GridCol definitions
int GridCol::getNumberRows(const bool include_header) const {
  return values.size() + (include_header? 2 : 0);
}

SomeText GridCol::getElem(const int row, const bool include_header) const {
  if (include_header) {
    if (row == 0){
      return SomeText(header);
    }
    else if (row == 1){
      return SomeText(unit);
    }
  }
  return SomeText(values[row - (include_header? 2 : 0)]);
}

void GridCol::pushBack(const std::string val){
  values.push_back(val);
}

// GridData definitions
size_t GridData::getNumberRows(bool include_header) const {return columns[0].getNumberRows(include_header);}
size_t GridData::getNumberCols() const {return columns.size();}
  
unsigned char GridData::getFormat(const size_t col) const {
  return columns[col].format;
}

bool GridData::hideCol(const int col) const{
  return columns[col].hide_col;
}

void GridData::print() const {
  constexpr int width = 20;
  
  for (size_t row = 0; row < getNumberRows(true); ++row){
    for (const GridCol& col : columns) {
      if (col.hide_col){continue;}
      SomeText cell = col.getElem(row,true);
      cell.replaceNewlines();
      if (cell.str.empty()) {
        Ctrl::getInstance()->notifyUser(wfield(width, cell.wstr));
      }
      else{
        Ctrl::getInstance()->notifyUser(field(width, cell.str));
      }
    }
    Ctrl::getInstance()->notifyUser("\n");
  }
}
 
std::string GridData::getHeader(const size_t col) const {
  return columns[col].header;
}
std::wstring GridData::getUnit(const size_t col) const {
  return columns[col].unit;
}

std::string GridData::getValue(const size_t row, const size_t col) const{
  return columns[col].getElem(row, false).str;
}

void GridData::storeValues(const std::vector<std::string> vals){
  for (size_t col = 0; col < vals.size(); ++col){
    columns[col].pushBack(vals[col]);
  }
}

// SomeText definitions
void SomeText::replaceNewlines(){
  std::replace( str.begin(), str.end(), '\n', ' ');
  std::replace( wstr.begin(), wstr.end(), '\n', ' ');
}
