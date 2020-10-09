angular.module('starter.services', [])

.service('invervalService', function ($interval) {
  var clock = null;
  var service = {
    startClock: function (fn) {
      if (clock === null) {
        clock = $interval(fn, 8000);
      }
    },

    stopClock: function () {
      if (clock !== null) {
        $interval.cancel(clock);
        clock = null;
      }
    },
  };

  return service;
})

.service('LoginService', function ($q) {
  return {
    loginUser: function (name, pass) {
      var deferred = $q.defer();
      var promise = deferred.promise;

      // if (name == 'Admin' && pass == 'Admin') {
      deferred.resolve('Valid credentials.');

      // } else {
      // deferred.reject('Invalid credentials.');
      // }

      promise.success = function (fn) {
        promise.then(fn);
        return promise;
      };

      promise.error = function (fn) {
        promise.then(null, fn);
        return promise;
      };

      return promise;
    },
  };
})

.service('ApiService', function ($q, $timeout, $cordovaFileTransfer, $http) {
  return {
    getLatest: function (count) {
      var deferred = $q.defer();
      var promise = deferred.promise;

      // var ft = new FileTransfer();
      var uri = encodeURI('http://eulerian-project.herokuapp.com/latest');
      var target = cordova.file.dataDirectory + count + '.jpg';

      var options = new FileUploadOptions();
      options.chunkedMode = false;
      options.headers = {
        Connection: 'close',
      };

      $cordovaFileTransfer.download(uri, target, options, true)
      .then(function (result) {
          result = result.toURL();
          console.log('download complete');
          deferred.resolve(result);
        },

        function (err) {
          console.log('failed ' + err.code + ' ' + err.target + ' URL:' + uri);
          deferred.reject('Download Error');
        },

        function (progress) {});

      return promise;
    },

    getData: function () {
      var deferred = $q.defer();
      var promise = deferred.promise;

      $http.get('http://eulerian-project.herokuapp.com/data')
      .then(function (response) {
        var data = response.data;
        if (response.data === 'no change') {
          deferred.reject('no change in server data');
        } else {
          deferred.resolve({ bpm: data.bpm, motion: data.motion });
        }
      });

      return promise;
    },
  };
})

.service('fileService', function ($q, $cordovaFile) {
  return {
    deleteFile: function (fileNum) {
      var deferred = $q.defer();
      var promise = deferred.promise;

      if (fileNum === 0) fileNum = 25;

      $cordovaFile.removeFile(cordova.file.dataDirectory, fileNum + '.jpg')
        .then(function (result) {
          console.log('file deleted successfully ' + JSON.stringify(result));
          deferred.resolve();
        }, function (err) {

          console.log('Error deleting file ' + JSON.stringify(err));
          deferred.reject();
        });

      return promise;
    },

    clearCache: function () {
      for (var i = 0; i < 25; i++)
        this.deleteFile(i);
    },
  };
});
