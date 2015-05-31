from abc import ABCMeta, abstractmethod
## Provides a default interface for Agent class
#
class Agent:
  __metaclass__ = ABCMeta
  
  ## This method should handle connection to a real world agent
  #
  # This method must be implemented
  @abstractmethod
  def connect(self): pass

  ## This methods should handle disconnection from a real world agent
  #
  # This method must be implemented
  @abstractmethod
  def disconnect(self): pass

  ## This methods handles performing selected action according to action number on a real world agent
  #
  # This method must be implemented
  # @param action_number Number of action to be performed
  @abstractmethod
  def do(self, action_number): pass
