var express = require('express');
var fs = require('fs');
var inspect = require('util').inspect;
var Busboy = require('busboy');
var path = require('path');
var os = require('os');
var router = express.Router();

// data json obj
var data = {};
var dataChanged = false;

router.get('/', function (req, res) {
  console.log('GET');

  res.render('home.ejs');
});

// Get latest snapshot from the server
router.get('/latest', function (req, res) {
  var files = fs.readdirSync(__dirname + '/images');

  if (files.length > 1) {
    console.log('error with number of files in /images');
    throw 'error';
  }

  fs.readFile(__dirname + '/images/' + files[0], function (err, data) {
    if (err) throw err; // Fail if the file can't be read.
    res.writeHead(200, {
      'Content-Type': 'image/jpeg',
    });
    res.end(data); // Send the file data to the browser.
  });
});

// Get latest data from the server
router.get('/data', function (req, res) {
  if (dataChanged) {
    setTimeout(function () {
      dataChanged = false;
    }, 4000);

    res.send(data);
  } else
    res.send('no change');
});

// Get data from the form
router.post('/upload', function (req, res) {
  var error = false;

  try {

    // parse req
    var busboy = new Busboy({
      headers: req.headers,
    });
    var uploadedFile = false;

    // handle file upload
    busboy.on('file', function (fieldname, file, filename, encoding, mimetype) {
      console.log('File [' + fieldname + ']: filename: ' + filename + ', encoding: ' + encoding + ', mimetype: ' + mimetype);

      // Remove old file
      var files = fs.readdirSync(__dirname + '/images');

      if (files.length > 1) {
        console.log('error with number of files in /images');
        throw 'error';
      }

      if (files.length == 1)
        fs.unlinkSync(__dirname + '/images/' + files[0]);

      console.log('removed old files');

      // Save file to /images path
      var saveTo = path.join(__dirname + '/images', path.basename(filename));

      var fstream = fs.createWriteStream(saveTo);
      fstream.on('error', function (err) {
        error = true;
        console.log('ERROR Stream:' + err);
        file.unpipe();
        fstream.end();
      });

      file.pipe(fstream);

      file.on('end', function () {
        console.log('File [' + fieldname + '] Finished');
        uploadedFile = true;
      });
    });

    // handle field upload
    busboy.on('field', function (fieldname, val, fieldnameTruncated, valTruncated, encoding, mimetype) {
      console.log('Got Field [' + fieldname + ']: value: ' + inspect(val));
      data[fieldname] = inspect(val);
    });

    // Finished parsing the form
    busboy.on('finish', function () {
      console.log('Done parsing form!');

      if (!error) {
        if (!uploadedFile)
          res.send('no file provided on request!');
        else if (!data.bpm || !data.motion)
          res.send('no data fields provided on request!');
        else {
          dataChanged = true;
          res.send('done uploading files');
        }
      }
    });

    busboy.on('error', function (data) {
      error = true;
      console.log('inside busboy error handler: ' + data);
      res.send('got an error inside busboy error handler');
    });

    req.pipe(busboy);

  } catch (e) {
    console.log('caught error ' + e);
  }
});

module.exports = router;
