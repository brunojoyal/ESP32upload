#include "ESP32upload.h"

Uploader::Uploader(const char *URL) : _URL(URL), _https(false)
{
}

Uploader::Uploader(const char *URL, FS *tempfileFS) : _URL(URL), _tempfileFS(tempfileFS), _https(false)
{
}

Uploader::Uploader(const char *URL, const char *cert) : _URL(URL), _https(true), _cert(cert)
{
}

Uploader::Uploader(const char *URL, const char *cert, FS *tempfileFS) : _URL(URL), _https(true), _cert(cert), _tempfileFS(tempfileFS)
{
}

int Uploader::send(PushoverMessage newMessage)
{

	HTTPClient myClient;
	int responseCode = 0;
	if (_https)
	{
		myClient.begin(_URL, _cert);
	}
	else
	{
		myClient.begin(_URL);
	}

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
			int read = newMessage.attachment->read(buf, 256);
			tempfile.write(buf, read);
			vTaskDelay(1);
		}
		newMessage.attachment->close();
		tempfile.print("----abcdefg--\r\n");
		tempfile.flush();
		tempfile.close();
		myClient.addHeader("Content-Type", "multipart/form-data; boundary=--abcdefg");
		tempfile = _tempfileFS->open("/tempfile.temp");

		if (tempfile)
		{
			responseCode = myClient.sendRequest("POST", &tempfile, tempfile.size());
			while (myClient.getStream().available())
			{
				Serial.write(myClient.getStream().read());
			}
		}
	}

	myClient.end();
	return responseCode;
}
