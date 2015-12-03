#pragma once
#ifndef PHUNE_REST_BASE
#define PHUNE_REST_BASE

#include <IwHTTP.h>
#include <s3e.h>
#include <IwDebug.h>
#include <string>
#include <queue>
#include <deque>
#include <ctime>
#include <stdlib.h>
#include "pthread.h"
#include "s3eWebView.h"
#include <IwUriEscape.h>


#include "../model/phune/phune_preference.h"
#include "../model/phune/phune_preference_base64.h"
#include "../model/base64.h"
#include "../model/phune/phune_user.h"
#include "../model/json_object.h"
#include "../model/json_list_object.h"
#include "../model/phune/phune_player.h"
#include "../model/jamp/jamp_score.h"
#include "../model/phune/phune_match.h"
#include "../model/phune/phune_game_triggered_event.h"

//timeout 30 seconds
#define REQUEST_TIMEOUT 30000

//define base url
#define URL_SCHEMA "http"
#define URL_HOST "api.jamp.io"
#define URL_PORT "80"
#define URL_CONTEXT "/phune/r"

#define LOGIN_RESOURCE "/me/login/FACEBOOK"
#define LOGOUT_RESOURCE "/me/logout"
#define LOGIN_REDIRECT "/socialgateway/success"

#define LOGOUT_FACEBOOK_PAGE "http://jamp.io/platform/redirects/facebook-logout.html"
#define LOGOUT_FACEBOOK_REDIRECT "http://api.jamp.io/phune/r/jamp/utils/time"



/*
The request status
*/
enum RequestStatus
{
	READY,
	ONGOING_REQUEST,
	REQUEST_ERROR,
	INVALID_RESPONSE_OBJECT,
    NO_CONTENT
};

/*
Type of response. Used to parse object.
*/
enum ResponseObjectType
{
	PHUNE_TIMESTAMP,
	PHUNE_USER_OBJECT,
	PHUNE_MATCH_OBJECT,
	PHUNE_PREFERENCE_OBJECT,
	PHUNE_PREFERENCE_JSON,
	PHUNE_PREFERENCE_JSON_LIST,
	PHUNE_STRING,
	PHUNE_REDIRECT_LOGIN,
    PHUNE_LOGOUT,
	NONE
};

enum WebViewRequestStatus
{
	LOADING,
	HIDE_VIEW,
	STOP_VIEW,
	FINISHED,
};

/*
Class to represent a request error.
*/
class RequestError
{
public:
	RequestError(RequestStatus type, uint32 httpCode);
	~RequestError();
	RequestStatus type;
	uint32 httpCode;
};

struct pendingRequest {
	std::string resource;
	CIwHTTP::SendType sendType;
	ResponseObjectType responseObjectType;
	std::string body;
	void *userData;
	s3eCallback onResult;
	s3eCallback onError;
};

static pthread_mutex_t mutex;

static std::deque<pendingRequest> _pendingRequests;

/*
Class to represent a request
*/
class RequestData
{
public:
	RequestData(){};
	RequestData(const char *URI, CIwHTTP *http_object, CIwHTTP::SendType sendType, s3eCallback onResult, ResponseObjectType responseObjectType, const char *body, void *userData);
	~RequestData();
	static int32 GotHeaders(void*, void*);
	static int32 GotData(void*, void*);
public:
	CIwHTTP *http_object;
	char* result;
	uint32 len;
	s3eCallback onResult;
	RequestStatus requestStatus;
	ResponseObjectType responseObjectType;
	void* base;
	void *userData;
	static const char* getUri(const char *resource);

};

class PhuneRestBase
{
public:
	PhuneRestBase();
	~PhuneRestBase();

	template<class T>
	int32 StoreGameDataJsonList(const char *gameId, const char *key, JsonListObject<T> jsonListObject, s3eCallback onResult, s3eCallback onError, void *userData){
		char resource[200];
		memset(resource, 0, sizeof(resource));
		sprintf(resource, "/jamp/me/preferences/%s?gameId=%s", key, gameId);

        pendingRequest pr;
        pr.body = jsonListObject.Serialize();
        pr.onError = onError;
        pr.onResult = onResult;
        pr.resource = std::string(resource);
        pr.responseObjectType = NONE;
        pr.sendType = CIwHTTP::PUT;
        pr.userData = userData;

        pthread_mutex_lock(&mutex);
        _pendingRequests.push_back(pr);
        if (_pendingRequests.size() > 1)
        {
            pthread_mutex_unlock(&mutex);
            return 0;
        }
        pthread_mutex_unlock(&mutex);
		
		_onResult = onResult;
		_onError = onError;

		new RequestData(resource, http_object, CIwHTTP::PUT, GotResult, NONE, jsonListObject.Serialize().c_str(), userData);

		return 0;
	}

	int32 GetGameDataJsonList(const char *gameId, const char *key, s3eCallback onResult, s3eCallback onError, void *userData);

	int64 getCurrentTime();
protected:

	int32 _GetMe(s3eCallback onResult, s3eCallback onError, void *userData);

	int32 StoreGameDataBase64(const char *gameId, const char *key, unsigned char const* bytes, s3eCallback onResult, s3eCallback onError, void *userData);

	int32 StoreGameDataJson(const char *gameId, const char *key, const char* jsonObject, s3eCallback onResult, s3eCallback onError, void *userData, bool append = false);
    
    int32 StoreGameDataJsonBatch(const char *gameId, const char *key, const char* jsonObject, s3eCallback onResult, s3eCallback onError, void *userData, bool append);

	int32 GetGameDataBase64(const char *gameId, const char *key, s3eCallback onResult, s3eCallback onError, void *userData);

	int32 GetGameDataJson(const char *gameId, const char *key, s3eCallback onResult, s3eCallback onError, void *userData);

	int32 DeleteGameData(const char *gameId, const char *key, s3eCallback onResult, s3eCallback onError, void *userData);

	int32 _Init(s3eCallback onResult, s3eCallback onError, void *userData);

	int32 _Login(s3eWebView* g_WebView, s3eCallback onResult, s3eCallback onError, void *userData);

	int32 _Logout(s3eCallback onResult, s3eCallback onError, void *userData);

	int32 _StartMatch(const char *gameId, s3eCallback onResult, s3eCallback onError, void *userData);

	int32 _EndMatch(int64 matchId, PhunePlayer player, s3eCallback onResult, s3eCallback onError, void *userData);
    
    int32 _StoreMatchEvents(JsonListObject<GameTriggerdEvent> events, int64 matchId, s3eCallback onResult, s3eCallback onError, void *userData);
public:
	

	/*
	Callback in case of success.
	*/
	static s3eCallback _onResult;
	
	/*
	Callback in case of error.
	*/
	static s3eCallback _onError;

	int32 removeOngoingRequest();
    
    

protected:
	/*
	The http object to communicato to the server
	*/
	CIwHTTP *http_object;
	/*
	Used to get the result
	*/
	static int32 GotResult(void*, void*);

	static int32 _onHideWebView(s3eWebView *instance, void *systemData, void *userData);
    
    static int32 _onLogoutWebView(s3eWebView *instance, void *systemData, void *userData);

private:

	

};


static void* _objectToappend;
inline static void*& objectToAppend(){
	return _objectToappend;
}


static int64 lib_time;
static int64 server_time;

static std::string cookieSession;


//define the template functions
//#include "phune_rest_base.tpp"

#endif // !PHUNE_REST_BASE

