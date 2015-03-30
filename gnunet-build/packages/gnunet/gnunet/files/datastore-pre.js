datastore_prerun = function() {
  addRunDependency('datastore-indexedDB');
  var request = indexedDB.open('datastore', 1);
  request.onsuccess = function(e) {
    self.dsdb = e.target.result;
    removeRunDependency('datastore-indexedDB');
  };
  request.onerror = function(e) {
    console.error('Error opening datastore database');
  };
  request.onupgradeneeded = function(e) {
    var db = e.target.result;
    var store = db.createObjectStore('datastore', {keyPath: 'uid',
                                                   autoIncrement: true});
    store.createIndex('by_key', 'key');
    store.createIndex('by_anon_type', ['anonymity', 'type']);
    store.createIndex('by_replication', 'replication');
    store.createIndex('by_expiry', 'expiry');
  };
};
Module.preRun.push(datastore_prerun);

// vim: set expandtab ts=2 sw=2
