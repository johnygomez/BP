/** @file
 * @author Jan Gamec
 * @date 24 May 2015
 *
 * @brief Javascript application for visualisation of NFQ algorithm
 *
 * This application is served as a webpage when running the server application.
 */
var socket;
var tasks;
/** @brief Initializes new websocket connection to server.
 *
 *  Establishes a connection to robot through opening a new websocket connection to server.
 *  In addition it binds corresponding callbacks for websocket events. When a new message is 
 *  received, function serverResponse is called, with a received object.
 */
function connectRobot() {
  // Ip address of the server
  var host = "ws://192.168.43.134:9000/ws";
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

/** @brief Parses received message object from websocket connection
 *
 * This function handles basically 4 types of responses and reactions to them:
 *   - Confirmation about succesful connection to robot - display a control UI.
 *   - Warning about connection error - warns about an error.
 *   - State update - refreshes the UI.
 *   - Result of the performed task - displays final cumulative reward after completion of the task.
 * 
 * @param response A message object received over websocket 
 */
function serverResponse(response) {
  if (response.type === 'info') {
    console.log('Status: ', response.status);
    if (response.status === 'connected') {
      showControls();
      tasks = response.tasks;
      loadTasks(response.tasks);
    } else if (response.status === 'robot problem') {
      showIntro('Could not connect to robot');
    } else if (response.status === 'update') {
      enableAllButtons();
      updateState(response.distance, response.angle);
    } else if (response.status === 'result') {
      enableAllButtons();
      if (response.reward < 0) {
        $('#controls').prepend(
          '<div class="alert alert-warning alert-dismissable" id="save-alert"> \
              <button type="button" class="close" data-dismiss="alert" aria-hidden="true"> \
              &times; \
              </button> \
            Such task was not found! \
          </div>');
      } else {
        $('#controls').prepend(
          '<div class="alert alert-success alert-dismissable" id="save-alert"> \
              <button type="button" class="close" data-dismiss="alert" aria-hidden="true"> \
              &times; \
              </button> \
            Task run with a final cumulative reward ' + response.reward + '\
          </div>');
      }
    }
  } else {
    console.log(response.data);
  }
};

/** @brief Displays a introduction page
 *
 *  @param msg If any warning is received, display it
 */
function showIntro(msg) {
  $('#intro').css('display', 'block');
  $('#controls').css('display', 'none');
  if (typeof msg !== 'undefined') {
    var el = $('#error_message');
    el.css('display','block');
    el.text(msg);
  }
};

/** @brief Displays a NFQ controls and state information.
 *
 */
function showControls() {
  $('#intro').css('display', 'none');
  $('#controls').css('display', 'block');
  document.getElementById('action1').addEventListener("click", function() {
      doAction(0);
  }, false);
  document.getElementById('action2').addEventListener("click", function() {
      doAction(1);
  }, false);
  document.getElementById('action3').addEventListener("click", function() {
      doAction(2);
  }, false);
  document.getElementById('saveBtn').addEventListener("click", function() {
    saveTask();
  }, false);
};

/** @brief Request saving current task progress to the database.
 *
 * Takes a name from corresponding input box, requests a server to save the task in DB.
 */
function saveTask() {
  var taskName = document.getElementById('taskName');
  socket.send(JSON.stringify({
    type: 'save',
    data: taskName.value
  }));
  taskName.value = '';
  $('#saveSection').prepend(
    '<div class="alert alert-success alert-dismissable" id="save-alert"> \
        <button type="button" class="close" data-dismiss="alert" \
        aria-hidden="true"> \
        &times; \
        </button> \
        The task was succesfully saved! \
        </div>');
};

/** @brief Creates a table of all available tasks from DB
 *
 *  @param tasks List of task names from database
 */
function loadTasks(tasks) {
  if (typeof tasks === 'undefined' || tasks.length === 0) {
    $('#tasks').append('<div class="text-center">No tasks have been learned yet</div>');
  } else {
    tasks.forEach(function(task, i) {
      $('#tasks').append('<tr class="row"><td class="col-xs-9 text-left">'+ task +'</td><td class="col-xs-3 text-right"> \
        <button type="button" class="btn btn-success" id="task'+i+'"><span class="glyphicon glyphicon-play" aria-hidden="true"></span>Run</button></td></tr>');
      document.getElementById('task'+i).addEventListener("click", function() {
        runTask(task);
      }, false);
    });
  }
};

/** @brief Requests a server to perform a task on a robot.
 *
 *  @param task Name of the task to be performed
 */
function runTask(task) {
  socket.send(JSON.stringify({
    type: 'run',
    data: task
  }));
  disableAllButtons();
};

/** @brief Order a robot to do selected action.
 *
 * Handles click events on each movement arrows in order to move the "real" robot.
 * @param action_num Number of action from action set
 */
function doAction(action_num) {
  socket.send(JSON.stringify({
    type: 'action',
    data: action_num
  }));
  disableAllButtons();
}; 

/** @brief Enables all buttons on page
 *
 */
function enableAllButtons() {
  $('button').removeAttr("disabled"); 
};

/** @brief Disables all buttons on the page to block request until the current action is finished.
 *
 */
function disableAllButtons() {
  $('button').attr("disabled", "disabled");
};

/** @brief Updates current state information with a new, received over Websocket
 *
 * @param distance New distance state
 * @param angle New angle state
 */
function updateState(distance, angle) {
  document.getElementById('distance').value = (distance * 3.0).toFixed(3) + 'm';
  document.getElementById('angle').value = (angle * 360 - 180).toFixed(2) + '°';
}

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
    
    function showServerResponse(txt) {
      var p = document.createElement('p');
      p.innerHTML = txt;
      document.getElementById('output').appendChild(p);
    }

  }

});