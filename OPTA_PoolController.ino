#pragma region header

//Lib's
#include <NTPClient.h>
#include <mbed_mktime.h>
#include <Ethernet.h>
#include <EthernetUdp.h>
//#include "mbed.h"
#include <TimeLib.h> // For handling time and date
#include <FlashIAPBlockDevice.h>
#include <TDBStore.h>
#include "FlashIAPLimits.h"
#include <MDNS_Generic.h>


//Defines
#define greenLED     LEDG 
#define redLED       LEDR
#define userButton   BTN_USER
//#define LogToUSBDrive   //Comment if you want USBport as Serial for debug

/*#ifdef LogToUSBDrive
  #include "Arduino_UnifiedStorage.h"
  bool usbInitialized = false;        //Used for USB logging
  volatile bool usbAvailable = false; //Used for USB logging
  Arduino_UnifiedStorage::debuggingModeEnabled  =  false;
#endif*/

using namespace mbed;
using namespace rtos;

//Variables
bool StateChanged = false;

bool debugFlag = true;
bool isWinter = false;
//bool pumpOverride = false;

bool runState = true;

int watchDogTime = 10000;// Default Watchdogtime in ms

unsigned long lastDebounceTime = 0; // the last time the output pin was toggled
const unsigned long debounceDelay = 50;   // debounce time in milliseconds

//const char* mqttServer = "10.0.0.5";// MQTT Broker
//const int mqttPort = 1883; //MQTT Port

//LEDs and Relays
const int LED[] = {LED_D0, LED_D1, LED_D2, LED_D3};
const int RELAY[] = {D0, D1, D2, D3}; 

//D0 = Light
//D1 = Pump
//D2 = UV
//D3 = Heater

//Poollight Color
int sequenceColor = 1;// Default pool light color (white)

struct TimeSetting {
  String name;
  String value;
};

TimeSetting timeSettings[] = {
  {"fromPump",    "00:00"},
  {"toPump",      "00:00"},
  {"fromPump2",   "00:00"},
  {"toPump2",     "00:00"},
  {"fromUV",      "00:00"},
  {"toUV",        "00:00"},
  {"fromUV2",     "00:00"},
  {"toUV2",       "00:00"},
  {"fromHeater",  "00:00"},
  {"toHeater",    "00:00"},
  {"fromHeater2", "00:00"},
  {"toHeater2",   "00:00"},
  {"fromLight",   "00:00"},
  {"toLight",     "00:00"},
  {"fromLight2",  "00:00"},
  {"toLight2",    "00:00"}
};

TimeSetting defaultTimeSettings[] = {
  {"fromPump",    "08:00"},
  {"toPump",      "12:00"},
  {"fromPump2",   "14:00"},
  {"toPump2",     "18:00"},
  {"fromUV",      "00:00"},
  {"toUV",        "00:00"},
  {"fromUV2",     "00:00"},
  {"toUV2",       "00:00"},
  {"fromHeater",  "00:00"},
  {"toHeater",    "00:00"},
  {"fromHeater2", "00:00"},
  {"toHeater2",   "00:00"},
  {"fromLight",   "00:00"},
  {"toLight",     "00:00"},
  {"fromLight2",  "00:00"},
  {"toLight2",    "00:00"}
};

const int numberOfSettings = sizeof(timeSettings) / sizeof(TimeSetting);

//Instances
EthernetUDP NTPUdp;
NTPClient timeClient(NTPUdp); // NTP client Ethernet UDP configuration

EthernetClient ethClient;
EthernetServer server(80);

EthernetUDP udp;  //mdns Service
MDNS mdns(udp);

FlashIAPLimits iapLimits { getFlashIAPLimits() };
FlashIAPBlockDevice blockDevice(iapLimits.start_address, iapLimits.available_size);
TDBStore store(&blockDevice);

/*#ifdef LogToUSBDrive
  USBStorage usbStorage;
  Folder backupFolder = Folder();
#endif*/

static rtos::Thread Thread2;
//static rtos::Thread Thread3;
//static rtos::Thread Thread4;

#pragma endregion 

void setup() {
  
  delay(2000);

  #ifndef LogToUSBDrive
    Serial.begin(115200);
    Serial.println("Start setup");  
  #endif
  
  initIOs();  //Initialize IO Pins

  Ethernet.begin();

  configMDNS();

  initTime();
  
  #ifndef LogToUSBDrive
    Serial.println("Read from Flash");    
  #endif

  readFromFlash(); // To read the settings from flash

  server.begin(); //start Webserver

#ifdef LogToUSBDrive
    usbStorage = USBStorage();                        //USB Logging functions
    usbStorage.onConnect(connectionCallback);
    usbStorage.onDisconnect(disconnectionCallback);    
#endif




 #ifndef LogToUSBDrive
    Serial.print("IP: ");
    Serial.println(Ethernet.localIP());
    Serial.println("Setup finished");    
  #endif

  //Starting other Threads
  Thread2.start(loop2);
  //Thread3.start(loop3);
  //Thread4.start(loop4);

  //setWatchdog();

  if(runState){
    startProgram();
  }else{
    stopProgram();
  }

}

void loop() {

  mdns.run();

  // Handle incoming client requests
  EthernetClient client = server.available();
  if (client) {
    handleClient(client);
  }

  // Reset Watchdog timer
  //mbed::Watchdog::get_instance().kick();
}

void loop2() {
  while(1){
    // if it is not Winter and runState = true (Automatic)

    ThisThread::sleep_for(1000);
    if(debugFlag){
      #ifndef LogToUSBDrive
        Serial.println(runState ? "Running state: Running" : "Running state: Not running");    
      #endif

      if(StateChanged)
      {
        StateChanged = false;
        #ifndef LogToUSBDrive
          Serial.print("State changed. Write to Flash");    
        #endif
        writeToFlash();
      }
    }
    
    if(!isWinter && runState)
    {
      #ifndef LogToUSBDrive
        Serial.println("Manage Relays");    
      #endif

      manageRelays();  //proceed with relays
      ThisThread::yield();
    }else{
      PumpOff();
      HeaterOff();
      UVOff();
      turnOffPoolLight();
    }
  }
}

void loop3() {
  while(1){
    ThisThread::sleep_for(5000);
  }
}

void loop4() {
  while(1){
    ThisThread::sleep_for(5000);
  }
}

//*************************************************
// Functions used in setup
//*************************************************
#pragma region setup
//*************************************************
// Specific functions for ISR User Button /runstate
//*************************************************
void userButtonPressed() {
  if ((millis() - lastDebounceTime) > debounceDelay) {
      lastDebounceTime = millis();
      runState = !runState; // Toggle state

      StateChanged = true;
      // Start or stop the program based on the state
      if (runState) {
          startProgram();
      } else {
          stopProgram();
      }
  }
}

//*************************************************
// Set Watchdog enable and define timeout
//*************************************************
void setWatchdog(){
  
  mbed::Watchdog::get_instance().start(watchDogTime);// Initialize Watchdog timer 10 seconds timeout
}

//*************************************************
// Initialize IO Pins
//*************************************************
void initIOs(){

  // Initialize LEDs and User Button
  pinMode(greenLED, OUTPUT);
  digitalWrite(greenLED,LOW);

  pinMode(redLED, OUTPUT);
  digitalWrite(redLED,LOW);

  pinMode(userButton, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(userButton), userButtonPressed, FALLING);

  // Initialize Relays and corresponding LEDs
  for(int i = 0; i < 4; i++) {
    pinMode(RELAY[i], OUTPUT); 
    digitalWrite(RELAY[i], LOW);
    pinMode(LED[i], OUTPUT); 
    digitalWrite(LED[i], LOW);
  }

}
#pragma endregion

//*************************************************
// General functions
//*************************************************
#pragma region general
//*************************************************
// Functions to manage Relays
//*************************************************
void manageRelays() {
    String currentTime = getLocalTime().substring(0, 5);

    
    #pragma region pump
      // Extract time settings for both pump schedules
      String onTimePump1 = timeSettings[0].value; // "fromPump" value
      String offTimePump1 = timeSettings[1].value; // "toPump" value
      String onTimePump2 = timeSettings[2].value; // "fromPump2" value
      String offTimePump2 = timeSettings[3].value; // "toPump2" value

      // Check if current time is within either of the pump schedules
      bool isPump1Active = (currentTime >= onTimePump1 && currentTime < offTimePump1) || 
                          (onTimePump1 > offTimePump1 && !(currentTime >= offTimePump1 && currentTime < onTimePump1));
      bool isPump2Active = (currentTime >= onTimePump2 && currentTime < offTimePump2) || 
                          (onTimePump2 > offTimePump2 && !(currentTime >= offTimePump2 && currentTime < onTimePump2));
    #pragma endregion

    #pragma region heater
      // Extract time settings for both pump schedules
      String onTimeHeater1 = timeSettings[8].value; // "froHeater" value
      String offTimeHeater1 = timeSettings[9].value; // "toHeater" value
      String onTimeHeater2 = timeSettings[10].value; // "fromHeater" value
      String offTimeHeater2 = timeSettings[11].value; // "toHeater" value

      // Check if current time is within either of the pump schedules
      bool isHeater1Active = (currentTime >= onTimeHeater1 && currentTime < offTimeHeater1) || 
                          (onTimeHeater1 > offTimeHeater1 && !(currentTime >= offTimeHeater1 && currentTime < onTimeHeater1));
      bool isHeater2Active = (currentTime >= onTimeHeater2 && currentTime < offTimeHeater2) || 
                          (onTimeHeater2 > offTimeHeater2 && !(currentTime >= offTimeHeater2 && currentTime < onTimeHeater2));
    #pragma endregion

    // Turn the pump on or off based on the combined condition
      if (isPump1Active || isPump2Active) {
          PumpOn();
      } else if (!(isPump1Active || isPump2Active) && !(isHeater1Active || isHeater2Active)) {
          PumpOff();
      }

    // Turn the heater and pump on or off based on the combined condition
      if (isHeater1Active || isHeater2Active) {
          PumpOn();
          HeaterOn();
      } else {
          HeaterOff();
      }

}

//*************************************************
// Function to start the program
//*************************************************
void startProgram() {
  digitalWrite(redLED, LOW);
  digitalWrite(greenLED, HIGH);
  runState = true;
}

//*************************************************
// Function to stop the program
//*************************************************
void stopProgram() {
  digitalWrite(redLED, HIGH);
  digitalWrite(greenLED, LOW);
  runState = false;
}

#pragma endregion

//*************************************************
// Function for Outputs
//*************************************************
#pragma region Outputs
void PumpOn(){
  digitalWrite(RELAY[1], HIGH);
  digitalWrite(LED[1], HIGH);
  //send status to Nextion
  #ifndef LogToUSBDrive
    Serial.println("Pump On");    
  #endif

  UVOn();

#ifdef LogToUSBDrive
    logToUSB("Pump On"); // Log to USB    
#endif


}

void PumpOff(){
  digitalWrite(RELAY[1], LOW);
  digitalWrite(LED[1], LOW);
  //send status to Nextion
  #ifndef LogToUSBDrive
    Serial.println("Pump Off");    
  #endif
  
  UVOff();
  #ifdef LogToUSBDrive
      logToUSB("Pump Off"); // Log to USB    
  #endif



}

void UVOn(){
  digitalWrite(RELAY[2], HIGH);
  digitalWrite(LED[2], HIGH);
  //send status to Nextion
#ifdef LogToUSBDrive
      logToUSB("UV On"); // Log to USB
#endif

}

void UVOff(){
  digitalWrite(RELAY[2], LOW);
  digitalWrite(LED[2], LOW);
  //send status to Nextion
#ifdef LogToUSBDrive
      logToUSB("UV Off"); // Log to USB
#endif

}

void HeaterOn(){
  digitalWrite(RELAY[3], HIGH);
  digitalWrite(LED[3], HIGH);
  //send status to Nextion
#ifdef LogToUSBDrive
      logToUSB("Heater On"); // Log to USB
#endif

}

void HeaterOff(){
  digitalWrite(RELAY[3], LOW);
  digitalWrite(LED[3], LOW);
  //send status to Nextion
#ifdef LogToUSBDrive
      logToUSB("Heater Off"); // Log to USB
#endif

}

//******************************************
// Specific functions for pool light control
//******************************************

void setColor(int colorNumber) {
  // Set the light to the specified color sequence
  if (colorNumber > 1 && colorNumber <= 14) {
    while (sequenceColor != colorNumber) {
      changePoolLight(); // Change to the desired color
      delay(500); // Delay for color change
    }
    #ifndef LogToUSBDrive
      Serial.print("Changed Coloursequence to: ");
      Serial.println(sequenceColor);        
    #endif

  } else {
    // Default to white if colorNumber is 1
    PoolLightWhite();
    #ifndef LogToUSBDrive
      Serial.println("Changed Coloursequence to: 1");        
    #endif
    
  }
}

void PoolLightWhite() {
  sequenceColor = 1;
  digitalWrite(RELAY[0], LOW);
  digitalWrite(LED[0], LOW);
  delay(3000);
  digitalWrite(RELAY[0], HIGH);
  digitalWrite(LED[0], HIGH);
}

void changePoolLight() {
  sequenceColor = sequenceColor % 14 + 1; // Cycle through colors
  digitalWrite(RELAY[0], LOW);
  digitalWrite(LED[0], LOW);
  delay(500);
  digitalWrite(RELAY[0], HIGH);
  digitalWrite(LED[0], HIGH);
}

void turnOffPoolLight() {
  digitalWrite(RELAY[0], LOW);
  digitalWrite(LED[0], LOW);
}

void turnOnPoolLight() {
  digitalWrite(RELAY[0], HIGH);
  digitalWrite(LED[0], HIGH);
}
#pragma endregion

//******************************************
// ETHERNET AND WEB
//******************************************
#pragma region EthernetAndWeb
void configMDNS(){
   String hostname = String("PoolController");
  
  hostname.toLowerCase();
  hostname.replace(" ",  "-");
  hostname.replace(".",  "-");
  hostname.replace("_",  "-");

  mdns.begin(Ethernet.localIP(), hostname.c_str());

  String mDNS_Service = String("PoolController") + "_mDNS_Webserver._http";

  mdns.addServiceRecord(mDNS_Service.c_str(), 80, MDNSServiceTCP);
}

void handleClient(EthernetClient& client) {
  String currentLine = "";
  boolean currentLineIsBlank = true;
  String requestType = "";

  while (client.connected()) {
    if (client.available()) {
      char c = client.read();
      if (c == '\n' && currentLineIsBlank) {
        if (requestType == "POST") {
          // POST request, handle the data
          String postData = "";
          while (client.available()) {
            char ch = client.read();
            postData += ch;
          }

          for (int i = 0; i < numberOfSettings; i++) {
            String key = timeSettings[i].name + "=";
            int valueIndex = postData.indexOf(key) + key.length();
            int nextAmp = postData.indexOf('&', valueIndex);
            timeSettings[i].value = urlDecode(postData.substring(valueIndex, nextAmp != -1 ? nextAmp : postData.length()));

            #ifndef LogToUSBDrive
              Serial.print(timeSettings[i].name + ": ");
              Serial.println(timeSettings[i].value);                
            #endif
            
          }

          // Handle runState
          if (postData.indexOf("runState=true") != -1) {
              runState = true;
              startProgram();
          } else if (postData.indexOf("runState=false") != -1) {
              runState = false;
              stopProgram();
          }
          #ifndef LogToUSBDrive
            Serial.println(runState);    
          #endif

          // Handle debugFlag
          if (postData.indexOf("debugFlag=true") != -1) {
              debugFlag = true;
          } else if (postData.indexOf("debugFlag=false") != -1) {
              debugFlag = false;
          }
          #ifndef LogToUSBDrive
            Serial.println(debugFlag);              
          #endif

          // Handle isWinter
          if (postData.indexOf("isWinter=true") != -1) {
              isWinter = true;
          } else if (postData.indexOf("isWinter=false") != -1) {
              isWinter = false;
          }
          #ifndef LogToUSBDrive
            Serial.println(isWinter);              
          #endif

          #ifndef LogToUSBDrive
            Serial.println("Write to Flash");              
          #endif
          
          writeToFlash();
        }

        // Send the webpage to the client
        client.println("HTTP/1.1 200 OK");
        client.println("Content-type:text/html");
        client.println();
        sendWebPage(client);

        break;
      }
      if (c == '\n') {
        currentLineIsBlank = true;
        currentLine = "";
      } else if (c != '\r') {
        currentLine += c;
        currentLineIsBlank = false;
      }
      // Check if the request is GET or POST
      if (currentLine.endsWith("GET / ")) {
        requestType = "GET";
      } else if (currentLine.startsWith("POST")) {
        requestType = "POST";
      }
    }
  }
  client.stop();
}

void sendWebPage(EthernetClient& client) {
  client.println("<html><head><title>Pool Controller</title>");
  client.println("<style>");
  client.println("body { font-size: 30px; }"); // Adjust the base font size as per your requirement
  client.println("input, select, button { font-size: 30px; }"); // Adjust font size for input fields, dropdowns and buttons
  client.println("</style>");
  client.println("<link rel='icon' href='data:;base64,iVBORw0KGgo='></head>");
  client.println("<body>");

  // Fieldset for runState
  client.println("<fieldset>");
  client.println("  <legend>Run State</legend>");
  client.println("  <input type='radio' id='runStateOn' name='runState' value='true'" + String(runState ? " checked" : "") + "><label for='runStateOn'>On</label>");
  client.println("  <input type='radio' id='runStateOff' name='runState' value='false'" + String(!runState ? " checked" : "") + "><label for='runStateOff'>Off</label>");
  client.println("</fieldset>");

  // Fieldset for debugFlag
  client.println("<fieldset>");
  client.println("  <legend>Debug</legend>");
  client.println("  <input type='radio' id='debugOn' name='debugFlag' value='true'" + String(debugFlag ? " checked" : "") + "><label for='debugOn'>Yes</label>");
  client.println("  <input type='radio' id='debugOff' name='debugFlag' value='false'" + String(!debugFlag ? " checked" : "") + "><label for='debugOff'>No</label>");
  client.println("</fieldset>");

  // Fieldset for isWinter
  client.println("<fieldset>");
  client.println("  <legend>Wintermodus</legend>");
  client.println("  <input type='radio' id='winterYes' name='isWinter' value='true'" + String(isWinter ? " checked" : "") + "><label for='winterYes'>Yes</label>");
  client.println("  <input type='radio' id='winterNo' name='isWinter' value='false'" + String(!isWinter ? " checked" : "") + "><label for='winterNo'>No</label>");
  client.println("</fieldset>");

  client.println("<fieldset>");
  client.println("  <legend>Zeiten</legend>");

  for (int i = 0; i < numberOfSettings; i++) {
    client.println("<small>" + timeSettings[i].name + "</small>  <input type='time' name='" + timeSettings[i].name + "' id='" + timeSettings[i].name + "' value='" + timeSettings[i].value + "'><br>");
  }

    client.println("</fieldset>");

  client.println("  <input type='button' onclick='sendTimes()' value='Speichern'>");
  client.println("</form>");
  client.println("<script>");
  client.println("function sendTimes() {");
  client.println("  var xhttp = new XMLHttpRequest();");
  client.println("  xhttp.open('POST', '/', true);");
  client.println("  xhttp.setRequestHeader('Content-type', 'application/x-www-form-urlencoded');");
  client.println("  var params = '';");
  for (int i = 0; i < numberOfSettings; i++) {
    client.println("  params += '" + timeSettings[i].name + "=' + document.getElementById('" + timeSettings[i].name + "').value + '&';");
  }

  // Append runState, debugFlag, and isWinter to params
  client.println("  params += 'runState=' + document.querySelector('input[name=\"runState\"]:checked').value + '&';");
  client.println("  params += 'debugFlag=' + document.querySelector('input[name=\"debugFlag\"]:checked').value + '&';");
  client.println("  params += 'isWinter=' + document.querySelector('input[name=\"isWinter\"]:checked').value + '&';");


  client.println("  xhttp.send(params);");
  client.println("}");
  client.println("</script></body></html>");
}

String urlDecode(String str) {
  String encodedString = "";
  char c, code0, code1;
  for (unsigned int i = 0; i < str.length(); i++) {
    c = str.charAt(i);
    if (c == '+') {
      encodedString += ' ';
    } else if (c == '%') {
      i++;
      code0 = str.charAt(i);
      i++;
      code1 = str.charAt(i);
      c = (decodeChar(code0) << 4) | decodeChar(code1);
      encodedString += c;
    } else {
      encodedString += c;
    }
    yield();
  }
  return encodedString;
}

char decodeChar(char c) {
  if (c >= '0' && c <= '9') {
    return c - '0';
  } else if (c >= 'a' && c <= 'f') {
    return c - 'a' + 10;
  } else if (c >= 'A' && c <= 'F') {
    return c - 'A' + 10;
  }
  return 0;
}
#pragma endregion

//******************************************
// RTC and Time
//******************************************
#pragma region RTC
//*************************************************
// Get Time from NTP and store it in RTC
// once in setup and every 30 minutes
//*************************************************
void initTime() {
    timeClient.begin();
    timeClient.update();
    
    unsigned long epochTimeUTC = timeClient.getEpochTime();
    unsigned long epochTimeLocal;

    if (isDaylightSavingTime(epochTimeUTC)) {
        epochTimeLocal = epochTimeUTC + 7200; // UTC + 2 hours for daylight saving
    } else {
        epochTimeLocal = epochTimeUTC + 3600; // UTC + 1 hour for standard time
    }

    set_time(epochTimeLocal); // Set internal RTC to local time
    timeClient.end();
}

//******************************************
// Specific functions for get RTC Time
//******************************************
void displayRTC() {
  Serial.println();
  Serial.print("RTC time: ");
  Serial.println(getLocalTime());
}

// Function to retrieve and format the current time
String getLocalTime() {
  char buffer[32];
  tm t;
  _rtc_localtime(time(NULL), &t, RTC_FULL_LEAP_YEAR_SUPPORT);
  strftime(buffer, 32, "%k:%M", &t);
  return String(buffer);
}

//*************************************************
// Function to calculate if it's Summer or Wintertime
//*************************************************
bool isDaylightSavingTime(unsigned long epoch) {
  TimeElements te;
  breakTime(epoch, te); // Convert epoch time to TimeElements structure

  int month = te.Month;
  int day = te.Day;
  int weekday = te.Wday; // 1 = Sunday, 2 = Monday, ..., 7 = Saturday

  // Daylight saving time starts on the last Sunday of March
  bool startDST = (month == 3) && (day > (31 - 7)) && (weekday == 1);

  // Daylight saving time ends on the last Sunday of October
  bool endDST = (month == 10) && (day > (31 - 7)) && (weekday == 1);

  return (month > 3 && month < 10) || startDST || (month == 10 && !endDST);
}
#pragma endregion

#pragma region FLASH
void writeToFlash() {
    int result = store.init();
    if (result != MBED_SUCCESS) {
        #ifndef LogToUSBDrive
          Serial.println("Error initializing storage");    
        #endif

        return;
    }

    for (int i = 0; i < numberOfSettings; i++) {
        String key = timeSettings[i].name;
        String value = timeSettings[i].value;
        store.set(key.c_str(), value.c_str(), value.length() + 1, 0);
    }

    // Use temporary non-volatile variables
    bool tempRunState = runState;
    bool tempDebugFlag = debugFlag;
    bool tempIsWinter = isWinter;

    store.set("runState", &tempRunState, sizeof(tempRunState), 0);
    store.set("debugFlag", &tempDebugFlag, sizeof(tempDebugFlag), 0);
    store.set("isWinter", &tempIsWinter, sizeof(tempIsWinter), 0);

    store.deinit();
}

void readFromFlash() {
    int result = store.init();
    if (result != MBED_SUCCESS) {
        Serial.println("Error initializing storage");
        return;
    }

    char buffer[32]; // Adjust size as needed
    size_t actual_size;

    for (int i = 0; i < numberOfSettings; i++) {
        String key = timeSettings[i].name;
        result = store.get(key.c_str(), buffer, sizeof(buffer), &actual_size);
        if (result == MBED_SUCCESS) {
            timeSettings[i].value = String(buffer);
        } else {
            // If failed to read, use default value
            #ifndef LogToUSBDrive
              Serial.print("Using default for ");
              Serial.println(key);    
            #endif

            timeSettings[i].value = defaultTimeSettings[i].value;
        }
    }

    // Read boolean variables
    bool tempRunState, tempDebugFlag, tempIsWinter;

    result = store.get("runState", &tempRunState, sizeof(tempRunState), &actual_size);
    if (result == MBED_SUCCESS) {
        runState = tempRunState;
    } else {
      #ifndef LogToUSBDrive
        Serial.println("Error reading runState, using default");    
      #endif

    }

    result = store.get("debugFlag", &tempDebugFlag, sizeof(tempDebugFlag), &actual_size);
    if (result == MBED_SUCCESS) {
        debugFlag = tempDebugFlag;
    } else {
      #ifndef LogToUSBDrive
        Serial.println("Error reading debugFlag, using default");    
      #endif

    }

    result = store.get("isWinter", &tempIsWinter, sizeof(tempIsWinter), &actual_size);
    if (result == MBED_SUCCESS) {
        isWinter = tempIsWinter;
    } else {
      #ifndef LogToUSBDrive
        Serial.println("Error reading isWinter, using default");    
      #endif

    }

    store.deinit();
}
#pragma endregion

#pragma region USB
#ifdef LogToUSBDrive
void logToUSB(String message) {
    if (!usbAvailable) {
        #ifndef LogToUSBDrive
            Serial.println("USB not available");    
        #endif

        return;
    }

    if (!usbInitialized) {
        usbStorage.begin();
        Folder usbRoot = usbStorage.getRootFolder();
        String folderName = "pool_logs";
        backupFolder = usbRoot.createSubfolder(folderName);
        usbInitialized = true;
        usbStorage.unmount();
        #ifndef LogToUSBDrive
            Serial.println("USB initialized and folder created");    
        #endif

    }

    if (usbStorage.begin()) {
        UFile logFile = backupFolder.createFile("pool_controller_log.txt", FileMode::APPEND);
        String timestamp = getLocalTime();  // Using existing getLocalTime() function
        String logEntry = timestamp + " - " + message + "\n";
        
        logFile.write(logEntry);
        logFile.close();
        usbStorage.unmount();

        #ifndef LogToUSBDrive
            Serial.println("Log entry written: " + logEntry);    
        #endif

    } else {
        #ifndef LogToUSBDrive
            Serial.println("Failed to open USB storage for writing");    
        #endif

    }
}

void connectionCallback() {
    usbAvailable = true;
    //Arduino_UnifiedStorage::debugPrint("- USB device connected!");
    usbStorage.removeOnConnectCallback();
}

void disconnectionCallback() {
    usbAvailable = false;
    //Arduino_UnifiedStorage::debugPrint("- USB device disconnected!");
    usbStorage.onConnect(connectionCallback);
}
#endif

#pragma endregion