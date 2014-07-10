
var auth = require('./auth').tAuth;

var express = require('express'),
	fs = require('fs')
	bodyParser = require('body-parser'),
	cookieParser = require('cookie-parser'),
	session = require('express-session'),
	http = require('http'),
	tumblr = require('tumblr'),
	OAuth = require('oauth').OAuth,
	querystring = require('querystring'),
	watchr = require('watchr');

// Setup the Express.js server
var app = express();
var server = http.createServer();
app.use(bodyParser.urlencoded({ extended: true }));
app.use(cookieParser());
app.use(session({
	secret: "skjghskdjfhbqigohqdiouk"
}));

// Home Page
app.get('/', function(req, res){
	if(!req.session.oauth_access_token) {
		res.redirect("/login");
	}
});

// Request an OAuth Request Token, and redirects the user to authorize it
app.get('/login', function(req, res) {
	
	// setup tumblr oauth
	var oa = new OAuth(
		'http://www.tumblr.com/oauth/request_token',
		'http://www.tumblr.com/oauth/access_token',
		auth.consumer_key,
	    auth.consumer_secret,
		'1.0A',
		"http://localhost:3000/callback"+( req.param('action') && req.param('action') != "" ? "?action="+querystring.escape(req.param('action')) : "" ),
		'HMAC-SHA1'
	);

	oa.getOAuthRequestToken(function(error, oauth_token, oauth_token_secret, results){
	  if(error) {
			console.log('error');
	 		console.log(error);
		}
	  else { 
	  	console.log("wtf");
			// store the tokens in the session
			req.session.oa = oa;
			req.session.oauth_token = oauth_token;
			req.session.oauth_token_secret = oauth_token_secret;
		
			// redirect the user to authorize the token
			res.redirect("http://www.tumblr.com/oauth/authorize?oauth_token="+oauth_token);
	  }
	})

});

// Callback for the authorization page
app.get('/callback', function(req, res) {
		
	// get the OAuth access token with the 'oauth_verifier' that we received
	var oa = new OAuth(req.session.oa._requestUrl,
	                  req.session.oa._accessUrl,
	                  req.session.oa._consumerKey,
	                  req.session.oa._consumerSecret,
	                  req.session.oa._version,
	                  req.session.oa._authorize_callback,
	                  req.session.oa._signatureMethod);
	
    console.log(oa);

	oa.getOAuthAccessToken(
		req.session.oauth_token, 
		req.session.oauth_token_secret, 
		req.param('oauth_verifier'), 
		function(error, oauth_access_token, oauth_access_token_secret, results2) {
			
			if(error) {
				console.log('error');
				console.log(error);
	 		}
	 		else {
				// store the access token in the session
				req.session.oauth_access_token = oauth_access_token;
				req.session.oauth_access_token_secret = oauth_access_token_secret;

				var client = tumblr.createClient({
				  consumer_key: auth.consumer_key,
				  consumer_secret: auth.consumer_secret,
				  token: req.session.oauth_access_token,
				  token_secret: req.session.oauth_access_token_secret
				});

				// listen to folder
				watchr.watch({
				    paths: ['../images'],
				    listeners: {
				        log: function(logLevel){
				            //console.log('a log message occured:', arguments);
				        },
				        error: function(err){
				            console.log('an error occured:', err);
				        },
				        watching: function(err,watcherInstance,isWatching){
				            if (err) {
				                console.log("watching the path " + watcherInstance.path + " failed with error", err);
				            } else {
				                console.log("watching the path " + watcherInstance.path + " completed");
				            }
				        },
				        change: function(changeType,filePath,fileCurrentStat,filePreviousStat){
				            //console.log('a change event occured:',arguments);
				            if ( changeType == 'create'){
								// var photo = fs.readFileSync(filePath);
					            client.photo(auth.blog_name, {data:filePath}, function(c){
					            	console.log(c);
					            });
					        }
				        }
				    }
				});
	 		}

	});
	
});

app.listen(3000);
console.log("listening on http://localhost:3000");
