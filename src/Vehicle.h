#ifndef VEHICLE_H
#define VEHICLE_H

#include "TrafficObject.h"

// forward declarations to avoid include cycle
class Street;
class Intersection;

class Vehicle : public TrafficObject,
                public std::enable_shared_from_this<Vehicle> {
public:
  // constructor / desctructor
  Vehicle();

  // getters / setters
  void setCurrentStreet(std::shared_ptr<Street> street) {
    _currStreet = street;
  };
  void setCurrentDestination(std::shared_ptr<Intersection> destination);

  // typical behaviour methods
  void simulate() override;

  // miscellaneous
  std::shared_ptr<Vehicle> get_shared_this() { return shared_from_this(); }

private:
  // typical behaviour methods
  void drive();

  // street on which the vehicle is currently on
  std::shared_ptr<Street> _currStreet; 

  // destination to which the vehicle is currently driving
  std::shared_ptr<Intersection> _currDestination;
  
  // position on current street
  double _posStreet;    
  
  // ego speed in m/s
  double _speed;
};

#endif