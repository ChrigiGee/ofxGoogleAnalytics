#include "ofApp.h"

//--------------------------------------------------------------
void ofApp::setup(){

	ofBackground(22);
	ofSetFrameRate(60);
	ofSetVerticalSync(false);
	currentScreen = 1;

	ga = new ofxGoogleAnalytics();

	ofxSimpleHttp::createSslContext();

	//add listener to GoogleAnalytics, to get feedback
	ofAddListener(ga->gaResponse, this, &ofApp::googleAnalyticsResponse);

	ga->setShouldReportFramerates(true); //send timing tracking info
	ga->setFramerateReportInterval(600); //every 60 sec, report app's framerate
	ga->setUserID("armadillu"); //you can set a random user string for the GA session

	ga->setup("UA-127415904-1",				//google track ID << REPLACE WITH YOURS!
			 "ofxGoogleAnalytics SimpleExample",	//app name
			 "v1",	//app version
			 "mySimpleAppID",						//ap id
			 "mySimpleAppInstallerID"				//app installer id
			 );

	//listen to ofxRemteUIClient events
	ofAddListener(RUI_GET_OF_EVENT(), this, &ofApp::remoteUIClientDidSomething);

	RUI_SETUP();
	RUI_SHARE_PARAM(sendScreenViews);
	RUI_SHARE_PARAM(sendPage);
	RUI_SHARE_PARAM(sendExceptions);
	RUI_SHARE_PARAM(sendEvents);
	RUI_SHARE_PARAM(sendFramerate);
	RUI_SHARE_PARAM(sendInterval, 10, 300); //seconds
	RUI_SHARE_PARAM(maxRequestsPerSession, 3, 500); //seconds
	RUI_SHARE_PARAM(sendToGoogleRate, 0.01, 2); //seconds
	RUI_SHARE_PARAM(randomizeUUID);

	RUI_LOAD_FROM_XML();

	ga->setEnabled(true);
	ga->setCustomUserAgent("");
	ga->setRandomizeUUID(randomizeUUID);
	ga->setShouldReportFramerates(sendFramerate);
	ga->setMaxRequestsPerSession(maxRequestsPerSession);
	ga->setSendToGoogleInterval(sendToGoogleRate);

	time = 0;
	TIME_SAMPLE_ENABLE();
}

//get feedback on our request
void ofApp::googleAnalyticsResponse(ofxGoogleAnalytics::AnalyticsResponse &r){
	if(!r.ok){
		ofLog() << "AnalyticsResponse: " << r.httpStatus << " - " << r.status;
	}
}

//--------------------------------------------------------------
void ofApp::update(){

	ga->update();
	time += ofGetLastFrameTime();

	if(time > sendInterval + timeRandomness){

		if(sendScreenViews){
			string currentScreenS = "Screen " + ofToString((int)ofRandom(10));
			ga->sendScreenView(ofToString(currentScreenS));
		}

		if(sendEvents){
			string eventCategory = "KeyboardEvent";
			string eventAction = "Event " + ofToString((int)ofRandom(20));
			string eventLabel = "Label " + ofToString((int)ofRandom(60));
			int eventValue = ofRandom(500);
			ga->sendEvent(eventCategory, eventAction, eventValue, eventLabel);
		}

		if(sendPage){
			//this fakes an http like hierarchy for pages
			string level = "level" + ofToString((int)ofRandom(22));
			string page = "page" + ofToString((int)ofRandom(10));
			string pageTitle = "My Page " + ofToString((int)ofRandom(50));
			ga->sendPageView("levels/" + level + "/" + page, pageTitle);
		}

		if(sendExceptions){
			string exc = "Error " + ofToString((int)ofRandom(100));
			ga->sendException(exc, false);
		}

		time = 0;
		timeRandomness = ofRandom( -sendInterval * 0.2, sendInterval * 0.2);
	}
}

//--------------------------------------------------------------
void ofApp::draw(){

	ofColor color;
	color.r = (currentScreen == 1 ? 60 : 0);
	color.g = (currentScreen == 2 ? 60 : 0);
	color.b = (currentScreen == 3 ? 60 : 0);
	ofBackground(color);

	ga->draw(30,40);

	ofSetColor(200);
	ofDrawBitmapString("Screen: " + ofToString(currentScreen) + "\n"
					   "press 1-3 to report different screenViews\n"
					   "press 4-6 to report Exceptions\n"
					   "press 7-9 to report Events\n"
					   "press q,a,z to report PageViews\n"
					   "press SPACEBAR to report a simple benchmark\n",
					   20, ofGetHeight() - 84);

	ofDrawBitmapString( ofToString(time, 1) + "/" + ofToString(sendInterval + timeRandomness,1), 10, 20);
}


void ofApp::exit(){
	delete ga; //deleting the GA object closes the current session
	ga = NULL;
	ofxSimpleHttp::destroySslContext();
}


//--------------------------------------------------------------
void ofApp::keyPressed(int key){

	//update current screen
	switch (key) {
		case '1': currentScreen = 1; break;
		case '2': currentScreen = 2; break;
		case '3': currentScreen = 3; break;
	}
}

void ofApp::simpleBenchmark(){
	//measure how long it takes to calc 9000000 sinf()'s
	float a = 0;
	float t = ofGetElapsedTimef();
	for(int i = 0; i < 9000000; i++){
		a += sinf(i * 0.1) ;
	}
	t = ofGetElapsedTimef() - t + (a - a); //overcome compiler optimizations
	//send that timing
	ga->sendCustomTimeMeasurement("SimpleBenchMark", "9000000xsinf()", t * 1000);
}

//--------------------------------------------------------------
void ofApp::keyReleased(int key){

	string currentScreenS = "screen"+ofToString(currentScreen);
	
	switch (key) {
		case '1': ga->sendScreenView(currentScreenS); break;
		case '2': ga->sendScreenView(currentScreenS); break;
		case '3': ga->sendScreenView(currentScreenS); break;
		case '4': ga->sendException("Exception1", false); break;
		case '5': ga->sendException("Exception2", false); break;
		case '6': ga->sendException("Exception3", true); break;
		case '7': ga->sendEvent("KeyboardEvent", "pressed7", key, "someLabel"); break;
		case '8': ga->sendEvent("KeyboardEvent", "pressed8", key, "someLabel"); break;
		case '9': ga->sendEvent("KeyboardEvent", "pressed9", key, "someLabel"); break;
		case 'q': ga->sendPageView("keyboardKeys/row1/q", "my page for Q key"); break;
		case 'a': ga->sendPageView("keyboardKeys/row2/a", "my page for A key"); break;
		case 'z': ga->sendPageView("keyboardKeys/row3/z", "my page for Z key"); break;

		case ' ':{
			simpleBenchmark();
		}break;
	}
}


//define a callback method to get notifications of client actions
void ofApp::remoteUIClientDidSomething(RemoteUIServerCallBackArg &arg){
	switch (arg.action) {
		case CLIENT_UPDATED_PARAM:
			ga->setShouldReportFramerates(sendFramerate);
			ga->setMaxRequestsPerSession(maxRequestsPerSession);
			ga->setSendToGoogleInterval(sendToGoogleRate);
			if (arg.paramName == "randomizeUUID"){
				ga->setRandomizeUUID(arg.param.boolVal);
			}
			break;
		default:
			break;
	}
}


//--------------------------------------------------------------
void ofApp::mouseMoved(int x, int y){

}

//--------------------------------------------------------------
void ofApp::mouseDragged(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mousePressed(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mouseReleased(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::windowResized(int w, int h){

}

//--------------------------------------------------------------
void ofApp::gotMessage(ofMessage msg){

}

//--------------------------------------------------------------
void ofApp::dragEvent(ofDragInfo dragInfo){ 

}