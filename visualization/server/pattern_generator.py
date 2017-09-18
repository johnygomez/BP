import random
import json
import redis
from math import *

class PatternGenerator:
  def __init__(self):
    self.state = self.get_random_state()
    self.steps = 1
    self.epoch = 1

  def get_random_state(self):
    distance = random.normalvariate(0.5, 0.20)
    angle = random.normalvariate(0.5, 0.20)
    if distance > 1.0 or distance < 0:
      distance = distance % 1.0
    if angle > 1.0 or angle < 0:
      angle = angle % 1.0
    return [distance, angle]

  def get_state(self):
    if self.state[0] < 0.1 and abs(self.state[1] - 0.5) < 0.1:
      self.state = self.get_random_state()
      self.steps = 1
      self.epoch += 1
    return self.state

  def get_action(self, state):
    if state[1] > 0.55:
      return 1
    elif state[1] < 0.45: 
      return 2
    else:
      return 0

  def get_next_state(self, state, action):
    prev_distance = state[0] * 3.0 # denormalized distance
    prev_angle_sign = copysign(1, (state[1] - 0.5))
    prev_angle_d = abs((state[1] * 360) - 180) # denormalized degrees
    prev_angle_r = radians(prev_angle_d)       # radians
    
    if action == 0:
      step_len = 0.25
      # law of cosines
      distance = sqrt(prev_distance**2 + step_len**2 - 2*(step_len * prev_distance * cos(prev_angle_r)))
      angle_d = copysign(180 - degrees(acos((distance**2 + step_len**2 - prev_distance**2)/(2.0 * distance * step_len))), prev_angle_sign)
    elif action == 1 or action == 2:
      step_len = 0.1
      angle_d = prev_angle_d - 30
      angle_r = radians(angle_d)
      walk_dist = step_len * sqrt(2)
      distance = sqrt(walk_dist**2 + prev_distance**2 - (2 * prev_distance * walk_dist * cos(angle_r)))
      angle_d = copysign(angle_d, prev_angle_sign)
    angle_normalized = (angle_d + 180) / 360
    distance_normalized = distance / 3.0
    self.state = [distance_normalized, angle_normalized]
    return self.state

  def get_pattern(self):
    pattern = list()
    state_old = self.get_state()
    action = self.get_action(state_old)
    state_new = self.get_next_state(state_old, action)
    pattern.extend(state_old)
    pattern.append(action)
    pattern.extend(state_new)
    pattern.append(self.epoch)
    pattern.append(self.steps)
    self.steps += 1
    return pattern

  def get_patternset(self, count):
    patternset = list()
    for i in range(count):
      patternset.append(self.get_pattern())
    return patternset

if __name__ == "__main__":
  pg = PatternGenerator()
  srv = redis.Redis('localhost')
  # print(pg.get_pattern())
  patternset = pg.get_patternset(5000)
  srv.set('samples', json.dumps(patternset))
  print('Done!')


