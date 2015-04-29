import json
import redis
import _rprop as rp
import numpy as np

def getQ(state, net):
    result = list()
    action_codes = [
      [1, 0, 0, 0],
      [0, 1, 0, 0],
      [0, 0, 1, 0],
      [0, 0, 0, 1]
    ]
    for action in action_codes:
      pattern = list()
      pattern.extend(state)
      pattern.extend(action)
      q = rp.run(np.array(pattern), net)
      result.append(q)
    print(result)

srv = redis.Redis('localhost')
net1 = rp.init()
net2 = np.array(json.loads(srv.get('net')))

patset = json.loads(srv.lrange('patternset',1,1)[0])

print('pattern: ', patset[-1])
a = patset[-1][0:2]
# a.extend([1,0,0,0])
net3 = rp.learn(20000, np.array(patset), net1)
print(a)
getQ(a, net1)
getQ(a, net2)
getQ(a, net3)
# print rp.run(np.array(a), net1)
# print rp.run(np.array(a), net2)
# print rp.run(np.array(a), net3)


srv.set('net', json.dumps(net3.tolist()))

