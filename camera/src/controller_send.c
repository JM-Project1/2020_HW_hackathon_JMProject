#include <curl/curl.h>
#include <glib.h>
#include "log.h"

#define REQ_CON_TIMEOUT 5L
#define REQ_TIMEOUT 7L

#define API_HOST_URL "<API_HOST_URL_HERE>"
#define SEND_URL "/send"

static size_t _response_callback(char *ptr, size_t size, size_t nmemb, void *userdata)
{
	size_t res_size = 0;

	res_size = size*nmemb;

	if (res_size > 0)
		_I("CURL response : %s", ptr);
	/* What should we do here, if response body has negative message? */

	return res_size;
}

int controller_send_message()
{
	int ret = 0;
	CURL *curl = NULL;
	CURLcode response = CURLE_OK;

	curl = curl_easy_init();

	if (!curl) {
		_E("fail to init curl");
		return -1;
	}

	char* url = g_strdup_printf("%s%s",
		API_HOST_URL,
		SEND_URL);

	_D("Send Url: [%s]", url);

	curl_easy_setopt(curl, CURLOPT_URL, url);
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, _response_callback);
	curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, REQ_CON_TIMEOUT);
	curl_easy_setopt(curl, CURLOPT_TIMEOUT, REQ_TIMEOUT);

	response = curl_easy_perform(curl);

	if (response != CURLE_OK) {
		_E("curl_easy_perform() failed: %s",
			curl_easy_strerror(response));
		/* What should we do here, if response is kind of errors? */
		ret = -1;
	}

	g_free(url_with_msg);
	curl_easy_cleanup(curl);

	return ret;
}
