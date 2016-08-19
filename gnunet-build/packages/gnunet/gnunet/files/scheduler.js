// scheduler.js - scheduler routines for gnunet-web services
// Copyright (C) 2013-2015  David Barksdale <amatus@amatus.name>
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
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
    SCHEDULER_TASKS[id] = task_cls;
    return id;
  },
  GNUNET_SCHEDULER_add_read_file: function(delay, rfd, task, task_cls) {
    abort();
  },
  GNUNET_SCHEDULER_cancel__deps: ['$SCHEDULER_TASKS'],
  GNUNET_SCHEDULER_cancel: function(task) {
    clearTimeout(task);
    if (task in SCHEDULER_TASKS) {
      var cls = SCHEDULER_TASKS[task];
      delete SCHEDULER_TASKS[task];
      return cls;
    }
    return 0;
  }
});

// vim: set expandtab ts=2 sw=2:
