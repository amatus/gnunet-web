mergeInto(LibraryManager.library, {
  GNUNET_SCHEDULER_add_delayed: function(delay, priority, task, task_cls) {
    Module.print('scheduler add delayed');
  }
});

// vim: set expandtab ts=2 sw=2:
