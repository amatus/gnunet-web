mergeInto(LibraryManager.library, {
  GNUNET_SERVICE_run: function(argc, argv, service_name, options, task,
                               task_cls) {
    Module.print('hello service run');
    var server = 1; // opaque non-null pointer
    var cfg = 2; // same
    Runtime.dynCall('viii', task, [task_cls, server, cfg]);
  }
});

// vim: set expandtab ts=2 sw=2:
