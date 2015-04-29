import json
import math
from random import random
import redis
import _rprop as rp
import numpy as np

from Agent import Agent


class NFQ:
  # representation of action taken for input neurons in network
  action_codes = [
    [1, 0, 0, 0],
    [0, 1, 0, 0],
    [0, 0, 1, 0],
    [0, 0, 0, 1]
  ]

  def __init__(self):
    self.agent = Agent()
    self.connected = self.agent.connect(ip = '127.0.0.1')
    self.redis_server = redis.Redis('localhost')
    self.epoch = 1 if not self.redis_server.get('epoch') else int(self.redis_server.get('epoch'))
    self.discount = 0.99
    self.load_net()
    self.load_patterns()
    self.network = rp.init() if self.network is None else self.network
    self.training_set = list() if self.training_set is None else self.training_set 
    self.state = [1.0, 1.0]
    self.step_counter = 1


  def refresh_state(self):
    '''Has to implement this method to read states from environment'''
    try:
      self.state[0] = float(self.redis_server.get('distance'))
      self.state[1] = float(self.redis_server.get('angle'))
    except Exception, e:
      print('Could not parse state, reseting to last value')

  def save_net(self):
    self.redis_server.set('net', json.dumps(self.network.tolist()))

  def load_net(self):
    _net = self.redis_server.get('net')
    if _net is not None:
      _net_parsed = json.loads(_net)
      assert type(_net_parsed) is list
      self.network = np.array(_net_parsed)
    else:
      self.network = None

  def reset_patterns(self):
    self.training_set = list()
    self.redis_server.delete('patternset')

  def load_patterns(self):
    _patterns = self.redis_server.get('patternset')
    if _patterns is not None:
      self.training_set = json.loads(_patterns)
      assert type(self.training_set) is list
    else:
      self.training_set = None

  # ToDo: LPush patternset into list each epoch
  def write_pattern(self, pattern):
    self.training_set.append(pattern)
    self.redis_server.set('patternset', json.dumps(self.training_set))

  def manage(self):
    self.save_net()
    if self.get_reward() == 0:
      # reached goal state
      self.epoch += 1
      self.redis_server.set('epoch', self.epoch)
      # TODO: Change to LPUSH same as patterns?
      self.redis_server.rpush('steps', self.step_counter)
      self.step_counter = 1
      self.network = rp.learn(300, np.array(self.training_set), self.network)
      self.save_net()
      # self.reset_patterns()
      self.choose_controller()
      # TODO: Reset robot and target position and start process again
    else:
      self.choose_controller()

  def choose_controller(self):
    rnd = random()
    test = math.exp(-self.epoch/20.0)
    print('Probability: ', test)
    if test >= rnd:

      # control decission is passed back to human
      return
    else:
      # conrol will be handled automatically according to Q-learning
      self.do_action(self.minQ_index(self.state))
      return

  def do_action(self, action_num):
    pattern = list()
    self.refresh_state()
    pattern.append(self.state[0])
    pattern.append(self.state[1])
    pattern.extend(self.action_codes[action_num])
    print('Taking action', action_num)
    self.agent.do(action_num)
    self.refresh_state()
    pattern.append(self.get_target())
    self.write_pattern(pattern)
    self.step_counter += 1
    self.manage()

  def getQ(self, state, operator):
    result = list()
    for action in self.action_codes:
      pattern = list()
      pattern.extend(state)
      pattern.extend(action)
      q = rp.run(np.array(pattern), self.network)
      result.append(q)

    print('Q-value', operator(result))
    return [operator(result), result.index(operator(result))]

  def minQ(self, state):
    return self.getQ(state, min)[0]

  def minQ_index(self, state): 
    return self.getQ(state, min)[1]

  def maxQ(self, state):
    return self.getQ(state, max)[0]

  def maxQ_index(self, state):
    return self.getQ(state, max)[1]

  def get_reward(self):
    # closer to target than 0.5m cca 0.15 normalized and rotated less than 30 degrees
    if self.state[0] < 0.15 and abs(self.state[1] - 0.5) < 0.1:
      return 0
    else:
      return 0.1

  def get_target(self): 
    # ToDO: target = Q(st,at) + alpha*(rt+1 + gamma * maxQ(st+1,a) - Q(st,at)) ???
    # closer to target than 0.25m and rotated less than 10 degrees
    if self.state[0] < 0.15 and abs(self.state[1] - 0.5) < 0.1:
      return 0
    else:
      # !!! returning index of max Q, not Q
      print('------------------')
      print(math.pow(self.discount, self.step_counter))
      print(self.minQ(self.state))
      print(0.1 + math.pow(self.discount, self.step_counter) * self.minQ(self.state))
      print('------------------')
      return (0.1 + math.pow(self.discount, self.step_counter) * self.minQ(self.state))




