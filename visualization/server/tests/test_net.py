import json
import redis
import _rprop as rp
import numpy as np

if __name__ == "__main__":
  srv = redis.Redis('localhost')
  net = np.array(json.loads(srv.get('net')))
  pattern1 = [0.5, 0.7, 1, 0, 0]
  pattern2 = [0.5, 0.7, 0, 1, 0]
  pattern3 = [0.5, 0.7, 0, 0, 1]
  res = rp.run(np.array(pattern1), net)
  print(res)
  res = rp.run(np.array(pattern2), net)
  print(res)
  res = rp.run(np.array(pattern3), net)
  print(res)