#ifndef Request_h
#define Request_h


#include <WiFi.h>
#include <HTTPClient.h>

class Request{
  const char* ssid;
  const char* password;
  const String baseUrl;

public:
  Request(const char* ssid, const char* pw, const String baseUrl);
  void init();
  String getURL(String endPoint) const {return baseUrl + endPoint;}
  String get(const String& endPoint);
  String post(const String& endPoint, const String& payload);
  String put(const String& endPoint, const String& payload);
};

#endif //Request_h
