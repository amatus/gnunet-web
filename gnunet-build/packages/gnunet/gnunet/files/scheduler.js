// scheduler.js - scheduler routines for gnunet-web services
// Copyright (C) 2013-2016  David Barksdale <amatus@amat.us>
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.

mergeInto(LibraryManager.library, {
  $SCHEDULER_TASKS: {},
  GNUNET_SCHEDULER_add_delayed_with_priority_js__deps: ['$SCHEDULER_TASKS'],
  GNUNET_SCHEDULER_add_delayed_with_priority_js:
  function(delay, priority, task, task_cls) {
    //Module.print('GNUNET_SCHEDULER_add_delayed_with_priority(delay='+delay+',pirority='+priority+',task='+task+',task_cls='+task_cls+')');
    var id;
    id = setTimeout(function() {
      delete SCHEDULER_TASKS[id];
      Runtime.dynCall('vi', task, [task_cls]);
    }, delay);
    SCHEDULER_TASKS[id] = {cls: task_cls};
    return id;
  },
  GNUNET_SCHEDULER_add_read_file: function(delay, rfd, task, task_cls) {
    abort();
  },
  GNUNET_SCHEDULER_cancel__deps: ['$SCHEDULER_TASKS'],
  GNUNET_SCHEDULER_cancel: function(task) {
    console.debug("cancelling task", task);
    clearTimeout(task);
    if (task in SCHEDULER_TASKS) {
      var t = SCHEDULER_TASKS[task];
      delete SCHEDULER_TASKS[task];
      if ("socket" in t) {
        if (t.socket in SOCKETS) {
          delete SOCKETS[t.socket]["task"];
        }
      }
      return t.cls;
    }
    return 0;
  },
  GNUNET_SCHEDULER_add_read_net__deps: ['$SOCKETS'],
  GNUNET_SCHEDULER_add_read_net: function(delay, rfd, task, task_cls) {
    console.debug("add_read_net(", delay, rfd, task, task_cls, ")");
    if (!(rfd in SOCKETS)) {
      console.error("socket is not connected?");
      return 0;
    }
    var socket = SOCKETS[rfd];
    if ("task" in socket) {
      console.error("socket already has a read handler");
    }
    var id = setTimeout(function() {
      if (SOCKETS.listening == rfd) {
        if (0 == SOCKETS.incoming.length) {
          return;
        }
      } else if (0 == socket.queue.length) {
        return;
      }
      if (!(id in SCHEDULER_TASKS)) {
        return;
      }
      delete SCHEDULER_TASKS[id];
      delete socket["task"];
      Runtime.dynCall('vi', task, [task_cls]);
    }, 0);
    SCHEDULER_TASKS[id] = {
      cls: task_cls,
      socket: rfd,
    };
    socket["handler"] = task;
    socket["cls"] = task_cls;
    socket["task"] = id;
    console.debug("read task is", id);
    return id;
  },
  GNUNET_SCHEDULER_add_read_net_with_priority__deps: [
    'GNUNET_SCHEDULER_add_read_net'
  ],
  GNUNET_SCHEDULER_add_read_net_with_priority: function(delay, priroity, rfd,
      task, task_cls) {
    return _GNUNET_SCHEDULER_add_read_net(delay, rfd, task, task_cls);
  },
  GNUNET_SCHEDULER_add_write_net__deps: ['$SOCKETS'],
  GNUNET_SCHEDULER_add_write_net: function(delay, wfd, task, task_cls) {
    console.debug("add_write_net(", delay, wfd, task, task_cls, ")");
    if (!(wfd in SOCKETS)) {
      console.error("socket is not connected?");
      return 0;
    }
    // always writable
    var id = setTimeout(function() {
      if (!(id in SCHEDULER_TASKS)) {
        return;
      }
      delete SCHEDULER_TASKS[id];
      Runtime.dynCall('vi', task, [task_cls]);
    }, 0);
    SCHEDULER_TASKS[id] = {cls: task_cls};
    return id;
  },
  GNUNET_SCHEDULER_run: function(task, task_cls) {
    Runtime.dynCall('vi', task, [task_cls]);
    throw 'SimulateInfiniteLoop';
  },
});

// vim: set expandtab ts=2 sw=2:
