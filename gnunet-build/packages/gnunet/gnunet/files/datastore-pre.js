datastore_prerun = function() {
  addRunDependency('datastore-indexedDB');
  var request = indexedDB.open('datastore', 2);
  request.onsuccess = function(e) {
    self.dsdb = e.target.result;
    removeRunDependency('datastore-indexedDB');
  };
  request.onerror = function(e) {
    console.error('Error opening datastore database');
  };
  request.onupgradeneeded = function(e) {
    var db = e.target.result;
    if (0 == e.oldVersion) {
      var store = db.createObjectStore('datastore', {keyPath: 'uid',
                                                     autoIncrement: true});
      store.createIndex('by_key', ['key', 'uid']);
      store.createIndex('by_anon_type', ['anonymity', 'type', 'uid']);
      store.createIndex('by_replication', 'replication');
      store.createIndex('by_expiry', 'expiry');
    } else if (1 == e.oldVersion) {
      var store = request.transaction.objectStore('datastore');
      store.deleteIndex('by_key');
      store.createIndex('by_key', ['key', 'uid']);
      store.deleteIndex('by_anon_type');
      store.createIndex('by_anon_type', ['anonymity', 'type', 'uid']);
    }
  };
};
Module['preInit'].push(datastore_prerun);

// vim: set expandtab ts=2 sw=2
