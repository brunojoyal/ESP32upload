#include "PushoverESP32.h"

const char *PUSHOVER_ROOT_CA = "-----BEGIN CERTIFICATE-----\n"
							   "MIIDrzCCApegAwIBAgIQCDvgVpBCRrGhdWrJWZHHSjANBgkqhkiG9w0BAQUFADBh\n"
							   "MQswCQYDVQQGEwJVUzEVMBMGA1UEChMMRGlnaUNlcnQgSW5jMRkwFwYDVQQLExB3\n"
							   "d3cuZGlnaWNlcnQuY29tMSAwHgYDVQQDExdEaWdpQ2VydCBHbG9iYWwgUm9vdCBD\n"
							   "QTAeFw0wNjExMTAwMDAwMDBaFw0zMTExMTAwMDAwMDBaMGExCzAJBgNVBAYTAlVT\n"
							   "MRUwEwYDVQQKEwxEaWdpQ2VydCBJbmMxGTAXBgNVBAsTEHd3dy5kaWdpY2VydC5j\n"
							   "b20xIDAeBgNVBAMTF0RpZ2lDZXJ0IEdsb2JhbCBSb290IENBMIIBIjANBgkqhkiG\n"
							   "9w0BAQEFAAOCAQ8AMIIBCgKCAQEA4jvhEXLeqKTTo1eqUKKPC3eQyaKl7hLOllsB\n"
							   "CSDMAZOnTjC3U/dDxGkAV53ijSLdhwZAAIEJzs4bg7/fzTtxRuLWZscFs3YnFo97\n"
							   "nh6Vfe63SKMI2tavegw5BmV/Sl0fvBf4q77uKNd0f3p4mVmFaG5cIzJLv07A6Fpt\n"
							   "43C/dxC//AH2hdmoRBBYMql1GNXRor5H4idq9Joz+EkIYIvUX7Q6hL+hqkpMfT7P\n"
							   "T19sdl6gSzeRntwi5m3OFBqOasv+zbMUZBfHWymeMr/y7vrTC0LUq7dBMtoM1O/4\n"
							   "gdW7jVg/tRvoSSiicNoxBN33shbyTApOB6jtSj1etX+jkMOvJwIDAQABo2MwYTAO\n"
							   "BgNVHQ8BAf8EBAMCAYYwDwYDVR0TAQH/BAUwAwEB/zAdBgNVHQ4EFgQUA95QNVbR\n"
							   "TLtm8KPiGxvDl7I90VUwHwYDVR0jBBgwFoAUA95QNVbRTLtm8KPiGxvDl7I90VUw\n"
							   "DQYJKoZIhvcNAQEFBQADggEBAMucN6pIExIK+t1EnE9SsPTfrgT1eXkIoyQY/Esr\n"
							   "hMAtudXH/vTBH1jLuG2cenTnmCmrEbXjcKChzUyImZOMkXDiqw8cvpOp/2PV5Adg\n"
							   "06O/nVsJ8dWO41P0jmP6P6fbtGbfYmbW0W5BjfIttep3Sp+dWOIrWcBAI+0tKIJF\n"
							   "PnlUkiaY4IBIqDfv8NZ5YBberOgOzW6sRBc4L0na4UU+Krk2U886UAb3LujEV0ls\n"
							   "YSEY1QSteDwsOoBrp+uvFRTp2InBuThs4pFsiv9kuXclVzDAGySj4dzp30d8tbQk\n"
							   "CAUw7C29C79Fv1C5qfPrmAESrciIxpg0X40KPMbp1ZWVbd4=\n"
							   "-----END CERTIFICATE-----\n";

Pushover::Pushover(const char *token, const char *user) : _token(token), _user(user)
{
	;
}

Pushover::Pushover(const char *token, const char *user, FS * tempfileFS) : _token(token), _user(user), _tempfileFS(tempfileFS)
{
	;
}

Pushover::Pushover(FS * altFS){
	_tempfileFS = altFS;
}

Pushover::Pushover()
{
	;
}

int Pushover::send(PushoverMessage newMessage)
{

	HTTPClient myClient;
	int responseCode=0;
	myClient.begin("https://api.pushover.net/1/messages.json", PUSHOVER_ROOT_CA);
	
	if (!newMessage.attachment)
	{ //No attachment, so just a regular HTTPS POST request.
		myClient.addHeader("Content-Type", "application/json");
		StaticJsonDocument<512> doc;
		doc["token"] = _token;
		doc["user"] = _user;
		doc["message"] = newMessage.message;
		doc["title"] = newMessage.title;
		doc["url"] = newMessage.url;
		doc["url_title"] = newMessage.url_title;
		doc["html"] = newMessage.html;
		doc["priority"] = newMessage.priority;
		doc["sound"] = newMessage.sound;
		doc["timestamp"] = newMessage.timestamp;

		char output[512];
		serializeJson(doc, output);
		responseCode = myClient.POST(output);
	}
	else //attachment, so we enter multipart/form-data hell...
	{
		File tempfile;
		if (_tempfileFS->exists("/tempfile.temp"))
		{
			_tempfileFS->remove("/tempfile.temp");
		}
		tempfile = _tempfileFS->open("/tempfile.temp", FILE_WRITE);
		if (tempfile)
		{
			tempfile.print("----abcdefg\r\n");
			tempfile.print("Content-Disposition: form-data; name=\"user\"\r\n");
			tempfile.print("\r\n");
			tempfile.printf("%s\r\n", _user);
			tempfile.print("----abcdefg\r\n");
			tempfile.print("Content-Disposition: form-data; name=\"token\"\r\n");
			tempfile.print("\r\n");
			tempfile.printf("%s\r\n", _token);
			tempfile.print("----abcdefg\r\n");
			tempfile.print("Content-Disposition: form-data; name=\"message\"\r\n");
			tempfile.print("\r\n");
			tempfile.printf("%s\r\n", newMessage.message);
			tempfile.print("----abcdefg\r\n");
			tempfile.print("Content-Disposition: form-data; name=\"url\"\r\n");
			tempfile.print("\r\n");
			tempfile.printf("%s\r\n", newMessage.url);
			tempfile.print("----abcdefg\r\n");
			tempfile.print("Content-Disposition: form-data; name=\"url_title\"\r\n");
			tempfile.print("\r\n");
			tempfile.printf("%s\r\n", newMessage.url_title);
			tempfile.print("----abcdefg\r\n");
			tempfile.print("Content-Disposition: form-data; name=\"sound\"\r\n");
			tempfile.print("\r\n");
			tempfile.printf("%s\r\n", newMessage.sound);
			tempfile.print("----abcdefg\r\n");
			tempfile.print("Content-Disposition: form-data; name=\"timestamp\"\r\n");
			tempfile.print("\r\n");
			tempfile.printf("%u\r\n", newMessage.timestamp);
			tempfile.print("----abcdefg\r\n");
			tempfile.print("Content-Disposition: form-data; name=\"html\"\r\n");
			tempfile.print("\r\n");
			tempfile.printf("%u\r\n", newMessage.html);
			tempfile.print("----abcdefg\r\n");
			tempfile.print("Content-Disposition: form-data; name=\"title\"\r\n");
			tempfile.print("\r\n");
			tempfile.printf("%s\r\n", newMessage.title);
			tempfile.print("----abcdefg\r\n");
			tempfile.print("Content-Disposition: form-data; name=\"priority\"\r\n");
			tempfile.print("\r\n");
			tempfile.printf("%u\r\n", newMessage.priority);
			tempfile.print("----abcdefg\r\n");
			tempfile.printf("Content-Disposition: form-data; name=\"attachment\"; filename=\"test.jpg\"\r\n");
			tempfile.print("Content-Type: image/jpeg\r\n");
			tempfile.print("\r\n");
			uint8_t buf[256];
			while (newMessage.attachment->available())
			{
				int read = newMessage.attachment->read(buf,256);
				tempfile.write(buf, read);
				vTaskDelay(1);
			}
			newMessage.attachment->close();
			tempfile.print("----abcdefg--\r\n");
			tempfile.flush();
			tempfile.close();
			myClient.addHeader("Content-Type", "multipart/form-data; boundary=--abcdefg");
			tempfile = _tempfileFS->open("/tempfile.temp");
			
			if (tempfile){
				responseCode = myClient.sendRequest("POST", &tempfile, tempfile.size());
				while(myClient.getStream().available()){
					Serial.write(myClient.getStream().read());
				}
			}
		}
	}
	myClient.end();
	return responseCode;
}


void Pushover::setToken(const char * token){
	_token = token;
}

void Pushover::setUser(const char * user){
	_user = user;
}

