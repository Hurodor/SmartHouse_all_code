#include "Request.h"


Request::Request(const char* ssid, const char* pw, const String baseUrl)
    : ssid{ssid}, password{pw}, baseUrl{baseUrl}{
    
    }

void Request::init(){
    WiFi.begin(ssid, password);

    Serial.println("Connecting");
    while (WiFi.status() != WL_CONNECTED){
        Serial.print(".");
        delay(1000);
    }

    Serial.println("");
    Serial.println("Connected");
}


String Request::get(const String& endPoint){
    if (WiFi.status() == WL_CONNECTED){
        HTTPClient http;
        String url = getURL(endPoint);

        http.begin(url.c_str());
        int httpRespons = http.GET(); //vanlig get

        if (httpRespons != 200){
            Serial.println(httpRespons);  // print on bad request
            http.end();
            return String(httpRespons);
        }

        String content = http.getString();

        http.end();
        return content;
    }
    return String("");
}

String Request::post(const String& endPoint, const String& payload){
    if (WiFi.status() == WL_CONNECTED){
        HTTPClient http;
        String url = getURL(endPoint);

        http.begin(url.c_str());
        http.addHeader("Content-Type", "application/json");

        int httpRespons = http.POST(payload); //vanlig get

        if (httpRespons != 200){
            Serial.println(httpRespons);  // print on bad request
            http.end();
            return String(httpRespons);
        }
        
        String content = http.getString();

        http.end();
        return content;
    }
  return String("");
}

String Request::put(const String& endPoint, const String& payload){
  if (WiFi.status() == WL_CONNECTED){
    HTTPClient http;
    String url = getURL(endPoint);

    http.begin(url.c_str());
    http.addHeader("Content-Type", "application/json");

    int httpRespons = http.PUT(payload);

    if (httpRespons != 200){
      Serial.println(httpRespons);
      http.end();
      return String(httpRespons);
    }

    String content = http.getString();

    http.end();
    return content;
  }
  return String("");
}
