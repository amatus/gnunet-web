// vim: set expandtab ts=2 sw=2:
var WorkerMessageQueue = [];
flush_worker_message_queue = function(f) {
  WorkerMessageQueue.forEach(f);
  WorkerMessageQueue = [];
}
gnunet_prerun = function() {
  ENV.GNUNET_PREFIX = "/.";
  if (ENVIRONMENT_IS_WORKER) {
    Module["print"] = function(x) {
      WorkerMessageQueue.push(x);
    };
  }
}
if (typeof(Module) === "undefined") Module = { preRun: [] };
Module.preRun.push(gnunet_prerun);
