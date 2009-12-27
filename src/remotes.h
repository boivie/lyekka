#ifndef REMOTES_H_INCLUSION_GUARD
#define REMOTES_H_INCLUSION_GUARD
#include <list>
#include <string>
#include <exception>

namespace Lyekka {
  class RemoteInfo {  
  public: 
    int id;
    std::string name;
    std::string default_destination;
  };

  class Remotes {
  public:
    static std::list<RemoteInfo> get(void);
    static RemoteInfo get(std::string& name);
    static int add(std::string name, std::string default_destination);
    static void remove(std::string name);
  };

  class RemoteException : public std::exception { 
  };
  class RemoteAlreadyAddedException : public RemoteException {
    virtual const char* what() const throw() { return "The remote site has already been added."; } 
  };
  class NoSuchRemoteException : public RemoteException {
    virtual const char* what() const throw() { return "No remote site found with this name."; } 
  };

}
#endif
