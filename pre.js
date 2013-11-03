// vim: set expandtab ts=2 sw=2:
gnunet_prerun = function() {
  ENV.GNUNET_PREFIX = "/.";
  Module['print'] = function(x) { self.postMessage(x); };
}
if (typeof(Module) === "undefined") Module = { preRun: [] };
Module.preRun.push(gnunet_prerun);
