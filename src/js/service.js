// service.js - service routines for gnunet-web services
// Copyright (C) 2013,2014  David Barksdale <amatus@amatus.name>
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
  GNUNET_SERVICE_run: function(argc, argv, service_name, options, task,
                               task_cls) {
    var server = 1; // opaque non-null pointer
    var cfg = 2; // same
    Runtime.dynCall('viii', task, [task_cls, server, cfg]);
  }
});

// vim: set expandtab ts=2 sw=2:
