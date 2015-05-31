__author__ = 'Jan Gamec, jan.gamec@outlook.com'

from abc import ABCMeta, abstractmethod
## Provides a default interface for Tasks used in NFQ
#
class Task:
  __metaclass__ = ABCMeta
  
  ## Reward function
  #
  # This method has to be implemented
  @abstractmethod
  def get_reward(self, state): pass
  
  ## Method for updating environment state
  #
  # This method has to be implemented
  @abstractmethod
  def refresh_state(self): pass