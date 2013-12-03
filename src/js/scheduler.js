// scheduler.js - scheduler routines for gnunet-web services
// Copyright (C) 2013  David Barksdale <amatus@amatus.name>
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
  GNUNET_SCHEDULER_add_delayed_with_priority:
  function(delay, priority, task, task_cls) {
    if (delay) {
      delay = getValue(delay, 'i64');
    }
    //Module.print('GNUNET_SCHEDULER_add_delayed_with_priority(delay='+delay+',pirority='+priority+',task='+task+',task_cls='+task_cls+')');
    if (-1 == delay) {
      // This is the shutdown task, ignore for now
      return 0;
    }
    return setTimeout(function() {
      Runtime.dynCall('vii', task, [task_cls, 0]);
    }, delay / 1000);
  },
  GNUNET_SCHEDULER_add_delayed__deps:
  ['GNUNET_SCHEDULER_add_delayed_with_priority'],
  GNUNET_SCHEDULER_add_delayed: function(delay, task, task_cls) {
    return _GNUNET_SCHEDULER_add_delayed_with_priority(delay, 0, task,
      task_cls);
  },
  GNUNET_SCHEDULER_add_now__deps: ['GNUNET_SCHEDULER_add_delayed'],
  GNUNET_SCHEDULER_add_now: function(task, task_cls) {
    return _GNUNET_SCHEDULER_add_delayed(0, task, task_cls);
  },
});

// vim: set expandtab ts=2 sw=2:
