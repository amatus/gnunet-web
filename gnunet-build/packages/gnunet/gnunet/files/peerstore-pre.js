addRunDependency('peerstore-indexedDB');
var request = indexedDB.open('peerstore', 1);
request.onsuccess = function(e) {
  self.psdb = e.target.result;
  removeRunDependency('peerstore-indexedDB');
};
request.onerror = function(e) {
  Module.print('Error opening peerstore database');
};
request.onupgradeneeded = function(e) {
  var db = e.target.result;
  var store = db.createObjectStore('peerstore', {autoIncrement: true});
  store.createIndex('by_subsystem', 'sub_system');
  store.createIndex('by_pid', ['sub_system', 'peer_id']);
  store.createIndex('by_key', ['sub_system', 'key']);
  store.createIndex('by_all', ['sub_system', 'peer_id', 'key']);
  store.createIndex('by_expiry', 'expiry');
};
