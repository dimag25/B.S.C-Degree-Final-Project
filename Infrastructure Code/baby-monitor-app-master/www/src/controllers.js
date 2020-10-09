angular.module('starter.controllers', [])

.controller('LoginCtrl', function ($scope, LoginService, $ionicPopup, $state, invervalService) {
  $scope.data = {};

  $scope.login = function () {
    LoginService.loginUser($scope.data.username, $scope.data.password).success(function (data) {
      $state.go('home');
    }).error(function (data) {
      var alertPopup = $ionicPopup.alert({
        title: 'Login failed!',
        template: 'Invalid credentials!',
      });
    });
  };
})

.controller('HomeCtrl', function ($scope, ApiService, $ionicPlatform, invervalService, fileService, $state) {

  $ionicPlatform.ready(function () {
    $scope.spinner = false;
    $scope.debugging = 'waiting for interval';
    var count = 0;

    // for testing
    $scope.bpm = 'init';
    $scope.motion = 'init';
    $scope.image = 'img/latest.jpg';

    //clear previous cache
    fileService.clearCache();

    // Start img inverval updater
    invervalService.startClock(function () {

      //TODO total of 5 cache images
      count++;
      if (count > 25) count = 1;

      $scope.debugging = 'intervaling..';
      $scope.spinner = true;

      ApiService.getData()
      .then(function (result) {
        var data = result;
        ApiService.getLatest(count).then(function (fileUrl) {
          data.fileUrl = fileUrl;
          fileService.deleteFile(count - 1);
          console.log('applying latest data ' + JSON.stringify(data));
          $scope.debugging = 'applied latest data';
          $scope.bpm = data.bpm;
          $scope.motion = data.motion;
          $scope.image = data.fileUrl;
          $scope.spinner = false;
        });

      }, function (reason) {

        $scope.spinner = false;
        $scope.debugging = reason.toString();
        console.log('rejected: ' + reason);
      });
    });

    $scope.logout = function () {
      console.log('stopping invervalService');
      invervalService.stopClock();
      $state.go('login');
    };
  });
});
