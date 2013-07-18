gnunet_prerun = function() {
	ENV.GNUNET_PREFIX = "/.";
	_pipe = function(filedes) {
		var fd0 = FS.createFileHandle({isDevice: true});
		var fd1 = FS.createFileHandle({isDevice: true});
		setValue(filedes, fd0, 'i32');
		setValue(filedes + 4, fd1, 'i32');
		return 0;
	}
}
if (typeof(Module) === "undefined") Module = { preRun: [] };
Module.preRun.push(gnunet_prerun);
