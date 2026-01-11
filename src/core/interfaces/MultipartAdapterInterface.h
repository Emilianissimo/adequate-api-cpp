#pragma once
#include <string>
#include <unordered_map>
#include <vector>

struct Part {
  std::string name;
  std::string filename;
  std::string contentType;
  std::string data;
  std::unordered_map<std::string, std::string> headers;
};

struct MultipartAdapterInterface {
  virtual ~MultipartAdapterInterface() = default;
  virtual std::vector<Part> parse(const std::string& contentType, const std::string& body) const =0;
};
