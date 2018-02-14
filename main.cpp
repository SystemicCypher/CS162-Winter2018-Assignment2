// This file contains parsing code that reads the input and makes
// the corresponding function calls to your Heap class. You don't
// need to worry about anything that happens here.

#include <algorithm>
#include <cstdint>
#include <iostream>
#include <vector>

#include "heap.hpp"

// Removes spaces from a string (in place).
// Based on https://stackoverflow.com/a/83481
void strip_whitespace(std::string& input) {
  auto end = std::remove(input.begin(), input.end(), ' ');
  input.erase(end, input.end());
}

// Splits an assignment path (like a.b.c) into a vector of path segments.
// Based on https://stackoverflow.com/a/14267455
std::vector<std::string> split(const std::string& input) {
  size_t start = 0;
  std::vector<std::string> tokens;

  while(true) {
    size_t end = input.find('.', start);
    if(end == std::string::npos) {
      tokens.push_back(input.substr(start));
      return tokens;
    }
    else {
      tokens.push_back(input.substr(start, end - start));
      start = end + 1;
    }
  }
}

int main(int argc, char *argv[]) {
  int size;

  if(argc == 1) {
    size = 200;
  }
  else if(argc == 2) {
    size = std::atoi(argv[1]);
    if(size < 1 || size % 2 != 0) {
      std::cerr << "Heap size must be positive and even.\n";
      std::cerr << "USAGE: ./a.out size=[200]\n";
      exit(1);
    }
  }
  else {
    std::cerr << "USAGE: ./a.out size=[200]\n";
    exit(1);
  }

  Heap heap(size);

  while(true) {
    std::string line;
    std::getline(std::cin, line);
    if(std::cin.eof()) return 0;

    strip_whitespace(line);
    if(line.length() == 0 || line[0] == '#') {
      // Blank or a comment; skip it.
      continue;
    }

    if(line == "PRINT") {
      heap.print();
      continue;
    }
    if(line == "DEBUG") {
      heap.debug("CALLED");
      continue;
    }
    if(line == "COLLECT") {
      heap.collect();
      continue;
    }

    int pos = line.find('=');
    if(pos == std::string::npos) {
      std::cerr << "Bad command: " << line << '\n';
      continue;
    }

    auto lhs = line.substr(0, pos);
    auto rhs = line.substr(pos + 1);
    auto obj = obj_ptr(-1);

    try {
      if(rhs == "Foo") {
        obj = heap.new_foo();
        // std::cout << "Allocated Foo " << lhs << " at " << obj << std::endl;
      }
      else if(rhs == "Bar") {
        obj = heap.new_bar();
        // std::cout << "Allocated Bar " << lhs << " at " << obj << std::endl;
      }
      else if(rhs == "Baz") {
        obj = heap.new_baz();
        // std::cout << "Allocated Baz " << lhs << " at " << obj << std::endl;
      }
      else if(rhs == "NULL") {
        obj = obj_ptr(-1);
      }
      else {
        auto rpath = split(rhs);
        obj = heap.get(rpath);
      }

      auto lpath = split(lhs);
      heap.set(lpath, obj);
    }
    catch(OutOfMemoryException e) {
      std::cout << "OUT OF MEMORY\n";
    }
  }
}
