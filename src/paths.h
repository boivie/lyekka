#ifndef PATHS_H_INCLUSION_GUARD
#define PATHS_H_INCLUSION_GUARD
#include <list>
#include <string>
#include <exception>

namespace Lyekka {
  class PathInfo {  
  public:
    int id;
    std::string path;
  };

  class Paths {
  public:
    static std::list<PathInfo> get(void);
    static int add(std::string& path);
    static void remove(int id);
  };

  class PathException : public std::exception { 
  };
  class PathAlreadyAddedException : public PathException {
    virtual const char* what() const throw() { return "The path has already been added."; } 
  };
  class InvalidPathException : public PathException {
    virtual const char* what() const throw() { return "The path is invalid (not representing an existing directory)."; } 
  };
}
#endif
