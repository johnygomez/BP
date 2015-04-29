# File:          SupervisorController.py
# Date:          
# Description:
# Author: Jan Gamec
# Modifications: 

from math import *
import redis

# You may need to import some classes of the controller module. Ex:
#  from controller import Robot, LED, DistanceSensor
#
# or to import the entire module. Ex:
#  from controller import *
from controller import Supervisor

# Here is the main class of your controller.
# This class defines how to initialize and how to run your controller.
# Note that this class derives Robot and so inherits all its functions
class SupervisorController (Supervisor):
  
  # User defined function for initializing and running
  # the SupervisorController class
  def run(self):
    self.redis_server = redis.Redis('localhost')
    # You should insert a getDevice-like function in order to get the
    # instance of a device of the robot. Something like:
    #  led = self.getLed('ledname')
    
    # Main loop
    while True:
      naoPosition = self.getFromDef('Nao1').getPosition()
      naoRotation = self.getFromDef('Nao1').getOrientation()
      targetPosition = self.getFromDef('target').getPosition()
      targetVector = [
        targetPosition[0] - naoPosition[0],
        targetPosition[1] - naoPosition[1],
        targetPosition[2] - naoPosition[2]
        ]
      pokVector = [
        naoRotation[0],
        naoRotation[3],
        naoRotation[6]      
      ]
      self.redis_server.set('distance', self.getDistance(naoPosition, targetPosition))
      self.redis_server.set('angle', self.getAngle(pokVector, targetVector)) 
      # Perform a simulation step of 64 milliseconds
      # and leave the loop when the simulation is over
      if self.step(64) == -1:
        break
      
      # Read the sensors:
      # Enter here functions to read sensor data, like:
      #  val = ds.getValue()
      
      # Process sensor data here.
      
      # Enter here functions to send actuator commands, like:
      #  led.set(1)
    
    # Enter here exit cleanup code

  def getDistance(self, pos1, pos2):
    assert (len(pos1) == len(pos2))
    sum = 0
    for id in range(len(pos1)):
      sum += (pos2[id] - pos1[id])**(2)
    return self.normalize_distance(sum**(.5))
    
  def getAngle(self, vc1, vc2):
    angle = 0;
    cross = 0;
    # calculate cosine of angle from dot product
    angle = self.getDotProd(vc1, vc2)
    # calculate sign of angle using cross product
    cross = self.getCrossProd(vc1, vc2)
    # z-axis is on 1st index
    angle_final = copysign(1, cross[1])*degrees(acos(angle))
    return self.normalize_angle(angle_final)
  
  
  # get dot product of 2 vectors
  def getDotProd(self, a, b):
    angle = 0
    angle += a[0]*b[0] + a[2]*b[2]
    angle /= (a[0]**2 + a[2]**2)**(.5)
    angle /= (b[0]**2 + b[2]**2)**(.5)
    return angle
    
  # get cross product of 2 vectors
  def getCrossProd(self, a, b):
    c = [a[1]*b[2] -a[2]*b[1],
         a[2]*b[0] - a[0]*b[2],
         a[0]*b[1] - a[1]*b[0]]
    return c
    
  def normalize_distance(self, dis):
    # normalize to <0,1> where max is 3m
    return dis/3.0
    
  def normalize_angle(self, angle):
    #normalize to <0,1>
    return (angle+180.0)/360.0
# The main program starts from here

# This is the main program of your controller.
# It creates an instance of your Robot subclass, launches its
# function(s) and destroys it at the end of the execution.
# Note that only one instance of Robot should be created in
# a controller program.
controller = SupervisorController()
controller.run()
