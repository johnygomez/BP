import json
import redis
import _rprop as rp
import numpy as np

srv = redis.Redis('localhost')
net1 = rp.init()
net2 = np.array(json.loads(srv.get('net')))

patset = json.loads(srv.get('patternset'))

print('pattern: ', patset[-1])
a = patset[-1][0:2]
a.extend([0,0,0,1])
net3 = rp.learn(10000, np.array(patset), net1)
print rp.run(np.array(a), net1)
print rp.run(np.array(a), net2)
print rp.run(np.array(a), net3)
# srv.set('net', json.dumps(net3.tolist()))
# srv.set('net', json.dumps(net3.tolist()))

