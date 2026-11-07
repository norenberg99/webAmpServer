#include <Arduino.h>


//#include <SPI.h>
#include <Ethernet.h>
#include <AmpMaxim.h>

// Enter a MAC address and IP address for your controller below.
// fake mac address - must be unique on your network
byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };

// The static IP address will be dependent on your local network:
IPAddress ip(192, 168, 1, 177); // Set a static IP on your network

// Initialize the Ethernet server library
// with the IP address and port you want to use (port 80 is default for HTTP)
EthernetServer server(80);

// Global volume variable (0 to 100)
int gVolume = 0; 
// need a amp object to set the volume on the amp
AmpMaxim amp;

void setup() 
{
  // Open serial communications and wait for port to open:
  Serial.begin(115200);
  while (!Serial) 
  {
    ; // wait for serial port to connect. Needed for native USB port only
  }

  int8_t result = amp.begin(); // Initialize the amplifier
  if (result != 0) 
  {
    Serial.println("Failed to initialize amplifier");
  }
  else 
  {
    Serial.println("Amplifier initialized successfully");

    gVolume = amp.getCurrentVolume(); // Get the current volume from the amplifier
    Serial.print("Current volume: ");
    Serial.println(gVolume);
  }


  // start the Ethernet connection and the server:
  Ethernet.begin(mac, ip);
  server.begin();
  
  Serial.print("Server is at http://");
  Serial.println(Ethernet.localIP());
}

void loop() 
{
  // listen for incoming clients
  EthernetClient client = server.available();
  if (client) 
  {
    Serial.println("New Client Connected");
    // an http request ends with a blank line
    boolean currentLineIsBlank = true;
    String requestStr = "";

    while (client.connected()) 
    {
      if (client.available()) 
      {
        char c = client.read();
        requestStr += c; // Capture the request text

        // if you've gotten to the end of the line (received a newline
        // character) and the line is blank, the http request has ended,
        // so you can send a reply
        if (c == '\n' && currentLineIsBlank) 
        {
        
          // Check if the request contains a volume adjustment query
          // Format expected: GET /?val=XX HTTP/1.1
          if (requestStr.indexOf("GET /?val=") >= 0) 
          {
            int pos = requestStr.indexOf("GET /?val=");
            int spacePos = requestStr.indexOf(" HTTP", pos);
            String valStr = requestStr.substring(pos + 10, spacePos);
            
            int tempVolume = valStr.toInt();
            // Constrain volume between 0 and 100
            if (tempVolume < 0) tempVolume = 0;
            if (tempVolume > 100) tempVolume = 100;
            
            Serial.print("Volume updated to: ");
            Serial.println(tempVolume);

            // we could go set the new volume on the amplifier here, e.g., setAmplifierVolume(volume);
            if ( true == amp.setVolume(tempVolume) )
            {
                gVolume = tempVolume;
            }
            else
            {
                Serial.println("Failed to set volume on amplifier");
                gVolume = tempVolume;

            }

          }

          // send a standard http response header
          client.println("HTTP/1.1 200 OK");
          client.println("Content-Type: text/html");
          client.println("Connection: close");  // the connection will be closed after completion of the response
          client.println();
          
          // send the HTML web page
          client.println("<!DOCTYPE HTML>");
          client.println("<html>");
          client.println("<head>");
          client.println("<meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">");
          client.println("<title>Amplifier Control</title>");
          client.println("<style>");
          client.println("body { font-family: sans-serif; text-align: center; margin-top: 50px; background: #f0f0f0; }");
          client.println(".container { background: white; padding: 30px; display: inline-block; border-radius: 10px; box-shadow: 0 4px 6px rgba(0,0,0,0.1); }");
          client.println("h1 { color: #333; }");
          client.println(".vol-display { font-size: 48px; font-weight: bold; color: #007BFF; margin: 20px 0; }");
          client.println("input[type=range] { width: 80%; max-width: 300px; height: 15px; }");
          client.println("</style>");
          client.println("</head>");
          client.println("<body>");
          client.println("<div class=\"container\">");
          client.println("<h1>Patio Amp Volume</h1>");
          
          // Display current volume
          client.print("<div class=\"vol-display\">");
          client.print(gVolume);
          client.println("%</div>");
          
          // Volume slider
          client.print("<input type=\"range\" min=\"0\" max=\"100\" value=\"");
          client.print(gVolume);
          client.println("\" onchange=\"window.location.href='/?val=' + this.value;\">");
          
          client.println("</div>");
          client.println("</body>");
          client.println("</html>");
          break;
        }

        if (c == '\n') 
        {
          // you're starting a new line
          currentLineIsBlank = true;
        } else if (c != '\r') 
        {
          // you've gotten a character on the current line
          currentLineIsBlank = false;
        }
      }
    }

    // give the web browser time to receive the data
    delay(1);
    // close the connection:
    client.stop();
    Serial.println("Client Disconnected");
  }
}