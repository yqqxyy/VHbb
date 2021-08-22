#include "CxAODTools/ConfigField.h"

#include <algorithm>
#include <string>
#include <vector>

#define FMTWIDTH 30

// specializations of print
//   <bool>
template <>
void ConfigField<bool>::print(const std::string &fieldName) const {
  std::string msg = "bool " + fieldName;
  int newSize = std::max(FMTWIDTH, int(msg.size()));
  msg.resize(newSize, ' ');
  msg += " = ";
  msg += (bool)m_value ? "true" : "false";
  std::cout <<  msg.c_str() << std::endl;
}

//   <int>
template <>
void ConfigField<int>::print(const std::string &fieldName) const {
  std::string lhs = "int " + fieldName;
  int newSize = std::max(FMTWIDTH, int(lhs.size()));
  lhs.resize(newSize, ' ');
  lhs += " = ";
  std::cout << lhs.c_str() << m_value << std::endl;
}

//   <float>
template <>
void ConfigField<float>::print(const std::string &fieldName) const {
  std::string lhs = "float " + fieldName;
  int newSize = std::max(FMTWIDTH, int(lhs.size()));
  lhs.resize(newSize, ' ');
  lhs += " = ";
  std::cout << lhs.c_str() << m_value << std::endl;
}

//   <double>
template <>
void ConfigField<double>::print(const std::string &fieldName) const {
  std::string lhs = "double " + fieldName;
  int newSize = std::max(FMTWIDTH, int(lhs.size()));
  lhs.resize(newSize, ' ');
  lhs += " = ";
  std::cout << lhs.c_str() << m_value << std::endl;
}

//   <std::string>
template <>
void ConfigField<std::string>::print(const std::string &fieldName) const {
  std::string lhs = "string " + fieldName;
  int newSize = std::max(FMTWIDTH, int(lhs.size()));
  lhs.resize(newSize, ' ');
  lhs += " = ";
  std::cout << lhs.c_str() << m_value << std::endl;
}

//   <std::vector<bool>>
template <>
void ConfigField<std::vector<bool>>::print(const std::string &fieldName) const {
  std::string lhs = "vector<bool> " + fieldName;
  int newSize = std::max(FMTWIDTH, int(lhs.size()));
  lhs.resize(newSize, ' ');
  lhs += " =";
  std::cout << lhs.c_str();
  for (auto val : m_value) {
    std::cout << " " << (val ? "true" : "false");
  }
  std::cout << std::endl;
}

//   <std::vector<int>>
template <>
void ConfigField<std::vector<int>>::print(const std::string &fieldName) const {
  std::string lhs = "vector<int> " + fieldName;
  int newSize = std::max(FMTWIDTH, int(lhs.size()));
  lhs.resize(newSize, ' ');
  lhs += " =";
  std::cout << lhs.c_str();
  for (auto val : m_value) {
    std::cout << " " << val;
  }
  std::cout << std::endl;
}

//   <std::vector<float>>
template <>
void ConfigField<std::vector<float>>::print(const std::string &fieldName) const {
  std::string lhs = "vector<float> " + fieldName;
  int newSize = std::max(FMTWIDTH, int(lhs.size()));
  lhs.resize(newSize, ' ');
  lhs += " =";
  std::cout << lhs.c_str();
  for (auto val : m_value) {
    std::cout << " " << val;
  }
  std::cout << std::endl;
}

//   <std::vector<double>>
template <>
void ConfigField<std::vector<double>>::print(const std::string &fieldName) const {
  std::string lhs = "vector<double> " + fieldName;
  int newSize = std::max(FMTWIDTH, int(lhs.size()));
  lhs.resize(newSize, ' ');
  lhs += " =";
  std::cout << lhs.c_str();
  for (auto val : m_value) {
    std::cout << " " << val;
  }
  std::cout << std::endl;
}

//   <std::vector<std::string>>
template <>
void ConfigField<std::vector<std::string>>::print(const std::string &fieldName) const {
  std::string lhs = "vector<string> " + fieldName;
  int newSize = std::max(FMTWIDTH, int(lhs.size()));
  lhs.resize(newSize, ' ');
  lhs += " =";
  std::cout << lhs.c_str();
  for (auto &val : m_value) {
    std::cout << " " << val.c_str();
  }
  std::cout << std::endl;
}
