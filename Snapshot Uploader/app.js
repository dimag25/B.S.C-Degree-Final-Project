var chokidar = require('chokidar');
var keypress = require('keypress');
var request = require('request');
var fs = require('fs');
var path = require('path');

var filePath = __dirname;
var formData = {};

// load form data to upload to server
chokidar.watch(filePath, { awaitWriteFinish: true, atomic: true }).on('change', function (filename) {

  // Handle txt file
  if (path.extname(filename) == '.txt') {
    var content = fs.createReadStream(filename);
    var data = '';

    content.on('error', function (err) {
      console.log(err);
    });

    content.on('data', function (chunk) {
      data += chunk;
    });

    content.on('end', function () {
      data = JSON.parse(data);
      formData.bpm = data.bpm;
      formData.motion = data.motion;

      console.log(path.basename(filename) + ' changed, added to upload data');
    });
  }

  // Handle jpg file
  if (path.extname(filename) == '.jpg') {
    formData.file = fs.createReadStream(filename);

    console.log(path.basename(filename) + ' changed, added to upload data');

    formData.file.on('error', function (err) {
      console.log(err);
    });
  }

});

// Keypress to exit uploader
keypress(process.stdin);
process.stdin.on('keypress', function (ch, key) {
  if (ch === 'c') {
    console.log('exiting..');
    process.exit(1);
  }
});

process.stdin.setRawMode(true);
process.stdin.resume();

console.log('Watching files at ' + filePath + ' for changes.\npress C to terminate');

setInterval(function () {
  // Upload data to server if form is ready
  if (formData.bpm && formData.motion && formData.file) {
    request.post({ url: 'http://eulerian-project.herokuapp.com/upload', formData: formData },
    function (err, httpResponse, body) {
      if (err)
        return console.error('upload failed: ' + err);
      else
        console.log('Snapshot sent! Server response: ' + body);
    });

    // reset form data
    formData = {};
  }
}, 500);

// TESTER
// setInterval(function () {
//   fs.writeFile(filePath + '/test.txt', '{ "bpm": 50, "motion": "false" }', function (err) {
//     if (err) throw err;
//     console.log('It\'s saved!');
//   });
// }, 5000);
