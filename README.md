# B.S.C-Degree-Final-Project
Real-time infant physiological measure monitoring system using video footage




1. Before running the system, create a folder named BabyMonitor, under /: C.
Into this folder the information transmitted to the server will be written.


2. The contents of the Snapshot Uploader.zip file must then be moved into the folder we created in C: / BabyMonitor. Now navigate to the folder via cmd, and run the node install command. (Node.js must be installed before running the command and all neccesray node modules like: chokidar,keypress,request).
After running the install, we will run the snapshot uploader by the node app.js. command.


3. To see the changes everywhere, we will install the app on the mobile phone (Android type). The app installation file is android-debug.apk. Login details are:
Username: Admin
Password: Admin


4. The infrastructure is now ready to run the system. Code from the system is in the file - BabyMonitorFinalVersion.rar and can be run through Visual Studio.
   To run baby monitor C++ application make sure to you have opencv 2.4.10 version in your local pc : C:\opencv2410\opencv , and correctly configured in environment variables      path in the windows.

** When there is a change / upload of a new Snapshot to the server, the application will be updated and will display the latest information automatically. This is done to conserve resources and eliminate writing files on the phone due to issues that occurred during development.





Have a nice run!



