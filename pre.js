// vim: set expandtab ts=2 sw=2:
gnunet_prerun = function() {
  ENV.GNUNET_PREFIX = "/.";
}
if (typeof(Module) === "undefined") Module = { preRun: [] };
Module.preRun.push(gnunet_prerun);
