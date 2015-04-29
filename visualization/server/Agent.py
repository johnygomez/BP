import naoqi

class Agent:
  def __init__(self):
    self.robot_ip = 'Gomez-lx.local'
    self.robot_port = 9559

  def connect(self, ip = None, port = None):
    ip = ip if ip is not None else self.robot_ip
    port = port if port is not None else self.robot_port
    # try:
    #   self.behaviorProxy = naoqi.ALProxy('ALBehaviorManager', self.robot_ip, self.robot_port)
    # except Exception, e:
    #   print "Could not create proxy to ALBehaviorManager"
    #   print "Error was: ", e
    #   return False

    try:
      self.motionProxy = naoqi.ALProxy("ALMotion", ip, port)
    except Exception, e:
      print "Could not create proxy to ALMotion"
      print "Error was: ", e
      return False

    try:
      self.postureProxy = naoqi.ALProxy("ALRobotPosture", ip, port)
    except Exception, e:
      print "Could not create proxy to ALRobotPosture"
      print "Error was: ", e
      return False

    self.motionProxy.wakeUp()
    self.StiffnessOn(self.motionProxy)
    return True

  def disconnect(self): 
    self.behaviorProxy = None
    self.motionProxy = None


  def do(self, action_number):
    assert type(action_number) is int
    action_list = [
      self.go_forward,
      self.go_left,
      self.go_right,
      self.go_back,
    ]
    action_list[action_number % len(action_list)]()

  def go_forward(self):
    # 0.25m forward
    x = 0.25
    y = 0.0
    theta = 0.0
    self.motionProxy.moveInit()
    self.postureProxy.goToPosture("StandInit", 0.5)

    self.motionProxy.setMoveArmsEnabled(True, True)
    self.motionProxy.setMotionConfig([["ENABLE_FOOT_CONTACT_PROTECTION", True]])
    self.motionProxy.post.moveTo(x, y, theta)
    self.motionProxy.waitUntilMoveIsFinished()

  def go_right(self):
    x = 0.25
    y = 0.0
    # pi/6 clockwise
    theta = -0.5235
    self.motionProxy.moveInit()
    self.postureProxy.goToPosture("StandInit", 0.5)

    self.motionProxy.setMoveArmsEnabled(True, True)
    self.motionProxy.setMotionConfig([["ENABLE_FOOT_CONTACT_PROTECTION", True]])
    self.motionProxy.post.moveTo(x, y, theta)
    self.motionProxy.waitUntilMoveIsFinished()

  def go_left(self):
    x = 0.25
    y = 0.0
    # pi/6 anti-clockwise
    theta = 0.5235
    self.motionProxy.moveInit()
    self.postureProxy.goToPosture("StandInit", 0.5)

    self.motionProxy.setMoveArmsEnabled(True, True)
    self.motionProxy.setMotionConfig([["ENABLE_FOOT_CONTACT_PROTECTION", True]])
    self.motionProxy.post.moveTo(x, y, theta)
    self.motionProxy.waitUntilMoveIsFinished()

  def go_back(self):
    # 0.25m backwards
    x = -0.25
    y = 0.0
    theta = 0.0
    self.motionProxy.moveInit()
    self.postureProxy.goToPosture("StandInit", 0.5)

    self.motionProxy.setMoveArmsEnabled(True, True)
    self.motionProxy.setMotionConfig([["ENABLE_FOOT_CONTACT_PROTECTION", True]])
    self.motionProxy.post.moveTo(x, y, theta)
    self.motionProxy.waitUntilMoveIsFinished()

  def StiffnessOn(self, proxy):
    # We use the "Body" name to signify the collection of all joints
    pNames = "Body"
    pStiffnessLists = 1.0
    pTimeLists = 1.0
    proxy.stiffnessInterpolation(pNames, pStiffnessLists, pTimeLists)
