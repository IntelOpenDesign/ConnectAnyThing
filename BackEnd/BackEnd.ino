/*

*** ConnectAnyThing ***

Intel - Intel Labs / User eXperience Research

Carlos Montesinos <carlos.montesinos@intel.com>
Lucas Ainsworth <lucas.b.ainsworth@intel.com>

-------

This code was based on the LYT project: https://github.com/secondstory/LYT
Copyright (c) 2013 - Philippe Laulheret, Second Story [http://www.secondstory.com]
 
This code is licensed under MIT.
*/

// If debbuging declare
#define DEBUG_CAT

#include <Dhcp.h>
#include <Dns.h>
#include <Ethernet.h>

#include <libwebsockets.h>

#include <aJSON.h>

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <getopt.h>
#include <string.h>
#include <sys/time.h>
#include <assert.h>
#include <syslog.h>
#include <signal.h>

#include <trace.h>
#define MY_TRACE_PREFIX "cat"

int led = 13; 
boolean currentLED = false;
int sensor1 = 0;
int sensor1Pin = A0;    // select the input pin for the potentiometer

// JSON specifics
aJsonStream serial_stream(&Serial);

// Web sockets
#define WEB_SOCKET_BUFFER_SIZE 30000

// HW declaration
#define TOTAL_NUM_Dx 14 
#define TOTAL_NUM_Px 12
boolean g_abD[TOTAL_NUM_Dx];
int g_aiP[TOTAL_NUM_Px];

#define PIN_LABEL_SIZE 25
#define TOTAL_NUM_OF_PINS 20
#define NUM_OF_ANALOG_PINS 6
#define NUM_OF_DIGITAL_PINS 14

#define ANALOG_OUT_MAX_VALUE 255
#define ANALOG_IN_MAX_VALUE 1024 //4096

#define DIGITAL_VOLTAGE_THRESHOLD  0.5

#define TOTAL_NUM_OF_PAST_VALUES  1000 // Filtering buffer

#define MESSAGES_PROCESSED_TOTAL_SIZE 1000
#define PROCESSED_MESSAGE_ID_MAX_SIZE 100
#define MAX_N_CLIENTS 100

#define LOCAL_BUFFER_SIZE 1024

class MessageManager
{
  typedef struct _Msg_Id {
    char id[PROCESSED_MESSAGE_ID_MAX_SIZE];
    int count;
  }
  Msg_Id;
  
public:
  Msg_Id m_MessagesProcessed[MESSAGES_PROCESSED_TOTAL_SIZE];
 
  MessageManager()
  {
    for(int i=0; i<MESSAGES_PROCESSED_TOTAL_SIZE; i++)
    {
      *m_MessagesProcessed[i].id = NULL;
      m_MessagesProcessed[i].count = 0;
    }
  }
  
  ~MessageManager(){}

  void newProcessedMsg(char* _sId)
  {
    int index = -1;
    for(int i=0; i<MESSAGES_PROCESSED_TOTAL_SIZE; i++)
    {
      if (*m_MessagesProcessed[i].id == NULL) {
        index = i;
        break; 
      }
    }
    if (index < 0) {
      //Serial.print("ERROR! The MessageManager's array of messages is ALL FULL. Make it bigger. The current size is "); Serial.println(MESSAGES_PROCESSED_TOTAL_SIZE);
      return;
    }
    
    sprintf(m_MessagesProcessed[index].id,"%s",_sId);
  }
  
  void sent()
  {
    for (int i = 0; i<MESSAGES_PROCESSED_TOTAL_SIZE; i++)
    {
      if (*m_MessagesProcessed[i].id != NULL) {
        m_MessagesProcessed[i].count += 1;
        if (m_MessagesProcessed[i].count > MAX_N_CLIENTS) {
          *m_MessagesProcessed[i].id = NULL;
          m_MessagesProcessed[i].count = 0;
        } 
      }
    }
  }
   
};

MessageManager g_oMessageManager;

float g_afFilterDeltas[] = {1,0.8,0.6,0.4,0.2,0.1,0.075,0.050,0.025,0.010};

typedef struct Pin {
  char label[PIN_LABEL_SIZE];
  int is_analog;
  int is_input;
  float input_min;
  float input_max;
//  float sensitivity;
  int is_inverted;
  int is_visible;
  float value;
  int is_timer_on;
  float timer_value;
  unsigned long timer_start_time;
  int damping;
  int prev_damping;
  boolean connections[TOTAL_NUM_OF_PINS];
//  float past_values[TOTAL_NUM_OF_PAST_VALUES];
  float prev_values;
} 
Pin;

Pin g_aPins[TOTAL_NUM_OF_PINS];
#define SSID_MAX_LENGTH 32
char g_sSsid[SSID_MAX_LENGTH];

// Configuration file
#define CONFIG_FILE_MAX_SIZE WEB_SOCKET_BUFFER_SIZE
#define CONFIG_FILE_FULL_PATH "/home/root/hardware.conf"
#define HOSTAPD_CONFIG_FILE_FULL_PATH "/etc/hostapd/hostapd_test.conf"
//#define TEMP_CONFIG_FILE_FULL_PATH "/etc/hostapd/hostapd_tmp.conf"
#define TEMP_CONFIG_FILE_FULL_PATH "/etc/hostapd/hostapdTest.conf"

// Scripts
#define START_ACCESS_POINT_SCRIPT_FULL_PATH "/home/root/startAP"

// Serial Interface
int g_iByte = 0;
int g_iNewCode = 0;

#define UP 119 // w
#define DOWN 115 // s
#define LEFT 97 // a
#define RIGHT 100 // d
#define SPACE 32 // space 


static struct option options[] = {

  { 
    NULL, 0, 0, 0                                           }
};


//This part of the code is inspired by the stock example coming with libsockets.
// Check it for more details.
 

int force_exit = 0;
enum lyt_protocols {
  PROTOCOL_HTTP = 0,
  PROTOCOL_LYT,
  PROTOCOL_COUNT
};

#define LOCAL_RESOURCE_PATH "/home/root/srv/"

struct serveable {
  const char *urlpath;
  const char *mimetype;
}; 

static const struct serveable whitelist[] = {
  { 
    "/favicon.ico", "image/x-icon"                                           }
  ,
  { 
    "/static/css/app.css", "text/css"                                           }
  ,
  { 
    "/static/css/bootstrap.min.css", "text/css"                                           }
  ,
  { 
    "/static/css/play_mode.css", "text/css"                                           }
  ,
  { 
    "/static/fonts/droid-sans/DroidSans.ttf", "application/x-font-ttf"                                           }
  ,
  { 
    "/static/fonts/droid-sans/DroidSans-Bold.ttf", "application/x-font-ttf"                                           }
  ,
  { 
    "/static/fonts/NeoSansIntel/NeoSansIntel.ttf", "application/x-font-ttf"                                           }
  ,
  { 
    "/static/fonts/NeoSansIntel/NeoSansIntel-Italic.ttf", "application/x-font-ttf"                                           }
  ,
  { 
    "/static/fonts/NeoSansIntel/NeoSansIntel-Light.ttf", "application/x-font-ttf"                                           }
  ,
  { 
    "/static/fonts/NeoSansIntel/NeoSansIntel-LightItalic.ttf", "application/x-font-ttf"                                           }
  ,
  { 
    "/static/fonts/NeoSansIntel/NeoSansIntel-Medium.ttf", "application/x-font-ttf"                                           }
  ,
  { 
    "/static/fonts/NeoSansIntel/NeoSansIntel-MediumItalic.ttf", "application/x-font-ttf"                                           }
  ,
  { 
    "/static/fonts/glyphicons-halflings-regular.ttf", "application/x-font-ttf"                                           }
  ,  
  { 
    "/static/img/loading.gif", "image/gif"                                           }
  ,
  { 
    "/static/img/loading_inverted.gif", "image/gif"                                           }
  ,
  { 
    "/static/img/loading_old.gif", "image/gif"                                           }
  ,
  { 
    "/static/js/angular.min.js", "application/javascript"                                           }
  ,
  { 
    "/static/js/angular-route.min.js", "application/javascript"                                           }
  ,
  { 
    "/static/js/app.js", "application/javascript"                                           }
  ,
  { 
    "/static/js/d3.min.js", "application/javascript"                                           }
  ,
  { 
    "/static/js/jquery.min.js", "application/javascript"                                           }
  ,
  { 
    "/static/js/underscore-min.js", "application/javascript"                                           }
  ,
  { 
    "/templates/connect.html", "text/html"                                           }
  ,
  { 
    "/templates/pin.html", "text/html"                                           }
  ,
  { 
    "/templates/pin_button.html", "text/html"                                           }
  ,
  { 
    "/templates/pin_settings.html", "text/html"                                           }
  ,
  { 
    "/templates/pin_stub.html", "text/html"                                           }
  ,
  { 
    "/templates/play.html", "text/html"                                           }
  ,
  // last one is the default served if no match
  { 
    "/index.html", "text/html"                                           }
  ,
};


// this protocol server (always the first one) just knows how to do HTTP 


//This callback is called when the browser is refreshed (an HTTP call is performed).
// Here we send the files to the browser.
static int callback_http(struct libwebsocket_context *context,
struct libwebsocket *wsi,
enum libwebsocket_callback_reasons reason, void *user,
void *in, size_t len)
{
  
  //     Serial.println("callback_http()");
  // WE ARE ALWAYS HITTING THIS POINT

//  char buf[256];
  char buf[LOCAL_BUFFER_SIZE];
  memset(buf,'\0',LOCAL_BUFFER_SIZE);
  int n;

  switch (reason) {

  case LWS_CALLBACK_HTTP:
//    lwsl_notice("LWS_CALLBACK_HTTP");
       
    #ifdef DEBUG_CAT
      trace_info("%s(): LWS_CALLBACK_HTTP: IN", __func__);
    #endif     
   
    for (n = 0; n < (sizeof(whitelist) / sizeof(whitelist[0]) - 1); n++)
      if (in && strcmp((const char *)in, whitelist[n].urlpath) == 0)
        break;

    sprintf(buf, LOCAL_RESOURCE_PATH"%s", whitelist[n].urlpath);

    if (libwebsockets_serve_http_file(context, wsi, buf, whitelist[n].mimetype))
      return 1; // through completion or error, close the socket 

     ///////////
     // notice that the sending of the file completes asynchronously,
     // we'll get a LWS_CALLBACK_HTTP_FILE_COMPLETION callback when
     // it's done
     ///////////     

    #ifdef DEBUG_CAT
      trace_info("%s(): LWS_CALLBACK_HTTP: OUT", __func__);
    #endif     

    break;

  case LWS_CALLBACK_HTTP_FILE_COMPLETION:

//    lwsl_notice("LWS_FILE_COMPLETION");

    #ifdef DEBUG_CAT
      trace_info("%s(): LWS_FILE_COMPLETION", __func__);
    #endif     
    
    return 1;


  default:

    #ifdef DEBUG_CAT
      trace_info("%s(): default", __func__);
    #endif     
 
    break;
  }

  return 0;
}

// Testing
//char pcTestBuffer[512] = "{\"status\":OK,\"pins\":{\"14\":{\"label\":\"A0\",\"is_analog\":\"true\",\"is_input\":\"true\",\"value\":\"0.5\"},\"3\":{\"label\":\"PWM3\",\"is_analog\":\"true\",\"is_input\":\"false\",\"value\":\"0.0\"}},\"connections\":[{\"source\":\"14\",\"target\":\"3\"}]}\"";

// CAT protocol
static int
callback_cat_protocol(struct libwebsocket_context *context,
struct libwebsocket *wsi,
enum libwebsocket_callback_reasons reason,
void *user,
void *in, size_t len)
{

  //Serial.println("callback_cat_protocol()");
  // WE ARE ALWAYS HITTING THIS POINT

  int iNumBytes = -1;

  switch (reason)
  {

  case LWS_CALLBACK_SERVER_WRITEABLE:

    #ifdef DEBUG_CAT
      trace_info("%s(): LWS_CALLBACK_SERVER_WRITEABLE: IN", __func__);
    #endif     

    // **********************************
    // Send Galileo's HW state to ALL clients
    // This function is continuesly called 
    // **********************************
    iNumBytes = sendStatusToWebsiteNew(wsi);
    if (iNumBytes < 0) 
    {
      lwsl_err("ERROR %d writing to socket\n", iNumBytes);
      return 1;
    }

    #ifdef DEBUG_CAT
      trace_info("%s(): LWS_CALLBACK_SERVER_WRITEABLE: OUT", __func__);
    #endif     

    break;

  case LWS_CALLBACK_RECEIVE:

    #ifdef DEBUG_CAT
      trace_info("%s(): LWS_CALLBACK_RECEIVE: IN", __func__);
    #endif     

    // **********************************
    // Process Data Receieved from the Website
    // Update Galileo's HW    
    // **********************************    
    processMessage((char*) in);


    #ifdef DEBUG_CAT
      trace_info("%s(): LWS_CALLBACK_RECEIVE: OUT", __func__);
    #endif     


    break;

  default:
  
    #ifdef DEBUG_CAT
      trace_info("%s(): default", __func__);
    #endif     
    
    break;
  }

  return 0;
}

//-----------------------------------------------------------
// Init hardware using the configuration file
//------------------------------------------------------------
void initBoardStateFromFile(char* _sFullFilePath) {
  
  // Open File
  FILE *fp;
  char sJsonFile[CONFIG_FILE_MAX_SIZE];
 
  // Read file
  fp = fopen(_sFullFilePath, "r"); 
  fgets(sJsonFile, CONFIG_FILE_MAX_SIZE, fp);
  fclose(fp);
  
  /*
  #ifdef DEBUG_CAT
    trace_info("%s() -> Read file", __func__);
  #endif
  */
  Serial.print("Read File");
  
  // Parse file
  aJsonObject *poMsg = aJson.parse(sJsonFile);
    
  // Check if the message has pin data
  aJsonObject *pJsonPins = aJson.getObjectItem(poMsg, "pins");
  if( pJsonPins )  // Check if there is pin info
    procPinsMsg( pJsonPins );
  
  aJsonObject *pJsonConnections = aJson.getObjectItem(poMsg, "connections");
  if( pJsonConnections )  // Check if there is pin info
    procConnMsg( pJsonConnections );        
  
  aJsonObject *pJsonSsid = aJson.getObjectItem(poMsg, "ssid");      
  if( pJsonSsid )  // Check if there is connection info
    procSsidMsg( pJsonSsid );  

  // Set the HW state
  updateBoardState();

  Serial.print("Done updating code");

/*
  #ifdef DEBUG_CAT
    trace_info("%s() -> Set HW from file", __func__);
  #endif
*/

}

//-----------------------------------------------------------
// Process recieved message from the webpage here.... 
//------------------------------------------------------------
void procWebMsg(char* _in, size_t _len) {

  String sMsg(_in);

  sMsg.toLowerCase();

  //Serial.print("input: "); 
  //Serial.println(sMsg);
  //Serial.print("_len: "); 
  //Serial.println(String(_len));

  // Parse incomming message
  if(  sMsg.startsWith("d") && _len <= 3 )
  {
    // Check if we are setting a Dx pin
    int iPin = sMsg.substring(1,sMsg.length()).toInt();
    //Serial.print("Pin: ");
    //Serial.println(String(iPin));
    g_abD[iPin] = ~g_abD[iPin]; // Toggle state
    digitalWrite(iPin, g_abD[iPin]); // Set state
  }
  else if(  sMsg.startsWith("p") )
  {
    // Check if we are setting a Px pin 

    // Get pin number and value
    int iValue = 0;
    int iPin = 0;

    if( sMsg.charAt(2) == ',' )
    {
      iPin = sMsg.substring(1,2).toInt();
      iValue = sMsg.substring(3,sMsg.length()).toInt();
    }
    else if( sMsg.charAt(3) == ',' )
    {
      iPin = sMsg.substring(1,3).toInt();
      iValue = sMsg.substring(4,sMsg.length()).toInt();      
    }
    else
    {
      //Serial.print("Error. Unrecongnized message: ");   
      //Serial.println(sMsg);
    }

    // Set pin value
    //Serial.print("Pin: ");
    //Serial.println(String(iPin));
    //Serial.print("Value: ");
    //Serial.println(String(iValue));
    g_aiP[iPin] = iValue; // Toggle state
    analogWrite(iPin, g_aiP[iPin]); // Set state
  }
  else
  {
    //Serial.print("Error. Unrecongnized message: ");   
    //Serial.println(sMsg);
  }
}

// **********************************
// Send pin status to the Website    
// **********************************
int  sendStatusToWebsiteNew(struct libwebsocket *wsi)
{

  int n = 0;

  unsigned char buf[LWS_SEND_BUFFER_PRE_PADDING + WEB_SOCKET_BUFFER_SIZE + LWS_SEND_BUFFER_POST_PADDING];
  memset(buf,'\0',LWS_SEND_BUFFER_PRE_PADDING + WEB_SOCKET_BUFFER_SIZE + LWS_SEND_BUFFER_POST_PADDING);
  unsigned char *p = &buf[LWS_SEND_BUFFER_PRE_PADDING];
  
  updateBoardState();
  
  aJsonObject *msg = getJsonBoardState();  
  
  aJsonStringStream stringStream(NULL, (char *)p, WEB_SOCKET_BUFFER_SIZE);
  aJson.print(msg, &stringStream);
  String sTempString((char *)p);
    
  n = libwebsocket_write(wsi, p, sTempString.length(), LWS_WRITE_TEXT);

  g_oMessageManager.sent(); // Updating Message Manager
   
  aJson.deleteItem(msg);


  return n;
}

// list of supported protocols and callbacks 
static struct libwebsocket_protocols protocols[] = {
  // first protocol must always be HTTP handler, to serve webpage 

  {
    "http-only",		// name */
    callback_http,		// callback */
    0,			// per_session_data_size */
    0,			// max frame size / rx buffer */
  }
  , // manages data in and data out from and to the website
  {
    "hardware-state-protocol",
    callback_cat_protocol,
    0,
    128,
  }
  ,

  { 
    NULL, NULL, 0, 0                                           } // terminator */
};

void sighandler(int sig)
{
  force_exit = 1;
}

int initWebsocket()
{
  int n = 0;
  struct libwebsocket_context *context;
  int opts = LWS_SERVER_OPTION_SKIP_SERVER_CANONICAL_NAME;
  char interface_name[128] = "wlan0";
  const char *iface = NULL;

  unsigned int oldus = 0;
  struct lws_context_creation_info info;

  int debug_level = 7;

  memset(&info, 0, sizeof info);
  info.port = 80;

  signal(SIGINT, sighandler);
  int syslog_options =  LOG_PID | LOG_PERROR;

  // we will only try to log things according to our debug_level */
  setlogmask(LOG_UPTO (LOG_DEBUG));
  openlog("lwsts", syslog_options, LOG_DAEMON);

  // tell the library what debug level to emit and to send it to syslog */
  lws_set_log_level(debug_level, lwsl_emit_syslog);

  info.iface = iface;
  info.protocols = protocols;

  info.extensions = libwebsocket_get_internal_extensions();

  info.ssl_cert_filepath = NULL;
  info.ssl_private_key_filepath = NULL;

  info.gid = -1;
  info.uid = -1;
  info.options = opts;

  context = libwebsocket_create_context(&info);
  if (context == NULL) {
    lwsl_err("libwebsocket init failed\n");
    return -1;
  }

  n = 0;
  while (n >= 0 && !force_exit) {
    struct timeval tv;

    gettimeofday(&tv, NULL);

    if (((unsigned int)tv.tv_usec - oldus) > 50000) {
      libwebsocket_callback_on_writable_all_protocol(&protocols[PROTOCOL_LYT]);
      oldus = tv.tv_usec;

    }
    n = libwebsocket_service(context, 50);

    loop(); // call  Arduino loop as we have taken over the execution flow :[

  }
  libwebsocket_context_destroy(context);
  closelog();
  return 0;
}

//////////////////////////////////////////////////////
// Initialize the HW state datastructure
//////////////////////////////////////////////////////
void initBoardState()
{
   
  // Clear SSID string
  memset(g_sSsid, '\0', SSID_MAX_LENGTH);  
  
  // Initialize all pins with no lable and 0.0 value
  for(int i=0; i<TOTAL_NUM_OF_PINS; i++)
  {
//    memset(g_aPins[i].label, '\0', sizeof(g_aPins[i].label));
    memset(g_aPins[i].label, '\0', PIN_LABEL_SIZE);  
    strcpy(g_aPins[i].label,"");

    g_aPins[i].input_min = 0.0;
    g_aPins[i].input_max = 1.0;
    g_aPins[i].is_inverted = false;
    g_aPins[i].is_visible = false;
    g_aPins[i].value = 0.0;
    
 //   for(int j=0; j<TOTAL_NUM_OF_PAST_VALUES; j++)
  //    g_aPins[i].past_values[j] = 0.0;
    g_aPins[i].prev_values = 0.0;
    
    g_aPins[i].is_timer_on = false;
    g_aPins[i].timer_value = 0.0;
    g_aPins[i].timer_start_time = 0;
    g_aPins[i].damping = 0;
    g_aPins[i].prev_damping = 0;
    
    // Set pin connections
//    memset(g_aPins[i].connections, '\0', sizeof(g_aPins[i].connections));
    memset(g_aPins[i].connections, '\0', TOTAL_NUM_OF_PINS);
    for(int j=0; j<TOTAL_NUM_OF_PINS; j++)
    {
      if( i == j ) // Pins are always connected to themselves
        g_aPins[i].connections[j] = true;
      else
        g_aPins[i].connections[j] = false;
    }

    // Initialize analog pins 3,5,6,9,10,11,A0,A1,A2,A3,A4,A5
    if( i==3 || i==5 || i==6 || i==9 || i==10 || i==11 || i==14 || i==15 || i==16 || i==17 || i==18 || i==19 )
      g_aPins[i].is_analog = true;
    else  
      g_aPins[i].is_analog = false;   
       
    // Initialize digital pins as outputs
    if( i < NUM_OF_DIGITAL_PINS ) 
    {
      g_aPins[i].is_input = false;
      pinMode(i, OUTPUT);
    }
    else // Setting A0-A5 pins as inputs
    {
      g_aPins[i].is_input = true;
      pinMode(i, INPUT);
    }        
  }

  // Set the HW state
  updateBoardState();

}

//////////////////////////////////////////////////////
// Update the HW based on the HW state data structure
//////////////////////////////////////////////////////
void updateBoardState()
{
  // Update input pins first
  for(int i=0; i<TOTAL_NUM_OF_PINS; i++)
  {
    if( g_aPins[i].is_input ) // Process input pins
    {
      if( g_aPins[i].is_analog ) // Process analog pins
      {
        if( g_aPins[i].is_inverted )
          g_aPins[i].value = 1.0 - analogRead(i)/float(ANALOG_IN_MAX_VALUE);
        else
          g_aPins[i].value = analogRead(i)/float(ANALOG_IN_MAX_VALUE);
      }
      else // Process digital pins
      {
        if( g_aPins[i].is_inverted )
          g_aPins[i].value = 1.0 - digitalRead(i);
        else
          g_aPins[i].value = digitalRead(i);       
      }
    }
  }

  // Update outputs pins
  for(int i=0; i<TOTAL_NUM_OF_PINS; i++)
  {
    if( !g_aPins[i].is_input ) // Process output pins
    {
      if( g_aPins[i].is_analog ) // Process analog pins      
        analogWrite(i, getTotalPinValue(i)*ANALOG_OUT_MAX_VALUE );
      else // Process digital pins     
        digitalWrite(i, getTotalPinValue(i) );
    }
  }
  
}

////////////////////////////////////////////////////////////////////////////
// If an output pin has connections, return the sum of all its connections
////////////////////////////////////////////////////////////////////////////
float getTotalPinValue(int _iOutPinNum)
{
  float fPinValSum = 0;
  int iConnCount = 1; // Pins are always connected to themselves

  for(int i=0; i<TOTAL_NUM_OF_PINS; i++)
  {
    // Checking what pins are connected to iPinNum
    if( _iOutPinNum != i) // Ignore the value set to the pin and only use connected pin values
    {
      if(g_aPins[_iOutPinNum].connections[i])
      {
        iConnCount++;

        //fPinValSum += (g_aPins[i].value - g_aPins[i].input_min)/(g_aPins[i].input_max - g_aPins[i].input_min);  // Adding the Max/Min formula
        fPinValSum += getScaledPinValue(i);  // Adding the Max/Min formula

       if( !g_aPins[_iOutPinNum].is_analog )
       {
         if( fPinValSum >= DIGITAL_VOLTAGE_THRESHOLD) // Digital threshold
           fPinValSum = 1.0;
         else
           fPinValSum = 0.0;
       }
      }    
    } 
  }
   
  if(fPinValSum > 1.0)
    fPinValSum = 1.0;
  
  if( 1 == iConnCount ) // No other connections found besides itself
    fPinValSum = g_aPins[_iOutPinNum].value;
  else
    g_aPins[_iOutPinNum].value = fPinValSum;
  
  //Serial.print("Analog Pin #: "); Serial.print(_iOutPinNum); Serial.print(" Total Value: "); Serial.println(fPinValSum);

  // Set pin value to zero if timer expired
  if( g_aPins[_iOutPinNum].is_timer_on ) 
  {
//    Serial.print("Pin ");Serial.print(_iOutPinNum);Serial.print(" timer ON");
//    Serial.print(" Start Time: ");Serial.println(g_aPins[_iOutPinNum].timer_start_time);
    if( (millis() - g_aPins[_iOutPinNum].timer_start_time) >= (g_aPins[_iOutPinNum].timer_value-1)*1000 ) // Timer expired
    {
//      Serial.println("EXPIRED...!!!");
      fPinValSum = 0.0;
      g_aPins[_iOutPinNum].value = fPinValSum;      
    }
  }
  
  return fPinValSum;
}

float getScaledPinValue(int _iInPinNum)
{
  //return (g_aPins[_iInPinNum].value - g_aPins[_iInPinNum].input_min)/(g_aPins[_iInPinNum].input_max - g_aPins[_iInPinNum].input_min);   
  return (getFilteredPinValue(_iInPinNum) - g_aPins[_iInPinNum].input_min)/(g_aPins[_iInPinNum].input_max - g_aPins[_iInPinNum].input_min);   
}

float getFilteredPinValue(int _iInPinNum)
{
  static float fFiltered = 0.0;

//  float fDelta = 1.0/pow(10,g_aPins[_iInPinNum].damping);
//  float fDelta = 1.0 - 0.99/9.0*float(g_aPins[_iInPinNum].damping);
  float fDelta = g_afFilterDeltas[g_aPins[_iInPinNum].damping];

  //Serial.print("fDelta: "); Serial.println(fDelta);    

  if( fFiltered < g_aPins[_iInPinNum].value )
  {
    fFiltered += fDelta;
    if(fFiltered>g_aPins[_iInPinNum].value)
      fFiltered = g_aPins[_iInPinNum].value;
  }
  else if( fFiltered > g_aPins[_iInPinNum].value )  
  {
    fFiltered -= fDelta;
    if( fFiltered < g_aPins[_iInPinNum].value)
      fFiltered = g_aPins[_iInPinNum].value;
  }
  else
  {
    fFiltered = g_aPins[_iInPinNum].value;
  }

//Serial.print("fFiltered: "); //Serial.println(fFiltered);  
//Serial.print("Value: "); Serial.println(g_aPins[_iInPinNum].value);  

  return fFiltered; 
}

////////////////////////////////////////////////
// Get the board state and return a JSON object
////////////////////////////////////////////////
int g_iMsgCount = 0; // debug

aJsonObject* getJsonBoardState()
{

   g_iMsgCount++; // debug
    
//  char buf[LOCAL_BUFFER_SIZE];
//  memset(buf,'\0',LOCAL_BUFFER_SIZE);
  
  aJsonObject* poJsonBoardState = aJson.createObject();

  // Creating status JSON object
  aJsonObject* poStatus = aJson.createObject();

  // Creating pin JSON objects
  aJsonObject* poPins = aJson.createObject();
  aJsonObject* apoPin[TOTAL_NUM_OF_PINS];
  for(int i=0; i<TOTAL_NUM_OF_PINS; i++)
  {
    apoPin[i] = aJson.createObject();
  }

  // Creating connection JSON object
  aJsonObject* paConnections = aJson.createArray();

  // Creating message IDs JSON object
  aJsonObject* paMsgIds = aJson.createArray();

  // Create STATUS
  poStatus = aJson.createItem("OK");

  // Create PINS
  int iaPinConnects[TOTAL_NUM_OF_PINS];
  char caPinNumBuffer[LOCAL_BUFFER_SIZE];
  memset(caPinNumBuffer,'\0',LOCAL_BUFFER_SIZE);
  
  for(int i=0; i<TOTAL_NUM_OF_PINS ;i++)
  {
    aJson.addItemToObject(apoPin[i],"label", aJson.createItem( g_aPins[i].label ) );

    // Populate the pin's type    
    if( g_aPins[i].is_analog )
    {
      aJson.addItemToObject(apoPin[i],"is_analog", aJson.createTrue() );
    }
    else
    {
      aJson.addItemToObject(apoPin[i],"is_analog", aJson.createFalse() );
    }

    // Populate the pin's direction
    if( g_aPins[i].is_input )
    {
      aJson.addItemToObject(apoPin[i],"is_input", aJson.createTrue() );
    }
    else
    {
      aJson.addItemToObject(apoPin[i],"is_input", aJson.createFalse() );
    }

    aJson.addItemToObject(apoPin[i],"input_min", aJson.createItem( g_aPins[i].input_min ) );
    
//    Serial.print("Creating JSON Str. Pin: "); Serial.print(i); Serial.print(" input_max: "); Serial.println(g_aPins[i].input_max);
    aJson.addItemToObject(apoPin[i],"input_max", aJson.createItem( g_aPins[i].input_max ) );

    // Populate sensitivity
 //   aJson.addItemToObject(apoPin[i],"sensitivity", aJson.createItem( g_aPins[i].sensitivity ) );
    
    // Populate pin's inversion
    if( g_aPins[i].is_inverted )
    {
      aJson.addItemToObject(apoPin[i],"is_inverted", aJson.createTrue() );
    }
    else
    {
      aJson.addItemToObject(apoPin[i],"is_inverted", aJson.createFalse() );
    }
    
    // Populate pin's visibility
    if( g_aPins[i].is_visible )
    {
      aJson.addItemToObject(apoPin[i],"is_visible", aJson.createTrue() );
    }
    else
    {
      aJson.addItemToObject(apoPin[i],"is_visible", aJson.createFalse() );
    }
    
    aJson.addItemToObject(apoPin[i],"value", aJson.createItem( g_aPins[i].value ) );

    // Populate pin's timer state
    if( g_aPins[i].is_timer_on )
    {
      aJson.addItemToObject(apoPin[i],"is_timer_on", aJson.createTrue() );
    }
    else
    {
      aJson.addItemToObject(apoPin[i],"is_timer_on", aJson.createFalse() );
    }

    aJson.addItemToObject(apoPin[i],"timer_value", aJson.createItem( g_aPins[i].timer_value ) );
    
    aJson.addItemToObject(apoPin[i],"damping", aJson.createItem( g_aPins[i].damping ) );

    // Push to JSON structure
    sprintf(caPinNumBuffer,"%d",i);
    aJson.addItemToObject(poPins,caPinNumBuffer,apoPin[i]);

  }
   
  // Create CONNECTIONS
  for(int i=0; i<TOTAL_NUM_OF_PINS ;i++)
  {
    for(int j=0; j<TOTAL_NUM_OF_PINS ;j++)
    {
      if(g_aPins[i].connections[j] && i !=j )
      {
        aJsonObject* poConnObject = aJson.createObject();
        sprintf(caPinNumBuffer,"%d",j);      
        aJson.addItemToObject(poConnObject,"source",aJson.createItem(caPinNumBuffer));
        sprintf(caPinNumBuffer,"%d",i);        
        aJson.addItemToObject(poConnObject,"target",aJson.createItem(caPinNumBuffer));
        aJson.addItemToArray(paConnections,poConnObject);
      }
    }
  }
  
  // Create MSG IDs 
  for(int i=0; i<MESSAGES_PROCESSED_TOTAL_SIZE; i++)
  {
    if( *g_oMessageManager.m_MessagesProcessed[i].id != NULL )
    {
      aJson.addItemToArray(paMsgIds,aJson.createItem(g_oMessageManager.m_MessagesProcessed[i].id));
    }
  }
  
  ////// debug
    aJson.addItemToObject(poJsonBoardState,"Msg_Count", aJson.createItem( g_iMsgCount ) );
  //////

  // Push to JSON object
  aJson.addItemToObject(poJsonBoardState,"status",poStatus);  
  aJson.addItemToObject(poJsonBoardState,"pins",poPins);
//  aJson.addItemToObject(poJsonBoardState,"connections",poConnections);
  aJson.addItemToObject(poJsonBoardState,"connections",paConnections);
  aJson.addItemToObject(poJsonBoardState,"message_ids_processed",paMsgIds);
  

  return poJsonBoardState;

}

void processMessage(char *_acMsg)
{
  
//  Serial.print("Msg recvd: ");
//  Serial.println(_acMsg);

  debug_serial_print("Msg received. Size: ");
//  debug_serial_println(sizeof(_acMsg));
    #ifdef DEBUG_CAT
      Serial.println(strlen(_acMsg)); // debug      
    #endif
  debug_serial_print(_acMsg);
  
  // Ignore messages bigger than we can handle
  if( sizeof(_acMsg) >= WEB_SOCKET_BUFFER_SIZE )
    return;

  aJsonObject *poMsg = aJson.parse(_acMsg);

  if(poMsg != NULL)
  {
    aJsonObject *poMsgId = aJson.getObjectItem(poMsg, "message_id");
    if (!poMsgId) {
      //Serial.println("ERROR: Invalid Msg Id.");
      return;
    }
    else
    {
      g_oMessageManager.newProcessedMsg(poMsgId->valuestring);
    }
    
    aJsonObject *poStatus = aJson.getObjectItem(poMsg, "status");
    if (!poStatus) {
      //Serial.println("ERROR: No Status data.");
      return;
    }
    else if ( strncmp(poStatus->valuestring,"OK",2) == 0 )
    {
      // Process if status is OK
      //Serial.println("STATUS: OK");

      // Check if the message has pin data
      aJsonObject *pJsonPins = aJson.getObjectItem(poMsg, "pins");
      if( pJsonPins )  // Check if there is pin info
        procPinsMsg( pJsonPins );

      aJsonObject *pJsonConnections = aJson.getObjectItem(poMsg, "connections");
      if( pJsonConnections )  // Check if there is pin info
        procConnMsg( pJsonConnections );        

      aJsonObject *pJsonSsid = aJson.getObjectItem(poMsg, "ssid");      
      if( pJsonSsid )  // Check if there is connection info
        procSsidMsg( pJsonSsid );  
         
    }
    else if ( strncmp(poStatus->valuestring,"ERROR",5) == 0 )
    {
      // Process if status is ERROR
      //Serial.println("STATUS: ERROR");
    }
    else
    {
      //Serial.println("ERROR: Unknown status");      
    } 
  }
  else
  {
    //Serial.println("Client message is NULL");
  }

  aJson.deleteItem(poMsg);
}

///////////////////////////////
// Process SSID messages
///////////////////////////////
void procSsidMsg( aJsonObject *_pJsonSsid )
{
  aJsonObject *poSsid = aJson.getObjectItem(_pJsonSsid, "ssid");
  if (poSsid && String(poSsid->valuestring).length() <= SSID_MAX_LENGTH)
    sprintf(g_sSsid, "%s", poSsid->valuestring);
        
 }

///////////////////////////////
// Process pin value messages
///////////////////////////////
void procPinsMsg( aJsonObject *_pJsonPins )
{
//  Serial.println("Processing Pins");

  // Iterate all pins and check if we have data available
  for(int i=0; i<TOTAL_NUM_OF_PINS; i++)
  {
    char pinstr[10];
    snprintf(pinstr, sizeof(pinstr), "%d", i);    
    aJsonObject *poPinVals = aJson.getObjectItem(_pJsonPins, pinstr);
    if (poPinVals)
    {
      char sPinState[LOCAL_BUFFER_SIZE];

      aJsonObject *poLabel = aJson.getObjectItem(poPinVals, "label");
      if (poLabel)
        snprintf(g_aPins[i].label, PIN_LABEL_SIZE, "%s", poLabel->valuestring);

      aJsonObject *poIsAnalog = aJson.getObjectItem(poPinVals, "is_analog");
      if (poIsAnalog)
        g_aPins[i].is_analog = poIsAnalog->valuebool;        

      aJsonObject *poIsInput = aJson.getObjectItem(poPinVals, "is_input");
      if (poIsInput)
        g_aPins[i].is_input = poIsInput->valuebool;
        
      aJsonObject *poInputMin = aJson.getObjectItem(poPinVals, "input_min");
      if (poInputMin)
          g_aPins[i].input_min = getJsonFloat(poInputMin);    
//        g_aPins[i].input_min = poInputMin->valuefloat;
          
      aJsonObject *poInputMax = aJson.getObjectItem(poPinVals, "input_max");
//      Serial.print("Recieved JSON Str. Pin: "); Serial.print(i); Serial.print(" input_max: "); Serial.println(poInputMax->valuefloat);
      if (poInputMax)
          g_aPins[i].input_max = getJsonFloat(poInputMax);
//        g_aPins[i].input_max = poInputMax->valuefloat;

      aJsonObject *poIsInverted = aJson.getObjectItem(poPinVals, "is_inverted");
      if (poIsInverted)
        g_aPins[i].is_inverted = poIsInverted->valuebool;  

      aJsonObject *poIsVisible = aJson.getObjectItem(poPinVals, "is_visible");
      if (poIsVisible)
        g_aPins[i].is_visible = poIsVisible->valuebool;  

      aJsonObject *poValue = aJson.getObjectItem(poPinVals, "value");
      if (poValue)
          g_aPins[i].value = getJsonFloat(poValue);      
//        g_aPins[i].value = poValue->valuefloat;
     
      aJsonObject *poIsTimerOn = aJson.getObjectItem(poPinVals, "is_timer_on");
      if (poIsTimerOn)
      {
        g_aPins[i].is_timer_on = poIsTimerOn->valuebool; 
        if(g_aPins[i].is_timer_on)
          g_aPins[i].timer_start_time = millis();
      } 

      aJsonObject *poTimerValue = aJson.getObjectItem(poPinVals, "timer_value");
      if (poTimerValue)
          g_aPins[i].timer_value = getJsonFloat(poTimerValue);      
//        g_aPins[i].timer_value = poTimerValue->valuefloat;
        
      aJsonObject *poDamping = aJson.getObjectItem(poPinVals, "damping");
      if (poDamping){
        g_aPins[i].damping = poDamping->valueint;
        //Serial.print("poDamping->valueint: "); Serial.println(poDamping->valueint);
        //Serial.print("poDamping->valuefloat: "); Serial.println(poDamping->valuefloat);
       // Serial.print("poDamping->valuestring: "); Serial.println(poDamping->valuestring);
      }
    }    
  }
}

float getJsonFloat(aJsonObject * _poJsonObj)
{
 float fRet = 0.0; 
 switch(_poJsonObj->type)
 {
  case aJson_Int:
    fRet = float(_poJsonObj->valueint);
  break;  
  case aJson_Float:
    fRet = _poJsonObj->valuefloat;
  break;  
  default:
    // None 
  break;
 }
 
//  Serial.print("Type: ");Serial.print(_poJsonObj->type); Serial.print(" Int: ");Serial.print(_poJsonObj->valueint);Serial.print(" Float: ");Serial.println(_poJsonObj->valuefloat);
 
 return fRet;
}

///////////////////////////////
// Process connection messages
///////////////////////////////
void procConnMsg( aJsonObject *_pJsonConnections )
{
  int uiSourcePin = -1;
  int uiTargetPin = -1;
  unsigned char ucNumOfConns = aJson.getArraySize(_pJsonConnections);
  
  //Serial.print("Processing ");
  //Serial.print(String(ucNumOfConns));
  //Serial.println(" Connections");

  for(int i=0; i<ucNumOfConns; i++)
  {
   
    // Get item
    aJsonObject* poItem = aJson.getArrayItem(_pJsonConnections, i);
    if (!poItem)
      continue;
    
    // Get source
    aJsonObject* poSource = aJson.getObjectItem(poItem, "source");
    if (poSource)
    {
      uiSourcePin = atoi(poSource->valuestring);
      if( poSource->valueint < 0 || poSource->valueint <= TOTAL_NUM_OF_PINS)
      {
        //Serial.println("Source pin out of bounds");
        continue;
      }
    }
    else
    {
      //Serial.println("No source in connections");
      continue;
    }
    
    // Get target
    aJsonObject* poTarget = aJson.getObjectItem(poItem, "target");
    if (poTarget)
    {
        uiTargetPin = atoi(poTarget->valuestring);
       if( poTarget->valueint < 0 || poTarget->valueint <= TOTAL_NUM_OF_PINS)
      {
        //Serial.println("Target pin out of bounds");
        continue;
      }
    }
    else
    {
      //Serial.println("No target in connections");
      continue;
    }   
    
    // Get target
    aJsonObject* poConnect = aJson.getObjectItem(poItem, "connect");
    if (poConnect)
    {
      switch(poConnect->type)
      {
        case aJson_False:
        case aJson_True:
          g_aPins[uiTargetPin].connections[uiSourcePin] = poConnect->valuebool;
        break;
        default:
          //Serial.print("Error: 'Connect' member value is the wrong type: ");Serial.println((int)(poConnect->type));
          //Serial.print("Msg: ");Serial.println(poConnect->valuestring);
        break;        
      }
    }
    
 //   Serial.print("Source: ");        
 //   Serial.println(String(uiSourcePin));        
//    Serial.print("Target: "); 
//    Serial.println(String(uiTargetPin));   
//    Serial.print("Connect: "); 
//    Serial.println(String(uiTargetPin));   
    
  } 
}

/////////////////////////////////////////////////////////////////////////////////
// Arduino standard funtions
/////////////////////////////////////////////////////////////////////////////////
void setup()
{
  // put your setup code here, to run once:
  Serial.begin(9600);

  #ifdef DEBUG_CAT
    trace_info("-----------------------------------------------------------");  
    trace_info("%s(): Starting Backend", __func__);
  #endif
  
  //Serial.println("Starting WebServer");
//  system("/home/root/startAP");
  system(START_ACCESS_POINT_SCRIPT_FULL_PATH);

  // Initialize HW and JSON protocol code
  //Serial.println("Initilize Hardware");
  initBoardState();
//  initBoardStateFromFile(CONFIG_FILE_FULL_PATH);

  //Serial.println("Starting WebSocket");
  initWebsocket();  

}


int changeSsidName(char *_sSsidName)
{
  // Truncate Ssid if it too long
  char sSsidName[LOCAL_BUFFER_SIZE];
//  sprintf(sSsidName,"%s",_sSsidName);
  snprintf(sSsidName,SSID_MAX_LENGTH,"%s",_sSsidName);

// We need a buffer to read in data
  char      Buffer[LOCAL_BUFFER_SIZE];
  char      TempBuffer[LOCAL_BUFFER_SIZE];

  // Open the file for reading/write.
  FILE     *Input = fopen(HOSTAPD_CONFIG_FILE_FULL_PATH, "r"); // Read/write
  FILE     *Output = fopen(TEMP_CONFIG_FILE_FULL_PATH, "w"); // Read/write

  // Our find and replace arguments
  char     *Find = "ssid=";
  char    sSsidLine[LOCAL_BUFFER_SIZE];

  // Create file line    
  sprintf(sSsidLine,"ssid=%s\n", sSsidName);
  
  if(NULL == Input)
  {
      trace_info("%s(): %s file not found", __func__,HOSTAPD_CONFIG_FILE_FULL_PATH);
      return 1;
  }

  // For each line...
  while(NULL != fgets(Buffer, LOCAL_BUFFER_SIZE, Input))
  {
    char *Start = NULL;    // Where 'ssid=' was found in the line
    char *Line = Buffer; // Start at the beginning of the line, and after each match
     
    // Find match
    if( strstr(Line, Find) )
    {
      fputs(sSsidLine, Output);
    }
    else
    {
      fputs(Line, Output);      
    }
  }

  // Close our files
  fclose(Input);
  fclose(Output);
  
  rename(TEMP_CONFIG_FILE_FULL_PATH,HOSTAPD_CONFIG_FILE_FULL_PATH);
  
  return 0;
}

unsigned long last_print = 0;
int g_iCatNumber = 0;
int g_iChangeSsid = 1;

void loop()
{

  if (millis() - last_print > 1000) {

    ///////////////////////////////////////
    // Test code
    ///////////////////////////////////////
    
    getSerialCommand(); 
   
/*
    g_iCatNumber++;
    char sSsidName[100];
    sprintf(sSsidName, "CA3333333333333333333333333333344444444444444444444444444444444444444444444T%i", g_iCatNumber);
    changeSsidName(sSsidName);
 */
    //////////////////////////////////////////  

    last_print = millis();
  }
}

/////////////////////////////////////////////////////////////////////////////////
// Parce Serial Commands
/////////////////////////////////////////////////////////////////////////////////
void getSerialCommand() 
{
  if (Serial.available() > 0) {
    // get incoming byte:
    g_iByte = Serial.read();

    switch (g_iByte)
    {
    case UP:
      g_iNewCode = 0;      
      break;

    case DOWN:
    
        Serial.println("Changing SSID");
        changeSsidName("CAT1");
    
//        system("shutdown -r now");
        
    break;
    case LEFT:
    case RIGHT:
      g_iNewCode = 1;            
      break;
    }
  }
}

/////////////////////////////////////////////////////////////////////////////////
// Other tools
/////////////////////////////////////////////////////////////////////////////////
void debug_serial_print(char* _sString)
{
    #ifdef DEBUG_CAT
      Serial.print(_sString); // debug
    #endif
}

void debug_serial_println(char* _sString)
{
    #ifdef DEBUG_CAT
      Serial.println(_sString); // debug      
    #endif
}
