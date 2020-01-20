// plugin_transport_http_client_emscripten_int.js - js for transport plugin
// Copyright (C) 2016  David Barksdale <amatus@amat.us>
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
  abort_xhr: function(xhr) {
    //console.debug('Aborting xhr: ' + xhr);
    xhrs[xhr].abort();
  },
  http_client_plugin_send_int: function(url_pointer, data_pointer, data_size,
                                   cont, cont_cls, target) {
    var url = UTF8ToString(url_pointer);
    var data = HEAP8.subarray(data_pointer, data_pointer + data_size);
    var xhr = new XMLHttpRequest();
    xhr.open('PUT', url);
    xhr.send(data);
    xhr.onload = function(e) {
      //console.debug('put onload readyState ' + xhr.readyState + ' status ' + xhr.status);
      if (cont) {
        dynCall('viiiii', cont, [cont_cls, target, 1, data_size, data_size]);
      }
    };
    xhr.onerror = function(e) {
      //console.debug('put onerror readyState ' + xhr.readyState + ' status ' + xhr.status);
      if (cont) {
        dynCall('viiiii', cont, [cont_cls, target, -1, data_size, data_size]);
      }
    };
  },
  client_connect_get_int: function(get, s, url_pointer, client_receive,
                              session_disconnect, plugin) {
    var url = UTF8ToString(url_pointer);
    //console.debug('Creating new get xhr: ' + get);
    var xhr = new XMLHttpRequest();
    xhrs[get] = xhr;
    xhr.responseType = 'arraybuffer';
    xhr.resend = function() {
      xhr.open('GET', url + ',1');
      xhr.send();
    };
    xhr.onreadystatechange = function() {
      //console.debug('xhr' + get + ' readyState ' + xhr.readyState +
      //    ' status ' +  xhr.status + ':' + xhr.statusText);
    };
    xhr.onload = function(e) {
      var response = new Uint8Array(e.target.response);
      //console.debug('xhr' + get + ' got ' + response.length + ' bytes');
      ccallFunc(getFuncWrapper(client_receive, 'iiiii'), 'number',
        ['array', 'number', 'number', 'number'],
        [response, response.length, 1, s]);
      xhr.resend();
    };
    xhr.onerror = function(e) {
      //console.debug('xhr' + get + ' status:'
      //  + xhr.status + ':' + xhr.statusText);
      ccallFunc(
        getFuncWrapper(session_disconnect, 'iii'),
        'number',
        ['number', 'number'],
        [plugin, s]);
    };
    xhr.onabort = function() {
      //console.debug('xhr' + get + ' aborted');
    };
    xhr.ontimeout = function() {
      //console.debug('xhr' + get + ' timedout');
      xhr.resend();
    };
    xhr.resend();
  }
});

// vim: set expandtab ts=2 sw=2:
