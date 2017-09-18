import json
import redis
import _rprop as rp
import numpy as np
import math
from pattern_generator import PatternGenerator

action_codes = [
      [1, 0, 0],
      [0, 1, 0],
      [0, 0, 1]
    ]

def get_target(state, step, net): 
    # closer to target than 0.25m and rotated less than 10 degrees
    if state[0] < 0.15 and abs(state[1] - 0.5) < 0.1:
      return 0
    else:
      return (0.01 + 0.94 * getQ(state, net, min))

def sample_to_pattern(sample, net):
    ac = action_codes[:]
    pattern_list = list()
    pattern = list()
    pattern.extend(sample[0:2]) # state in t
    pattern.extend(ac[sample[2]])
    del ac[sample[2]]
    pattern.append(get_target(sample[0:2], sample[-1], net))
    # pattern.append(0) # q-value 0 for the correct decission
    pattern_list.append(pattern)
    for action in ac:
      pattern = list()
      pattern.extend(sample[0:2]) # state in t
      pattern.extend(action)
      pattern.append(getQ(sample[0:2], net, max)) 
      pattern_list.append(pattern)
    return pattern_list

def getQ(state, net, operator, write=False):
    result = list()
    for action in action_codes:
      pattern = list()
      pattern.extend(state)
      pattern.extend(action)
      q = rp.run(np.array(pattern), net)
      result.append(q)
    if write is True:
      print(result)
    return operator(result)

def get_training_set_from_samples(samples, net):
    training_set = list()
    for sample in samples:
      pattern = sample_to_pattern(sample, net)
      training_set.extend(pattern)
    return training_set


if __name__ == "__main__":
  pg = PatternGenerator()
  srv = redis.Redis('localhost')
  net1 = rp.init()
  # net2 = np.array(json.loads(srv.get('net2')))

  # samples = json.loads(srv.get('samples'))
  samples = pg.get_patternset(200)
  training_set = get_training_set_from_samples(samples, net1)

  print 'pattern: ', training_set[-1]
  a = training_set[-1][0:2]
  print(a)
  b = list()
  b.append(a[0])
  b.append(0.3)
  # a.extend([1,0,0,0])
  net3 = rp.learn(500, np.array(training_set), net1)
  for i in range(0, 10):
    training_set = get_training_set_from_samples(samples, net3)
    net3 = rp.learn(500, np.array(training_set), net3)
    srv.set('test1', json.dumps(net3.tolist()))
    samples = pg.get_patternset(200)

  # getQ(a, net2, True)
  # getQ(a, net2)
  getQ(a, net3, min, True)
  # getQ(b, net2)
  getQ(b, net3, min, True)

  # print rp.run(np.array(a), net1)
  # print rp.run(np.array(a), net2)
  # print rp.run(np.array(a), net3)

  srv.set('test1', json.dumps(net3.tolist()))


