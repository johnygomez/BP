import naoqi
from Agent import Agent

## Implementation of Agent interface on Nao robot in Webots simulator
#
class NaoAgent(Agent):
  ## Constructor method setups default parameters
  def __init__(self):
    ## Default connection IP address
    #
    self.robot_ip = 'Gomez-lx.local'
    ## Default connection port
    #
    self.robot_port = 9559

  ## Method handles connection to simulated robot and initialization of its features
  #
  # It connection to the robot on specified Ip:port and initializes the robot to the init position,
  # preparing it for motions.
  # @param ip IP address of a robot
  # @param port Port number of a robot
  # @return True, if connection was succesful, else False
  def connect(self, ip = None, port = None):
    ip = ip if ip is not None else self.robot_ip
    port = port if port is not None else self.robot_port
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

  ## Disconnects from simulated Nao
  #
  def disconnect(self): 
    self.behaviorProxy = None
    self.motionProxy = None

  ## Perform specified action from the set of all available motions.
  #
  # @param action_number specifies position of action in the action list
  def do(self, action_number):
    assert type(action_number) is int
    action_list = [
      self.go_forward,
      self.go_left,
      self.go_right,
    ]
    action_list[action_number % len(action_list)]()

  ## This method performs movements along the X-axis, with 0.25m distance
  #
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

  ## This method performs a rotation 30 degrees to the right
  #
  def go_right(self):
    x = 0.1
    y = -0.1
    # pi/6 clockwise
    theta = -0.5235
    self.motionProxy.moveInit()
    self.postureProxy.goToPosture("StandInit", 0.5)

    self.motionProxy.setMoveArmsEnabled(True, True)
    self.motionProxy.setMotionConfig([["ENABLE_FOOT_CONTACT_PROTECTION", True]])
    self.motionProxy.post.moveTo(x, y, theta)
    self.motionProxy.waitUntilMoveIsFinished()
  
  ## This method perform movement 30 degrees left
  #
  def go_left(self):
    x = 0.1
    y = 0.1
    # pi/6 anti-clockwise
    theta = 0.5235
    self.motionProxy.moveInit()
    self.postureProxy.goToPosture("StandInit", 0.5)

    self.motionProxy.setMoveArmsEnabled(True, True)
    self.motionProxy.setMotionConfig([["ENABLE_FOOT_CONTACT_PROTECTION", True]])
    self.motionProxy.post.moveTo(x, y, theta)
    self.motionProxy.waitUntilMoveIsFinished()
  
  ## Sets the stiffnes of Nao's joints to maximal value, to make him ready to perform motions
  #
  # @param proxy specifies a proxy(path) for setting the joint's stiffness
  def StiffnessOn(self, proxy):
    pNames = "Body"
    pStiffnessLists = 1.0
    pTimeLists = 1.0
    proxy.stiffnessInterpolation(pNames, pStiffnessLists, pTimeLists)
