import numpy as np
import _rprop as rp

patternSet = np.array([[1.0,0.0,1.0], [0.0,1.0,1.0], [0.0,0.0,0.0], [1.0,1.0,0.0]])

a = rp.init()
print a
b = rp.learn(100, patternSet, a)
print b
res = rp.run(np.array([1.0,0.3,1.0]), b)
print res