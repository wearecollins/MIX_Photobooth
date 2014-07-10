AIGA MIX : Kinect GIF Photobooth
=========
* By Brett Renfer
* [http://aigamix.tumblr.com](http://aigamix.tumblr.com)

Setting up
=========
* Visual App (BeachVibes)
	* openFrameworks requirements: 
		* [openFrameworks 0.8.3](http://www.openframeworks.cc/download/)
		* [ofxRCUtils](https://github.com/robotconscience/ofxRCUtils)
		* [ofxSlitscan (my fork, for now)](https://github.com/robotconscience/ofxSlitScan)
		* [ofxGifDecoder (my fork, for now)](https://github.com/robotconscience/ofxGifDecoder)
	* Clone this app into openFrameworks/myApps
	* Hardware
		* Kinect v1!
		* Only tested on OS X (sorry ;( )
	* Content
		* A movie called "beach.mov" in bin/data
			* We made a supercut of Baywatch intros, beach music vids, and stock beach video
* Tumblr Uploader
	* This is a very hastily compiled hodgepodge of examples. Use at your own peril!
		* install [node](http://nodejs.org/)
		* cd this_repo/scripts
		* npm install tumblr
		* npm install express
		* npm install body-parser
		* npm install cookie-parser
		* npm install oauth
		* npm install watchr
		* I hope that's it... sorry, a lot going on here.