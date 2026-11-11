/*--------------------------------------------------------------------------------
    Presents a simple web page with volume controls for each of the 6 different 
    Class D amps.  An I2C mux sits between the Arduino and the amps.

    The user adjusts the volume using a slider on the web page.  The volume is sent to the Arduino
    using a GET request.  The Arduino then sets the volume on the amp using the I2C mux.  

    
    Author: Jon Norenberg
    Date: 11/9/2026
--------------------------------------------------------------------------------*/

#include <Arduino.h>


#include <Ethernet.h>
#include <AmpMux.h>


// these are the mux ports and what we tell the mux when you want to talk to a specific amp.  

#define MUX_KITCHEN_PORT                0
#define MUX_MASTER_PORT                 1
#define MUX_OFFICE_PORT                 2
#define MUX_PATIO_PORT                  3
#define MUX_PLAYROOM_PORT               4
#define MUX_BATHROOM_PORT               5
#define MUX_AMP_COUNT                   6


String gAmplifierNames[MUX_AMP_COUNT] = {"Kitchen", "Master Bedroom", "Office", "Patio", "Playroom", "Master Bathroom"};


// Enter a MAC address and IP address for your controller below.
// fake mac address - must be unique on your network
byte gMacAddress[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };

// The static IP address will be dependent on your local network:
// all of our static IP addresses are in the 192.168.1.x range, so we will use
// our Norenberg router has all static IP addresses after 192.168.1.100
IPAddress gIPAddress(192, 168, 1, 177); // Set a static IP on your network

// Initialize the Ethernet server library
// with the IP address and port you want to use (port 80 is default for HTTP)
EthernetServer gServer(80);

// Array to store the volume levels (0 to 100) for the 6 amplifiers
int gVolumes[MUX_AMP_COUNT] = {50, 50, 50, 50, 50, 50};

// need a mux object to set/get the volume on the amps
AmpMux gMux;


// Function to extract channel and volume from the URL and update the array
void parseVolumeRequest(String req) 
{
  int chIndex = req.indexOf("ch=");
  int valIndex = req.indexOf("&val=");
  
  if (chIndex > 0 && valIndex > 0) 
  {
    int ch = req.substring(chIndex + 3, valIndex).toInt();
    
    // Find the space after the value (before HTTP/1.1)
    int spaceIndex = req.indexOf(" ", valIndex);
    int val = req.substring(valIndex + 5, spaceIndex).toInt();
    
    if (ch >= 0 && ch < MUX_AMP_COUNT && val >= 0 && val <= 100) 
    {

      // TODO: Add your hardware control code here (e.g., I2C digital pot, PWM, RS232)
      int8_t result = gMux.setVolume(ch, val); // Set the volume on the amp using the mux
      if (result == 0) 
      {
        gVolumes[ch] = val; // Update the volume in the array      

        Serial.print("Volume for Amplifier ");
        Serial.print(gAmplifierNames[ch]);
        Serial.print(" set successfully to ");
        Serial.println(gVolumes[ch]);
      }
      else
      {
        Serial.print("Error setting volume for Amplifier ");
        Serial.println(gAmplifierNames[ch]);
      }
    }
  }
}

// Function to send the HTML + JavaScript UI
void sendWebPage(EthernetClient &client) 
{ 
  client.println("HTTP/1.1 200 OK");
  client.println("Content-Type: text/html");
  client.println("Connection: close");
  client.println();
  
  // Use F() macro to keep static HTML strings in Flash memory instead of SRAM
  client.println(F("<!DOCTYPE HTML><html><head><meta name='viewport' content='width=device-width, initial-scale=1'>"));
  client.println(F("<style>body{font-family:sans-serif;max-width:400px;margin:20px auto;padding:10px;background:#1e1e1e;color:#fff;}"));
  client.println(F(".box{background:#2d2d2d;padding:15px;margin-bottom:10px;border-radius:8px;}"));
  client.println(F("input[type=range]{width:100%;margin-top:10px;}</style></head><body>"));
  client.println(F("<h2>House Amplifier VolumeControl</h2>"));

  // Dynamically generate the MUX_AMP_COUNT sliders with their current saved values
  for (int i = 0; i < MUX_AMP_COUNT; i++) 
  {
    client.print(F("<div class='box'><label>Amplifier "));
    client.print(gAmplifierNames[i]);
    client.print(F(": <span id='v"));
    client.print(i);
    client.print(F("'>"));
    client.print(gVolumes[i]);
    client.print(F("</span>%</label><br>"));
    
    client.print(F("<input type='range' min='0' max='100' value='"));
    client.print(gVolumes[i]);
    client.print(F("' oninput='document.getElementById(\"v"));
    client.print(i);
    client.print(F("\").innerText=this.value' onchange='sendVal("));
    client.print(i);
    client.println(F(",this.value)'></div>"));
  }

  // JavaScript function to send the value asynchronously using fetch()
  client.println(F("<script>function sendVal(ch,val){fetch('/set?ch='+ch+'&val='+val);}</script>"));
  client.println(F("</body></html>"));
}



void setup() 
{
  // Open serial communications and wait for port to open:
  Serial.begin(115200);
  while (!Serial) 
  {
    ; // wait for serial port to connect. Needed for native USB port only
  }

  // Initialize the mux and the amps behind it
  if (0 != gMux.begin()) 
  {
    Serial.println("Failed to initialize mux and amps");
  }
  else 
  {
    Serial.println("Mux and amps initialized successfully");
    for (int i = 0; i < MUX_AMP_COUNT; i++) 
    {
      gVolumes[i] = gMux.getVolume(i); // Get the current volume from each amplifier
      Serial.print("Current volume for ");
      Serial.print(gAmplifierNames[i]);
      Serial.print(": ");
      Serial.println(gVolumes[i]);
    }
  }


  // start the Ethernet connection and the server:
  Ethernet.begin(gMacAddress, gIPAddress);
  gServer.begin();
  
  Serial.print("Server is at http://");
  Serial.println(Ethernet.localIP());
}



void loop() 
{
  // Listen for incoming clients
  EthernetClient client = gServer.available();
  
  if (client) 
  {
    String request = "";
    boolean currentLineIsBlank = true;
    
    while (client.connected()) 
    {
      if (client.available()) 
      {
        char c = client.read();
        // Store the first line of the HTTP request
        if (request.length() < 100) 
        {
          request += c;
        }

        // If you've gotten to the end of the line (received a newline
        // character) and the line is blank, the http request has ended,
        // so you can send a reply
        if (c == '\n' && currentLineIsBlank) 
        {
          
          // Check if the request is updating a volume slider (e.g., GET /set?ch=0&val=75)
          if (request.indexOf("GET /set?") >= 0) 
          {
            parseVolumeRequest(request);
            
            // Send a quick 200 OK response without reloading the whole page
            client.println("HTTP/1.1 200 OK");
            client.println("Content-Type: text/plain");
            client.println("Connection: close");
            client.println();
            client.println("OK");
          } 
          else 
          {
            // Otherwise, serve the main HTML webpage
            sendWebPage(client);
          }
          break;
        }

        if (c == '\n') 
        {
          currentLineIsBlank = true;
        } else if (c != '\r') 
        {
          currentLineIsBlank = false;
        }
      }
    }
    // Give the web browser time to receive the data
    delay(1);
    // Close the connection:
    client.stop();
  }
}

