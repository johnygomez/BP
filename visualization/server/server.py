import json
import os
import tornado.ioloop
import tornado.web
import tornado.websocket
import tornado.template

from NFQ import NFQ

class MainHandler(tornado.web.RequestHandler):
  @tornado.web.asynchronous
  def get(request):
    request.render('index.html')

class WSHandler(tornado.websocket.WebSocketHandler):
  def open(self):
    print 'connection opened...'
    self.nfq = NFQ()
    self.write_message(json.dumps({
      'type': 'info',
      'status': 'connected' if self.nfq.connected else 'robot problem'
      }))

  def on_message(self, json_message):
    message = json.loads(json_message)
    if message['type'] == 'action':
      self.nfq.do_action(message['data'])
    else:
      self.write_message("The server says: " + message + " back at you")
      print 'received:', message

  def on_close(self):
    print 'connection closed...'



settings = {
  'debug': True,
  'template_path': os.path.abspath(os.path.join(os.path.dirname( __file__ ), '..', 'app')),
  'static_path': os.path.abspath(os.path.join(os.path.dirname( __file__ ), '..', 'app'))
}
application = tornado.web.Application([
  (r'/ws', WSHandler),
  (r'/', MainHandler),
  (r"/(.*)", tornado.web.StaticFileHandler, {"path": "./"}),
], **settings)

if __name__ == "__main__":
  application.listen(9000)
  tornado.ioloop.IOLoop.instance().start()