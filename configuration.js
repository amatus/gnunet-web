mergeInto(LibraryManager.library, {
  $CONFIG: {
    'peerinfo': {
      'HOSTS': '/hosts',
    },
  },
  GNUNET_CONFIGURATION_get_value__deps: ['$CONFIG'],
  GNUNET_CONFIGURATION_get_value: function(section, option) {
    var section = Pointer_stringify(section);
    var option = Pointer_stringify(option);
    Module.print('GNUNET_CONFIGURATION_get_value_yesno(' + section + ',' + option + ')');
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
  }
});

// vim: set expandtab ts=2 sw=2:
