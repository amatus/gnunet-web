// configuration.js - config data handler for gnunet-web services
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
  $CONFIG: {
    'PEER': {
      'PRIVATE_KEY' : '/private_key',
    },
  },
  GNUNET_CONFIGURATION_get_value__deps: ['$CONFIG'],
  GNUNET_CONFIGURATION_get_value: function(section, option) {
    var section = Pointer_stringify(section);
    var option = Pointer_stringify(option);
    Module.print('GNUNET_CONFIGURATION_get_value(' + section + ',' + option + ')');
    if (!(section in CONFIG)) {
      Module.print(section + ' not in CONFIG');
      return undefined;
    }
    section = CONFIG[section]
    if (!(option in section)) {
      Module.print(option + ' not in ' + section);
      return undefined;
    }
    Module.print('found: ' + section[option]);
    return section[option];
  },
  GNUNET_CONFIGURATION_get_value_string__deps:
    ['GNUNET_CONFIGURATION_get_value'],
  GNUNET_CONFIGURATION_get_value_string: function(cfg, section, option, value) {
    var tmp = _GNUNET_CONFIGURATION_get_value(section, option);
    if (undefined === tmp)
      return -1;
    var tmp = ccall('GNUNET_xstrdup_', 'number', ['string', 'string', 'number'],
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
    ['GNUNET_CONFIGURATION_get_value'],
  GNUNET_CONFIGURATION_get_value_time: function(cfg, section, option, time) {
    var tmp = _GNUNET_CONFIGURATION_get_value(section, option);
    if (undefined === tmp)
      return -1;
    return ccall('GNUNET_STRINGS_fancy_time_to_relative', 'number',
        ['string', 'number'], [tmp, time]);
  },
});

// vim: set expandtab ts=2 sw=2:
