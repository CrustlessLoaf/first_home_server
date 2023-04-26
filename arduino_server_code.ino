
#include <SPI.h>
#include <Ethernet.h>
#include <Servo.h>
#include<Ethernet.h>
#include<SD.h>
int led = 4;
Servo microservo;
#define REQ_BUF_SZ   20
byte mac[] = { 0xA6, 0x6F, 0xBB, 0x89, 0xAC, 0xED };   //physical mac address
byte ip[] = { 10, 0, 0, 247};                      // ip in lan (that's what you need to use in your browser. ("192.168.1.178")
byte gateway[] = {10, 0, 0, 1};                // internet access via router
byte subnet[] = { 255, 255, 255, 0 };                  //subnet mask
EthernetServer server(80);                             //server port
File webFile;
char HTTP_req[REQ_BUF_SZ] = {0}; // buffered HTTP request stored as null terminated string
char req_index = 0;              // index into HTTP_req buffer

void setup()
{
    // disable Ethernet chip
    pinMode(10, OUTPUT);
    digitalWrite(10, HIGH);
    
    Serial.begin(9600);       // for debugging
    
    // initialize SD card
    Serial.println("Initializing SD card...");
    if (!SD.begin(4)) {
        Serial.println("ERROR - SD card initialization failed!");
        return;    // init failed
    }
    Serial.println("SUCCESS - SD card initialized.");
    // check for index.htm file
    if (!SD.exists("index.htm")) {
        Serial.println("ERROR - Can't find index.htm file!");
        return;  // can't find index file
    }
    Serial.println("SUCCESS - Found index.htm file.");
    
      Ethernet.begin(mac, ip, gateway, subnet);
    server.begin();
    Serial.print("server is at ");
    Serial.println(Ethernet.localIP());
    byte macBuffer[6];  // create a buffer to hold the MAC address
    Ethernet.MACAddress(macBuffer); // fill the buffer
    Serial.print("The MAC address is: ");
    for (byte octet = 0; octet < 6; octet++) {
      Serial.print(macBuffer[octet], HEX);
      if (octet < 5) {
        Serial.print(':');
      }
    }
}

void loop()
{
    EthernetClient client = server.available();  // try to get client

    if (client) {  // got client?
        boolean currentLineIsBlank = true;
        while (client.connected()) {
            if (client.available()) {   // client data available to read
                char c = client.read(); // read 1 byte (character) from client
                // buffer first part of HTTP request in HTTP_req array (string)
                // leave last element in array as 0 to null terminate string (REQ_BUF_SZ - 1)
                if (req_index < (REQ_BUF_SZ - 1)) {
                    HTTP_req[req_index] = c;          // save HTTP request character
                    req_index++;
                }
                // print HTTP request character to serial monitor
                Serial.print(c);
                // last line of client request is blank and ends with \n
                // respond to client only after last line received
                if (c == '\n' && currentLineIsBlank) {
                    // open requested web page file
                    if (StrContains(HTTP_req, "GET / ")
                                 || StrContains(HTTP_req, "GET /index.htm")) {
                        client.println("HTTP/1.1 200 OK");
                        client.println("Content-Type: text/html");
                        client.println("Connnection: close");
                        client.println();
                        webFile = SD.open("index.htm");        // open web page file
                    }
                    else if (StrContains(HTTP_req, "GET /styles.css")) {
                        client.println("HTTP/1.1 200 OK");
                        client.println("Content-Type: text/css");
                        client.println("Connnection: close");
                        client.println();
                        webFile = SD.open("/styles.css");        // open web page file
                    }
                    else if (StrContains(HTTP_req, "GET /jessica.htm")) {
                        webFile = SD.open("/jessica.htm");
                        if (webFile) {
                        client.println("HTTP/1.1 200 OK");
                        client.println("Content-Type: text/html");
                        client.println("Connnection: close");
                        client.println();
                        }
                    }
                    else if (StrContains(HTTP_req, "GET /conf.png")) {
                        webFile = SD.open("/conf.png");
                        if (webFile) {
                            client.println("HTTP/1.1 200 OK");
                            client.println();
                        }
                    }
//                    else if (StrContains(HTTP_req, "GET /deadlift.gif")) {
//                        webFile = SD.open("/deadlift.gif");
//                        if (webFile) {
//                            client.println("HTTP/1.1 200 OK");
//                            client.println();
//                        }
//                    }
//                    else if (StrContains(HTTP_req, "GET /squat.gif")) {
//                        webFile = SD.open("/squat.gif");
//                        if (webFile) {
//                            client.println("HTTP/1.1 200 OK");
//                            client.println();
//                        }
//                    }
                    if (webFile) {
                        while(webFile.available()) {
                            client.write(webFile.read()); // send web page to client
                        }
                        webFile.close();
                    }
                    // reset buffer index and all buffer elements to 0
                    req_index = 0;
                    StrClear(HTTP_req, REQ_BUF_SZ);
                    break;
                }
                // every line of text received from the client ends with \r\n
                if (c == '\n') {
                    // last character on line of received text
                    // starting new line with next character read
                    currentLineIsBlank = true;
                } 
                else if (c != '\r') {
                    // a text character was received from client
                    currentLineIsBlank = false;
                }
            } // end if (client.available())
        } // end while (client.connected())
        delay(1);      // give the web browser time to receive the data
        client.stop(); // close the connection
    } // end if (client)
}

// sets every element of str to 0 (clears array)
void StrClear(char *str, char length)
{
    for (int i = 0; i < length; i++) {
        str[i] = 0;
    }
}

// searches for the string sfind in the string str
// returns 1 if string found
// returns 0 if string not found
char StrContains(char *str, char *sfind)
{
    char found = 0;
    char index = 0;
    char len;

    len = strlen(str);
    
    if (strlen(sfind) > len) {
        return 0;
    }
    while (index < len) {
        if (str[index] == sfind[found]) {
            found++;
            if (strlen(sfind) == found) {
                return 1;
            }
        }
        else {
            found = 0;
        }
        index++;
    }

    return 0;
}
