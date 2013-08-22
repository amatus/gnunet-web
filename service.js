mergeInto(LibraryManager.library, {
  GNUNET_SERVICE_run: function(argc, argv, service_name, options, task,
                               task_cls) {
    Module.print('hello service run');
    Runtime.dynCall('viii', task, [task_cls, 1, 2]);
  }
});

// vim: set expandtab ts=2 sw=2:
