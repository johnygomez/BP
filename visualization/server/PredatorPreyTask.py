from redis import Redis
from Task import Task

## Implementation of Task interface for a so called Predator-Prey task
#
class PredatorPreyTask(Task):
  ## Constructor method
  def __init__(self):
    ## Handles connection to the Redis database
    #
    self.database = Redis('localhost')
    ## Stores current state
    #
    self.state = [1.0, 1.0]
  
  ## A reward function
  #
  # If agent is closer to target than 0.5m cca 0.15 normalized and rotated less than 30 degrees,
  # its considered to be a target state so the reward is 0. Else reward is equal 0.01
  # @param state Current environment state
  # @return Returns a reward for a give state
  def get_reward(self, state):
    if state[0] < 0.15 and abs(state[1] - 0.5) < 0.1:
      return 0
    else:
      return 0.01

  ## Update current state loading the data from database
  #
  # @return Returns current environment state
  def refresh_state(self):
    try:
      self.state[0] = float(self.database.get('distance'))
      self.state[1] = float(self.database.get('angle'))
    except Exception, e:
      print('Could not parse state, reseting to last value')
    return self.state
