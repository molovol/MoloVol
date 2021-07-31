#ifndef GRIDDATA_H

#define GRIDDATA_H

#include <string>
#include <vector>

struct SomeText{
  SomeText(std::string str, std::wstring wstr) : str(str), wstr(wstr){}
  SomeText(std::string str) : SomeText(str, L"") {}
  SomeText(std::wstring wstr) : SomeText("", wstr) {}
  std::string str;
  std::wstring wstr;
  void replaceNewlines();
};

// GridCol
struct GridCol{
  GridCol(const std::string header, const std::wstring unit, const bool hide_col, const unsigned char format)
    : header(header), unit(unit), hide_col(hide_col), format(format){};
  GridCol(const std::string header, const std::wstring unit)
    : GridCol(header, unit, false, 0) {};
  std::string header;
  std::wstring unit;
  std::vector<std::string> values;
  bool hide_col;
  unsigned char format;

  int getNumberRows(const bool include_header) const;
  SomeText getElem(const int row, const bool include_header) const;
  void pushBack(const std::string val);
};

// GridData
struct GridData{
  GridData(std::vector<GridCol> columns) : columns(columns){};

  size_t getNumberRows(bool include_header) const;
  size_t getNumberCols() const;
  unsigned char getFormat(const size_t) const;
  bool hideCol(const int) const;
  void print() const;
  std::string getValue(const size_t, const size_t) const;
  std::string getHeader(const size_t) const;
  std::wstring getUnit(const size_t) const;
  void storeValues(const std::vector<std::string> vals);
  
  private:
    std::vector<GridCol> columns;
};
  

#endif
