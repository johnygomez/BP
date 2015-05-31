# @file
import json
import math
from random import random
import redis
import _rprop as rp
import numpy as np

from NaoAgent import NaoAgent as Agent
from PredatorPreyTask import PredatorPreyTask as Task

## This class handles mechanism behind NFQ learning and also decision process
#
# It contains definitions for methods that handles communication with database, 
# operating the neural network, performing tasks and finally the NFQ learning and decision procedure.
class NFQ:
  """
  @author Jan Gamec
  @version 1.0
  """
  ## Encoding of action number for purposes of neural network input
  #
  action_codes = [
    [1, 0, 0],
    [0, 1, 0],
    [0, 0, 1]
  ]

  ## Constructor of the class
  #
  # Handles basic configuration of the NFQ module and its constants.
  # It's arranging connection with Redis database and initializes Agent and Task objects, that are necessary for algorithm.
  # It handles initialization of neural network keeping learned knowledge if no record of network configuration is in database.
  # It also loads information about previous learning process from the database and parses previous samples intro training patterns.
  # @param discount Discount factor constant used during learning
  def __init__(self, discount = 0.99):
    ## An Agent performs actions in the environment
    #
    self.agent = Agent()
    ## A Task object provides reward function and also environment state update
    #
    self.task = Task()
    ## State variable signalizing if Agent is connected to the real robot
    #
    self.connected = self.agent.connect(ip = '192.168.1.18')
    ## Connection to the Redis database
    #
    self.redis_server = redis.Redis('localhost')
    ## Episode counter
    #
    self.epoch = 1 if not self.redis_server.get('epoch') else int(self.redis_server.get('epoch'))
    ## Discount factor constant for learning process
    #
    self.discount = discount
    self.load_net()
    self.load_samples()
    ## Holds weights of the neural network
    #
    self.network = rp.init() if self.network is None else self.network
    ## Set of training samples in a form of tuples according to work
    #
    self.samples = list() if self.samples is None else self.samples
    ## Set of training pattern, which can be applied directly to the neural network
    #
    self.training_set = list()
    self.get_training_set_from_samples()
    ## Representation of environment state
    #
    self.state = [1.0, 1.0]
    ## Counts steps made during current episode
    #
    self.step_counter = 1

  ## Saves current network to database
  # 
  # @param name Specifies name for the network
  def save_net(self, name='net'):
    self.redis_server.set(name, json.dumps(self.network.tolist()))
    if name != 'net':
      self.redis_server.lrem('tasks', name)
      self.redis_server.lpush('tasks', name)

  ## Loads and update current network weights with the network saved in database
  #
  # @param name Specifies the name of network in the database
  def load_net(self, name='net'):
    _net = self.redis_server.get(name)
    if _net is not None:
      _net_parsed = json.loads(_net)
      assert type(_net_parsed) is list
      self.network = np.array(_net_parsed)
    else:
      self.network = None

  ## Loads a list of all previously learned tasks from a database
  #
  # @return List of all learned tasks
  def load_tasks(self):
    tasks = self.redis_server.lrange('tasks', 0, -1)
    return tasks

  ## Loads a previously collected samples from the database and update current ones with them
  #
  def load_samples(self):
    _samples = self.redis_server.get('samples')
    if not _samples:
      self.samples = None
    else:
      self.samples = json.loads(_samples)
      assert type(self.samples) is list
  
  ## Upload current training samples set into database in JSON format
  #
  def write_samples(self):
    self.redis_server.set('samples', json.dumps(self.samples))

  ## This method provides basic transformation from the training sample into training patterns
  #
  # Training patterns are generated according to the algorithm in the work, where for action selected in sample 
  # pattern with minimal Q-value is generated and for all other actions patterns with maximal Q are generated
  # @param sample Training sample in the form of tuple
  # @return List of training patterns
  def sample_to_patterns(self, sample):
    _action_codes = self.action_codes[:]
    pattern = list()
    pattern_list = list()
    pattern.extend(sample[0:2]) # state in t
    pattern.extend(_action_codes[sample[2]])
    del _action_codes[sample[2]]
    pattern.append(self.get_target(sample[0:2], sample[-1]))
    # generate patterns for wrong decisions with higher Q value
    pattern_list.append(pattern)
    for action in _action_codes:
      pattern = list()
      pattern.extend(sample[0:2]) # state in t
      pattern.extend(action)
      pattern.append(self.maxQ(sample[0:2])) 
      pattern_list.append(pattern)
    return pattern_list

  ## Transform whole set of training samples into training patterns according to the proposed algorithm
  #
  # This method updates current training set used for training with the generated one
  def get_training_set_from_samples(self):
    self.training_set = list()
    for sample in self.samples:
      patterns = self.sample_to_patterns(sample)
      self.training_set.extend(patterns)
    print('Training set updated')

  ## The key method of this class, if target state is reached, this method updates a current Q-function.
  #
  # Update is realised training a neural network with a training patternset generated from samples collected during previous episodes.
  # It also updates information about the learning progress to the database (Episode no., Number of steps)
  # It delegates recursive control process to the @ref choose_controller
  # @see choose_controller method
  def learn(self):
    if self.task.get_reward(self.state) == 0:
      # reached goal state
      self.epoch += 1
      self.redis_server.set('epoch', self.epoch)
      self.redis_server.lpush('steps', self.step_counter)
      self.get_training_set_from_samples()
      self.network = rp.learn(500, np.array(self.training_set), self.network)
      self.step_counter = 1
      self.save_net()
    self.choose_controller()

  ## This method controls the decision process according to current autonomy level
  #
  # It either asks a humna operator for action or decides according to the policy
  def choose_controller(self):
    rnd = random()
    test = math.exp(-self.epoch/20.0)
    print('MTIA: ', (1-test))
    if test >= rnd:
      # control decission is passed back to human
      return
    else:
      # conrol will be handled automatically according to Q-learning
      self.do_action(self.minQ_index(self.state))
      return
  
  ## Runs a task saved in database with a specific name.
  #
  # The learned task is represented by its NN weights configuration which are loaded and replaces 
  # current network configuration during the task performance. Original network is restored after then.
  # @param name Specifies a name of task saved in database
  # @return Returns cumulative reward after task was performed
  def run_task(self, name):
    if not name in self.load_tasks():
      return -1
    _network = self.network
    self.load_net(name)
    reward = self.make_step_task(0)
    self.network = _network
    return reward
  
  ## This methods handles steps and reward counting during performing loaded tasks.
  #
  # It works in the recursive way. If target area or step limit is not reached, agent make a step and cumulate the reward.
  # The method calls itself until target state or step limit is reached.
  # @param reward Represents current cumulative reward
  # @return Reward after end of task performance
  def make_step_task(self, reward=0):
    self.do_action(self.minQ_index(self.state), True)
    rt = self.task.get_reward(self.state)
    if rt == 0 or reward > 0.5:
      return reward
    else:
      return self.make_step_task(reward+rt)

  ## Method representing an elementary step of agent in the environment.
  #
  # Given the action number, method make an Agent do selected action and collect a sample information.
  # Samples are collected in a tuples according to work. 
  # > [state at t, action number, state at t+1, epoch, step]
  # If total autonomy (in a case of performing leaded task) is off, sample is appended to current sample set.
  # This method then delegates decision process back to the @ref learn() method in a recursive way
  # @param action_num Number of action in the action set
  # @param autonomy Default value is False. During autonomy samples are not stored
  def do_action(self, action_num, autonomy=False):
    sample = list()
    self.state = self.task.refresh_state()
    sample.append(self.state[0])
    sample.append(self.state[1])
    sample.append(action_num)
    print('Taking action', action_num)
    self.agent.do(action_num)
    self.state = self.task.refresh_state()
    sample.append(self.state[0])
    sample.append(self.state[1])
    sample.append(self.epoch)
    sample.append(self.step_counter)
    if autonomy == False:
      self.samples.append(sample)
      self.write_samples()
      self.step_counter += 1
      self.learn()

  ## General method for acquiring the Q-value for current state-action pair.
  #
  # This method runs a neural network forward in order to get Q-value for current state
  # @param state Represents state of environment
  # @param operator Function handle, e.g. we want minimal Q-value in current state for available actions, we use function min()
  # @return A pair, where first value corresponds to the actual Q-value and second is the index of the action having this value
  def getQ(self, state, operator):
    result = list()
    for action in self.action_codes:
      pattern = list()
      pattern.extend(state)
      pattern.extend(action)
      q = rp.run(np.array(pattern), self.network)
      result.append(q)
    return [operator(result), result.index(operator(result))]

  ## Gets a minimum Q-value in current state for available actions
  #
  # @param state Represents state of environment
  # @return Minimal Q-value for current state
  def minQ(self, state):
    return self.getQ(state, min)[0]

  ## Gets an index of action with minimal Q-value in current state
  #
  # @param state Represents state of environment
  # @return Index of the action with minimal Q-value
  def minQ_index(self, state): 
    return self.getQ(state, min)[1]

  ## Gets a maximal Q-value in current state for available actions
  #
  # @param state Represents state of environment
  # @return maximal Q-value for current state
  def maxQ(self, state):
    return self.getQ(state, max)[0]

  ## Gets an index of action with a maximal Q-value in current state
  #
  # @param state Represents state of environment
  # @return Index of the action with a maximal Q-value
  def maxQ_index(self, state):
    return self.getQ(state, max)[1]

  ## Calculates a target Q-value according to Bellman equation mentioned in the work.
  #
  # @param state Represents an environment state. Default is current state.
  # @param step Time step. Default value is step count in current episode
  # @return Returns the target Q-value for a neural network training
  def get_target(self, state=None, step=None): 
    _state = self.state if state is None else state
    _step = self.step_counter if step is None else step
    return (self.task.get_reward(_state) + math.pow(self.discount, _step) * self.minQ(_state))


