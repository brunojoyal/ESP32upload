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

Uploader::Uploader(const char *URL, const char *cert, FS *tempfileFS) : _URL(URL),  _tempfileFS(tempfileFS), _https(true), _cert(cert)
{
}

void Uploader::set(const char *URL){
	_URL = URL;
}

Result Uploader::send(MultipartMessage message)
{
	size_t tempfileSize;

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
		std::map< const char*, const char * >::iterator it = message.otherData.begin();
		while (it != message.otherData.end())
		{
			
			tempfile.printf("Content-Disposition: form-data; name=\"%s\"\r\n", it->first);
			tempfile.print("\r\n");
			tempfile.printf("%s\r\n", it->second);
			tempfile.print("----abcdefg\r\n");

			it++;
		}

		tempfile.printf("Content-Disposition: form-data; name=\"%s\"; filename=\"%s\"\r\n", message.name, message.filename);
		tempfile.printf("Content-Type: %s\r\n", message.contentType);
		tempfile.print("\r\n");

		log_v("File has length %u\n", message.file->size());
		while (message.file->available())
		{
			int read = message.file->read(buf, BUFLEN);
			tempfile.write(buf, read);
		}
		message.file->close();
		tempfile.print("\r\n");
		tempfile.print("----abcdefg--\r\n");
		tempfile.flush();
		tempfile.close();
		myClient.addHeader("Content-Type", "multipart/form-data; boundary=\"--abcdefg\"");
		tempfile = _tempfileFS->open("/tempfile.temp");
		tempfileSize = tempfile.size();

		if (tempfile)
		{
			log_v("Sending tempfile of length %u\n", tempfileSize);
			responseCode = myClient.sendRequest("POST", &tempfile, tempfile.size());
		}
	}

	myClient.end();
	Result result;
	result.responseCode = responseCode;
	result.fileSize = tempfileSize;
	return result;
}
