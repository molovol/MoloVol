#ifndef GRIDDATA_H

#define GRIDDATA_H

#include <string>
#include <vector>

struct someText{
  someText(std::string str, std::wstring wstr) : str(str), wstr(wstr){}
  someText(std::string str) : someText(str, L"") {}
  someText(std::wstring wstr) : someText("", wstr) {}
  std::string str;
  std::wstring wstr;
};

// GridCol
struct GridCol{
  GridCol(const std::string header, const std::wstring unit) : header(header), unit(unit), hide_col(false){};
  GridCol(const std::string header, const std::wstring unit, const bool hide_col) 
    : header(header), unit(unit), hide_col(hide_col){};
  std::string header;
  std::wstring unit;
  std::vector<std::string> values;
  bool hide_col;

  int getNumberRows(const bool include_header) const;
  someText getElem(const int row, const bool include_header) const;
  void pushBack(const std::string val);
};

// GridData
struct GridData{
  GridData(std::vector<GridCol> columns) : columns(columns){};

  size_t getNumberRows(bool include_header) const;
  bool hideCol(const int) const;
  void print() const;
  std::string getValue(const size_t, const size_t) const;
  void storeValues(const std::vector<std::string> vals);
  
  private:
    std::vector<GridCol> columns;
};

#endif
