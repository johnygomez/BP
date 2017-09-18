import json
import math
import redis
import _rprop as rp
import numpy as np

from Agent import Agent


class NfqTest:
  # representation of action taken for input neurons in network
  action_codes = [
    [1, 0, 0],
    [0, 1, 0],
    [0, 0, 1]
  ]

  def __init__(self):
    self.agent = Agent()
    self.connected = self.agent.connect(ip = '192.168.43.96')
    self.redis_server = redis.Redis('localhost')
    self.load_net()
    self.network = rp.init() if self.network is None else self.network
    self.state = [1.0, 1.0]
    self.step_counter = 1
    self.reward = 0
    self.stateList = list()


  def refresh_state(self):
    '''Has to implement this method to read states from environment'''
    try:
      self.state[0] = float(self.redis_server.get('distance'))
      self.state[1] = float(self.redis_server.get('angle'))
      self.stateList.append(self.state[:])
    except Exception, e:
      print('Could not parse state, reseting to last value')

  def load_net(self):
    _net = self.redis_server.get('net5')
    if _net is not None:
      _net_parsed = json.loads(_net)
      assert type(_net_parsed) is list
      self.network = np.array(_net_parsed)
    else:
      self.network = None

  def write_state_history(self):
    with open("stateHist.txt", "w+") as f:
      for st in self.stateList:
        f.write(str(st[0]) + ', ' + str(st[1]) + '\n')

  def run(self):
    self.refresh_state()
    self.do_action(self.minQ_index(self.state))
    self.refresh_state()
    self.reward += self.get_reward()
    if self.get_reward() == 0:
      print('Number of steps: ', self.step_counter)
      print('with total reward: ', self.reward)
      print(self.stateList)
      self.write_state_history()
      return
    elif self.step_counter > 13:
      print(self.stateList)
      self.write_state_history()
      return
    else:
      self.run()

  def do_action(self, action_num):
    print('Taking action', action_num)
    self.agent.do(action_num)
    self.step_counter += 1

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
        return 0.01

if __name__ == "__main__":
  test = NfqTest()
  test.run()