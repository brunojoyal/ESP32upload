#include <Arduino.h>
#include <WiFiClientSecure.h>
#include <HTTPClient.h>
#include <FS.h>
#include <SPIFFS.h>
#include <map.h>

struct MultipartMessage
{
public:
	std::map<std::string, std::string> otherData;		//list containing key-value pairs for message parts other than the file
	std::string filename;	
	File * file;
};

class Uploader
{
private:
	FS *_tempfileFS = &SPIFFS;
	const char * _URL;
	bool _https;
	const char * _cert;

public:
	Uploader(const char * URL);
	Uploader(const char * URL, FS *tempfileFS);
	Uploader(const char * URL, const char * cert);
	Uploader(const char * URL, const char * cert, FS *tempfileFS);
	int send(MultipartMessage message);
};
