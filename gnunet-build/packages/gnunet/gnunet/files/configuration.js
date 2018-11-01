// configuration.js - config data handler for gnunet-web services
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
  $CONFIG: {
    peer: {
      PRIVATE_KEY: '/private_key',
    },
    statistics: {
      DISABLE: true,
    },
    transport: {
      UNIXPATH: 'transport',
      NEIGHBOUR_LIMIT: 50,
      PLUGINS: 'http_client',
    },
    ats: {
      UNIXPATH: 'ats',
      UNSPECIFIED_QUOTA_OUT: '65536',
      UNSPECIFIED_QUOTA_IN: '65536',
      LOOPBACK_QUOTA_OUT: '65536',
      LOOPBACK_QUOTA_IN: '65536',
      LAN_QUOTA_OUT: '65536',
      LAN_QUOTA_IN: '65536',
      WAN_QUOTA_OUT: '65536',
      WAN_QUOTA_IN: '65536',
      WLAN_QUOTA_OUT: '65536',
      WLAN_QUOTA_IN: '65536',
      BLUETOOTH_QUOTA_OUT: '65536',
      BLUETOOTH_QUOTA_IN: '65536',
    },
    core: {
      UNIXPATH: 'core',
      USE_EPHEMERAL_KEYS: true,
    },
    dht: {
      UNIXPATH: 'dht',
      CACHE_RESULTS: true,
      DISABLE_TRY_CONNECT: false,
    },
    dhtcache: {
      QUOTA: '8 MB',
      DATABASE: 'heap',
      DISABLE_BF: true,
    },
    nse: {
      UNIXPATH: 'nse',
      PROOFFILE: '/nse/proof.dat',
      WORKDELAY: '5 ms',
      INTERVAL: '1 h',
      WORKBITS: 22,
    },
    cadet: {
      UNIXPATH: 'cadet',
      MAX_MSGS_QUEUE: 10000,
      MAX_CONNECTIONS: 1000,
      REFRESH_CONNECTION_TIME: '5 min',
      ID_ANNOUNCE_TIME: '1 h',
    },
    datastore: {
      UNIXPATH: 'datastore',
      DATABASE: 'emscripten',
      QUOTA: '1 GB',
      BLOOMFILTER: '/datastore/bloomfilter',
    },
    fs: {
      UNIXPATH: 'fs',
      DELAY: true,
      CONTENT_CACHING: true,
      CONTENT_PUSHING: true,
      MAX_CADET_CLIENTS: 128,
      RESPECT: '/fs/credit',
    },
    peerinfo: {
      UNIXPATH: 'peerinfo',
      HOSTS: '/peerinfo/hosts',
    },
    peerstore: {
      UNIXPATH: 'peerstore',
      DATABASE: 'emscripten',
    },
    arm: {
      CONFIG: '',
    },
  },
  GNUNET_CONFIGURATION_get_value__deps: ['$CONFIG'],
  GNUNET_CONFIGURATION_get_value: function(section, option) {
    var section = Pointer_stringify(section).toLowerCase();
    var option = Pointer_stringify(option).toUpperCase();
    console.debug('GNUNET_CONFIGURATION_get_value(' + section + ',' + option
          + ')');
    if (!(section in CONFIG)) {
      console.debug(section + ' not in CONFIG');
      return undefined;
    }
    var sec = CONFIG[section]
    if (!(option in sec)) {
      console.debug(option + ' not in ' + section);
      return undefined;
    }
    console.debug('found:', sec[option]);
    return sec[option];
  },
  GNUNET_CONFIGURATION_have_value__deps:
    ['GNUNET_CONFIGURATION_get_value'],
  GNUNET_CONFIGURATION_have_value: function(cfg, section, option) {
    return undefined !== _GNUNET_CONFIGURATION_get_value(section, option);
  },
  GNUNET_CONFIGURATION_get_value_string__deps:
    ['GNUNET_CONFIGURATION_get_value', 'GNUNET_xstrdup_'],
  GNUNET_CONFIGURATION_get_value_string: function(cfg, section, option, value) {
    var tmp = _GNUNET_CONFIGURATION_get_value(section, option);
    if (undefined === tmp)
      return -1;
    var tmp = ccall('GNUNET_xstrdup_', 'number',
        ['string', 'string', 'number'],
        [tmp, 'configuration.js', 0]);
    setValue(value, tmp, 'i32');
    return 1;
  },
  GNUNET_CONFIGURATION_get_value_filename__deps:
    ['GNUNET_CONFIGURATION_get_value_string'],
  GNUNET_CONFIGURATION_get_value_filename: function(cfg, section, option,
                                                    value) {
    return _GNUNET_CONFIGURATION_get_value_string(cfg, section, option, value);
  },
  GNUNET_CONFIGURATION_get_value_yesno__deps:
    ['GNUNET_CONFIGURATION_get_value'],
  GNUNET_CONFIGURATION_get_value_yesno: function(cfg, section, option) {
    var tmp = _GNUNET_CONFIGURATION_get_value(section, option);
    if (undefined === tmp)
      return -1;
    return tmp;
  },
  GNUNET_CONFIGURATION_get_value_time__deps:
    ['GNUNET_CONFIGURATION_get_value', 'GNUNET_STRINGS_fancy_time_to_relative'],
  GNUNET_CONFIGURATION_get_value_time: function(cfg, section, option, time) {
    var tmp = _GNUNET_CONFIGURATION_get_value(section, option);
    if (undefined === tmp)
      return -1;
    return ccall('GNUNET_STRINGS_fancy_time_to_relative', 'number',
        ['string', 'number'],
        [tmp, time]);
  },
  GNUNET_CONFIGURATION_get_value_size__deps:
    ['GNUNET_CONFIGURATION_get_value', 'GNUNET_STRINGS_fancy_size_to_bytes'],
  GNUNET_CONFIGURATION_get_value_size: function(cfg, section, option, size) {
    var tmp = _GNUNET_CONFIGURATION_get_value(section, option);
    if (undefined === tmp)
      return -1;
    return ccall('GNUNET_STRINGS_fancy_size_to_bytes', 'number',
        ['string', 'number'],
        [tmp, size]);
  },
  GNUNET_CONFIGURATION_get_value_number__deps:
    ['GNUNET_CONFIGURATION_get_value'],
  GNUNET_CONFIGURATION_get_value_number: function(cfg, section, option, num) {
    var tmp = _GNUNET_CONFIGURATION_get_value(section, option);
    if (undefined === tmp)
      return -1;
    setValue(num, tmp, 'i64');
    return 1;
  },
  GNUNET_CONFIGURATION_get_value_float__deps:
    ['GNUNET_CONFIGURATION_get_value'],
  GNUNET_CONFIGURATION_get_value_float: function(cfg, section, option, num) {
    var tmp = _GNUNET_CONFIGURATION_get_value(section, option);
    if (undefined === tmp)
      return -1;
    setValue(num, tmp, 'float');
    return 1;
  },
  GNUNET_CONFIGURATION_iterate_section_values__deps: ['$CONFIG'],
  GNUNET_CONFIGURATION_iterate_section_values:
  function(cfg, section, iter, iter_cls) {
    var section_name = Pointer_stringify(section);
    console.debug('GNUNET_CONFIGURATION_iterate_section_values(' + section_name
          + ')');
    if (!(section_name in CONFIG)) {
      Module.print(section_name + ' not in CONFIG');
      return;
    }
    section = CONFIG[section_name];
    for (var option in section) {
      var value = section[option];
      ccallFunc(getFuncWrapper(iter, 'viiii'), 'void',
        ['i32', 'string', 'string', 'string'],
        [iter_cls, section_name, option, value]);
    }
  },
  GNUNET_CONFIGURATION_set_value_string: function(cfg, section, option, value) {
    var section_name = Pointer_stringify(section);
    var option_str = Pointer_stringify(option);
    var value_str = Pointer_stringify(value);
    if(!(section_name in CONFIG)) {
      CONFIG[section_name] = {}
    }
    var sec = CONFIG[section_name]
    sec[option] = value
  },
  GNUNET_CONFIGURATION_create: function() {
    return 1; // opaque non-null pointer
  },
  GNUNET_CONFIGURATION_load: function(cfg, filename) {
    return 1;
  },
  GNUNET_CONFIGURATION_destroy: function(cfg) {
  }
});

// vim: set expandtab ts=2 sw=2:
