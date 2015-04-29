var socket;
var connectRobot = function() {
  var host = "ws://localhost:9000/ws";
  socket = new WebSocket(host);

  if (socket) {

    socket.onopen = function() {
      //alert("connection opened....");
    }

    socket.onmessage = function(msg) {
      serverResponse(JSON.parse(msg.data));
    }

    socket.onclose = function() {
      //alert("connection closed....");
      // showServerResponse("The connection has been closed.");
    }

  } else {
    console.log("invalid socket");
  }
};

var serverResponse = function(response) {
  if (response.type === 'info') {
    console.log('Status: ', response.status);
    if (response.status === 'connected') {
      showControls();
    } else if (response.status === 'robot problem') {
      showIntro('Could not connect to robot');
    }
  } else {
    console.log(response.data);
  }
};

var showIntro = function(msg) {
  var container = document.getElementById('mainContainer');
  container.innerHTML = '                                                               \
    <div class="row text-center">                                                       \
      <h1>NAO Robot Control</h1>                                                        \
      <h3 class="text-danger">' + msg + '</h3>                                         \
      <button class="btn btn-lg btn-primary" id="connectBtn">Start Teleoperation</btn>  \
    </div>';
};

var showControls = function() {
  var container = document.getElementById('mainContainer');
  container.innerHTML = '                                                               \
    <div class="row text-center">                                                       \
      <h1>Controlling NAO</h1>                                                          \
      <div class="row"><button class="btn btn-lg btn-primary col-md-4 col-md-offset-4" id="action1">Go forward</btn></div>           \
      <div class="row"><button class="btn btn-lg btn-primary col-md-4 col-md-offset-4" id="action2">Go left</btn></div>              \
      <div class="row"><button class="btn btn-lg btn-primary col-md-4 col-md-offset-4" id="action3">Go right</btn></div>             \
      <div class="row"><button class="btn btn-lg btn-primary col-md-4 col-md-offset-4" id="action4">Go backward</btn></div>          \
    </div>';
  document.getElementById('action1').addEventListener("click", function() {
      doAction(0);
  }, false);
  document.getElementById('action2').addEventListener("click", function() {
      doAction(1);
  }, false);
  document.getElementById('action3').addEventListener("click", function() {
      doAction(2);
  }, false);
  document.getElementById('action4').addEventListener("click", function() {
      doAction(3);
  }, false);
};

var doAction = function(action_num) {
  socket.send(JSON.stringify({
    type: 'action',
    data: action_num
  }));
};

jQuery(function($) {

  if (!("WebSocket" in window)) {
    alert("Your browser does not support web sockets");
  } else {
    setup();
  }

  function setup() {
    $('#connectBtn').on('click', function() {
      connectRobot();
    });
    // var $txt = $("#data");
    // var $btnSend = $("#sendtext");

    // $txt.focus();

    // // event handlers for UI
    // $btnSend.on('click', function() {
    //   var text = $txt.val();
    //   if (text == "") {
    //     return;
    //   }
    //   socket.send(text);
    //   $txt.val("");
    // });

    // $txt.keypress(function(evt) {
    //   if (evt.which == 13) {
    //     $btnSend.click();
    //   }
    // });

    // // event handlers for websocket
    // if (socket) {

    //   socket.onopen = function() {
    //     //alert("connection opened....");
    //   }

    //   socket.onmessage = function(msg) {
    //     showServerResponse(msg.data);
    //   }

    //   socket.onclose = function() {
    //     //alert("connection closed....");
    //     showServerResponse("The connection has been closed.");
    //   }

    // } else {
    //   console.log("invalid socket");
    // }

    function showServerResponse(txt) {
      var p = document.createElement('p');
      p.innerHTML = txt;
      document.getElementById('output').appendChild(p);
    }

  }

});