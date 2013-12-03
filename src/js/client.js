// client.js - client routines for gnunet-web services
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
  $CLIENT_PORTS: {},
  $NEXT_PORT: 1,
  GNUNET_CLIENT_connect__deps: ['$CLIENT_PORTS', '$NEXT_PORT'],
  GNUNET_CLIENT_connect: function(service_name, cfg) {
    var service_name = Pointer_stringify(service_name);
    var channel = new MessageChannel();
    var port = NEXT_PORT;
    NEXT_PORT = port + 1;
    CLIENT_PORTS[port] = channel.port1;
    client_connect(service_name, channel.port2);
    return port;
  },
});

// vim: set expandtab ts=2 sw=2:
