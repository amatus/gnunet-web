mergeInto(LibraryManager.library, {
  GNUNET_SCHEDULER_add_delayed: function(delay, priority, task, task_cls) {
    Module.print('scheduler add delayed');
  },
  GNUNET_SCHEDULER_add_with_priority: function(priority, task, task_cls) {
    Module.print('scheduler add with prio');
  }
});

// vim: set expandtab ts=2 sw=2:
