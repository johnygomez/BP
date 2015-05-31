import json
import os
import tornado.ioloop
import tornado.web
import tornado.websocket
import tornado.template

from NFQ import NFQ
## Operates a web server for web application
#
class MainHandler(tornado.web.RequestHandler):
  ## Handles asynchronous web request and renders a web app
  #
  # @param request Web request received
  @tornado.web.asynchronous
  def get(request):
    request.render('index.html')

## Handles websocket communication with web app
#
class WSHandler(tornado.websocket.WebSocketHandler):
  ## Handles all new connections.
  # 
  # Initializes a new NFQ instance for each new WS connection
  def open(self):
    print 'connection opened...'
    self.nfq = NFQ()
    self.write_message(json.dumps({
      'type': 'info',
      'status': 'connected' if self.nfq.connected else 'robot problem',
      'tasks': self.nfq.load_tasks()
      }))
  ## Handles various type of messages received over WS.
  #
  # It distinc 3 main commands:
  # - Performing an action.
  # - Saving current task.
  # - Running loaded task.
  #
  # It also responds with corresponding data
  # @param json_message Data received in WS request (like action number)
  def on_message(self, json_message):
    message = json.loads(json_message)
    if message['type'] == 'action':
      self.nfq.do_action(int(message['data']))
      self.write_message(json.dumps({
        'type': 'info',
        'status': 'update',
        'distance': self.nfq.state[0],
        'angle': self.nfq.state[1]
        }))
    elif message['type'] == 'save':
      self.nfq.save_net(message['data'])
    elif message['type'] == 'run':
      response = self.nfq.run_task(message['data'])
      self.write_message(json.dumps({
        'type': 'info',
        'status': 'result',
        'reward': float(response)
        }))
    else:
      self.write_message("The server says: " + message + " back at you")
      print 'received:', message
  
  ## Handles close of WS connection from client
  def on_close(self):
    print 'connection closed...'

## Server setting for templates and static files paths
#
settings = {
  'debug': True,
  'template_path': os.path.abspath(os.path.join(os.path.dirname( __file__ ), '..', 'app')),
  'static_path': os.path.abspath(os.path.join(os.path.dirname( __file__ ), '..', 'app'))
}

## Default web paths available (app, websocket and statics)
#
application = tornado.web.Application([
  (r'/ws', WSHandler),
  (r'/', MainHandler),
  (r"/(.*)", tornado.web.StaticFileHandler, {"path": "./"}),
], **settings)

if __name__ == "__main__":
  application.listen(9000)
  tornado.ioloop.IOLoop.instance().start()