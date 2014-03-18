/*

 Copyright (c) 2013 - Philippe Laulheret, Second Story [http://www.secondstory.com]
 
 This code is licensed under MIT. 
 For more information visit  : https://github.com/secondstory/LYT
 
 */

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

int led = 13; 
boolean currentLED = false;
int sensor1 = 0;
int sensor1Pin = A0;    // select the input pin for the potentiometer

// JSON specifics
aJsonStream serial_stream(&Serial);

// Web sockets
#define WEB_SOCKET_BUFFER_SIZE 10000

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

//char** g_caaPinNames[20][PIN_LABEL_SIZE];

typedef struct Pin {
  char label[PIN_LABEL_SIZE];
  int is_analog;
  int is_input;
  float sensitivity;
  int is_inverted;
  int is_visible;
  float value;
  boolean connections[TOTAL_NUM_OF_PINS];
} 
Pin;

  /*
  {"status":<OK,ERROR>,  "pins":{"<0,1,…,13,A0,…,A5>":  {
  "label":"<label text>",  
	"is_analog":"<true,false>",  
	"is_input":"<true,false>", 
“sensitivity”:”<0.0,1.0>”, 
“is_inverted”:”<true,false>”, 
“is_visible”:”<true,false>”,
"value":"<0.0,1.0>",  },  ...,  }, 
"connections":[ {"source":"<0,1,…,13,A0,…,A5>","target":"<0,1,…,13,A0,…,A5>",”is_connected”:”<true,false>”},  ...,  ]  }
  */


Pin g_aPins[TOTAL_NUM_OF_PINS];

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

/*
This part of the code is inspired by the stock example coming with libsockets.
 Check it for more details.
 */

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
    "/static/css/jquery.mobile-1.4.1.css", "text/css"                                           }
  ,
  { 
    "/static/css/jquery.mobile-1.4.1.min.css", "text/css"                                           }
  ,
  { 
    "/static/fonts/droid-sans/DroidSans.ttf", "text/ttf"                                           }
  ,
  { 
    "/static/fonts/droid-sans/DroidSans-Bold.ttf", "text/ttf"                                           }
  ,
  { 
    "/static/fonts/NeoSansIntel/NeoSansIntel.ttf", "text/ttf"                                           }
  ,
  { 
    "/static/fonts/NeoSansIntel/NeoSansIntel-Italic.ttf", "text/ttf"                                           }
  ,
  { 
    "/static/fonts/NeoSansIntel/NeoSansIntel-Light.ttf", "text/ttf"                                           }
  ,
  { 
    "/static/fonts/NeoSansIntel/NeoSansIntel-LightItalic.ttf", "text/ttf"                                           }
  ,
  { 
    "/static/fonts/NeoSansIntel/NeoSansIntel-Medium.ttf", "text/ttf"                                           }
  ,
  { 
    "/static/fonts/NeoSansIntel/NeoSansIntel-MediumItalic.ttf", "text/ttf"                                           }
  ,
  { 
    "/static/fonts/glyphicons-halflings-regular.ttf", "text/ttf"                                           }
  ,  
  { 
    "/static/img/loading.gif", "image/gif"                                           }
  ,
  { 
    "/static/js/app.js", "application/javascript"                                           }
  ,
  { 
    "/static/js/jquery.jsPlumb-1.4.1-all.js", "application/javascript"                                           }
  ,
  { 
    "/static/js/jquery.mobile-1.4.1.js", "application/javascript"                                           }
  ,
  { 
    "/static/js/angular.min.js", "application/javascript"                                           }
  ,
  { 
    "/static/js/jquery.min.js", "application/javascript"                                           }
  ,
  { 
    "/static/js/jquery-ui.min.js", "application/javascript"                                           }
  ,
  { 
    "/static/js/underscore-min.js", "application/javascript"                                           }
  ,
  { 
    "/templates/pin.html", "text/html"                                           }
  ,
  { 
    "/templates/pin_settings.html", "text/html"                                           }
  ,
  /* last one is the default served if no match */
  { 
    "/index.html", "text/html"                                           }
  ,
};


/* this protocol server (always the first one) just knows how to do HTTP */

/*
This callback is called when the browser is refreshed (an HTTP call is performed).
 Here we send the files to the browser.
 */
static int callback_http(struct libwebsocket_context *context,
struct libwebsocket *wsi,
enum libwebsocket_callback_reasons reason, void *user,
void *in, size_t len)
{
  //     Serial.println("callback_http()");
  // WE ARE ALWAYS HITTING THIS POINT

  char buf[256];
  int n;

  switch (reason) {

  case LWS_CALLBACK_HTTP:
    lwsl_notice("LWS_CALLBACK_HTTP");

    for (n = 0; n < (sizeof(whitelist) / sizeof(whitelist[0]) - 1); n++)
      if (in && strcmp((const char *)in, whitelist[n].urlpath) == 0)
        break;

    sprintf(buf, LOCAL_RESOURCE_PATH"%s", whitelist[n].urlpath);

    if (libwebsockets_serve_http_file(context, wsi, buf, whitelist[n].mimetype))
      return 1; /* through completion or error, close the socket */

    /*
     * notice that the sending of the file completes asynchronously,
     * we'll get a LWS_CALLBACK_HTTP_FILE_COMPLETION callback when
     * it's done
     */

    break;

  case LWS_CALLBACK_HTTP_FILE_COMPLETION:

    lwsl_notice("LWS_FILE_COMPLETION");
    return 1;


  default:
    break;
  }

  return 0;
}

// Testing
char pcTestBuffer[512] = "{\"status\":OK,\"pins\":{\"14\":{\"label\":\"A0\",\"is_analog\":\"true\",\"is_input\":\"true\",\"value\":\"0.5\"},\"3\":{\"label\":\"PWM3\",\"is_analog\":\"true\",\"is_input\":\"false\",\"value\":\"0.0\"}},\"connections\":[{\"source\":\"14\",\"target\":\"3\"}]}\"";

/* lyt_protocol*/
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

    break;

  case LWS_CALLBACK_RECEIVE:

    Serial.println("Process Incomming Messag");

    // **********************************
    // Process Data Receieved from the Website
    // Update Galileo's HW    
    // **********************************    
//    procWebMsg((char*) in, len);
    processMessage((char*) in);

    break;

  default:
    break;
  }

  return 0;
}

//-----------------------------------------------------------
// Process recieved message from the webpage here.... 
//------------------------------------------------------------
void procClientMsg(char* _in, size_t _len) {

  /*
{	"status":<OK,ERROR>,
   	"pins":{	"<0,1,…,13,A0,…,A5>":
   				{	"label":"<label text>",
   					"is_analog":"<true,false>",
   					"is_input":"<true,false>",
   					"value":"<0.0,1.0>",
   				},
   				...,
   			},
   	"connections":	[	{"source":"<0,1,…,13,A0,…,A5>","target":"<0,1,…,13,A0,…,A5>"},
   						...,
   					]	
   }
   */

  Serial.println("procClientMsg()");
  Serial.println(String(_in));

  // Create JSON message
  aJsonObject *pJsonMsg = aJson.parse(_in);
  if( pJsonMsg == NULL )
  {
    Serial.println("ERROR: No JSON message to process");
    return;
  }

//  Serial.print("pJsonMsg is of type: ");
//  Serial.print(pJsonMsg->type);
  aJsonObject *pJsonStatus = aJson.getObjectItem(pJsonMsg, "status");
  aJsonObject *pJsonPins = aJson.getObjectItem(pJsonMsg, "pins");
  aJsonObject *pJsonConnections = aJson.getObjectItem(pJsonMsg, "connections");

  if( pJsonPins )  // Check if there is pin info
  {
    procPinsMsg( pJsonPins );
  }
  else if( pJsonConnections )  // Check if there is connection info
  {
    procConnMsg( pJsonConnections );  
  }
  else
  {
    Serial.println("No JSON message to process");
  }

}

//-----------------------------------------------------------
// Process recieved message from the webpage here.... 
//------------------------------------------------------------
void procWebMsg(char* _in, size_t _len) {

  String sMsg(_in);

  sMsg.toLowerCase();

  Serial.print("input: "); 
  Serial.println(sMsg);
  Serial.print("_len: "); 
  Serial.println(String(_len));

  // Parse incomming message
  if(  sMsg.startsWith("d") && _len <= 3 )
  {
    // Check if we are setting a Dx pin
    int iPin = sMsg.substring(1,sMsg.length()).toInt();
    Serial.print("Pin: ");
    Serial.println(String(iPin));
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
      Serial.print("Error. Unrecongnized message: ");   
      Serial.println(sMsg);
    }

    // Set pin value
    Serial.print("Pin: ");
    Serial.println(String(iPin));
    Serial.print("Value: ");
    Serial.println(String(iValue));
    g_aiP[iPin] = iValue; // Toggle state
    analogWrite(iPin, g_aiP[iPin]); // Set state
  }
  else
  {
    Serial.print("Error. Unrecongnized message: ");   
    Serial.println(sMsg);
  }
}

// **********************************
// Send pin status to the Website    
// **********************************
int  sendStatusToWebsite(struct libwebsocket *wsi)
{
  int n;
  unsigned char buf[LWS_SEND_BUFFER_PRE_PADDING + 512 + LWS_SEND_BUFFER_POST_PADDING];
  unsigned char *p = &buf[LWS_SEND_BUFFER_PRE_PADDING];

  n = sprintf((char *)p, "%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%f,%f,%f,%f,%f,%f",
  g_abD[0],
  g_abD[1],
  g_abD[2],
  g_aiP[3],   // Px
  g_abD[4],
  g_aiP[5],   // Px
  g_aiP[6],   // Px
  g_abD[7],
  g_abD[8],
  g_aiP[9],   // Px
  g_aiP[10],  // Px
  g_aiP[11],  // Px
  g_abD[12],
  g_abD[13],
  analogRead(A0)/float(ANALOG_IN_MAX_VALUE),
  analogRead(A1)/float(ANALOG_IN_MAX_VALUE),
  analogRead(A2)/float(ANALOG_IN_MAX_VALUE),
  analogRead(A3)/float(ANALOG_IN_MAX_VALUE),
  analogRead(A4)/float(ANALOG_IN_MAX_VALUE),
  analogRead(A5)/float(ANALOG_IN_MAX_VALUE)
    );

  n = libwebsocket_write(wsi, p, n, LWS_WRITE_TEXT); 

  return n;
}

static long unsigned int g_luiCounter = 0;
int  sendStatusToWebsiteNew(struct libwebsocket *wsi)
{

  int n = 0;

  if( g_luiCounter%100 == 0)
  {

    unsigned char buf[LWS_SEND_BUFFER_PRE_PADDING + WEB_SOCKET_BUFFER_SIZE + LWS_SEND_BUFFER_POST_PADDING];
    unsigned char *p = &buf[LWS_SEND_BUFFER_PRE_PADDING];
    
    updateBoardState();
        
    aJsonObject *msg = getJsonBoardState();  
    
    aJsonStringStream stringStream(NULL, (char *)p, WEB_SOCKET_BUFFER_SIZE);
    aJson.print(msg, &stringStream);
    String sTempString((char *)p);
      
    n = libwebsocket_write(wsi, p, sTempString.length(), LWS_WRITE_TEXT);
    
    aJson.deleteItem(msg);

  }

  g_luiCounter++;

  return n;
}

/* list of supported protocols and callbacks */
static struct libwebsocket_protocols protocols[] = {
  /* first protocol must always be HTTP handler, to serve webpage */

  {
    "http-only",		/* name */
    callback_http,		/* callback */
    0,			/* per_session_data_size */
    0,			/* max frame size / rx buffer */
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
    NULL, NULL, 0, 0                                           } /* terminator */
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

  /* we will only try to log things according to our debug_level */
  setlogmask(LOG_UPTO (LOG_DEBUG));
  openlog("lwsts", syslog_options, LOG_DAEMON);

  /* tell the library what debug level to emit and to send it to syslog */
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
   
  // Initialize all pins with no lable and 0.0 value
  for(int i=0; i<TOTAL_NUM_OF_PINS; i++)
  {
    memset(g_aPins[i].label, '\0', sizeof(g_aPins[i].label));
    //strcpy(g_aPins[i].label,"None");
    g_aPins[i].value = 0.0;
    memset(g_aPins[i].connections, '\0', sizeof(g_aPins[i].connections));
    for(int j=0; j<TOTAL_NUM_OF_PINS; j++)
    {
      if( i == j ) // Pins are always connected to themselves
        g_aPins[i].connections[j] = true;
      else
        g_aPins[i].connections[j] = false;
    }
  }

  strcpy(g_aPins[0].label,"0");
  strcpy(g_aPins[1].label,"1");
  strcpy(g_aPins[2].label,"2");
  strcpy(g_aPins[3].label,"3");
  strcpy(g_aPins[4].label,"4");
  strcpy(g_aPins[5].label,"5");
  strcpy(g_aPins[6].label,"6");
  strcpy(g_aPins[7].label,"7");
  strcpy(g_aPins[8].label,"8");
  strcpy(g_aPins[9].label,"9");
  strcpy(g_aPins[10].label,"10");
  strcpy(g_aPins[11].label,"11");
  strcpy(g_aPins[12].label,"12");
  strcpy(g_aPins[13].label,"13");
  strcpy(g_aPins[14].label,"A0");
  strcpy(g_aPins[15].label,"A1");
  strcpy(g_aPins[16].label,"A2");
  strcpy(g_aPins[17].label,"A3");
  strcpy(g_aPins[18].label,"A4");
  strcpy(g_aPins[19].label,"A5");

  // Initialize pins 0-13 as digital out pins
  for(int i=0; i<(TOTAL_NUM_OF_PINS-NUM_OF_ANALOG_PINS); i++)
  {
    if( i==3 || i==5 || i==6 || i==9 || i==10 || i==11 )
      g_aPins[i].is_analog = true;
    else  
      g_aPins[i].is_analog = false;   

    g_aPins[i].is_input = false;
    pinMode(i, OUTPUT);
    g_aPins[i].sensitivity = 0.5;
    g_aPins[i].is_inverted = false;
    g_aPins[i].is_visible = true;
  }

  // Initialize pins A0-A5 as analog in pins
  for(int i=NUM_OF_DIGITAL_PINS; i<TOTAL_NUM_OF_PINS; i++)
  {
    g_aPins[i].is_analog = true;
    g_aPins[i].is_input = true;
    pinMode(i, INPUT);
    g_aPins[i].sensitivity = 0.5;
    g_aPins[i].is_inverted = false;
    g_aPins[i].is_visible = true;
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
        g_aPins[i].value = analogRead(i)/float(ANALOG_IN_MAX_VALUE);
      }
      else // Process digital pins
      {
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
      {
        analogWrite(i, getTotalPinAnalogValue(i)*ANALOG_OUT_MAX_VALUE );
      }
      else // Process digital pins
      {
        digitalWrite(i, getTotalPinDigitalValue(i) );
      }
    }
  }
}

////////////////////////////////////////////////////////////////////////////
// If an output pin has connections, return the sum of all its connections
////////////////////////////////////////////////////////////////////////////
float getTotalPinAnalogValue(int _iPinNum)
{
  float fPinValSum = 0;
  int iConnCount = 1; // Pins are always connected to themselves

  for(int i=0; i<TOTAL_NUM_OF_PINS; i++)
  {
    // Checking what pins are connected to iPinNum
    if( _iPinNum != i) // Ignore the value set to the pin and only use connected pin values
    {
      if(g_aPins[_iPinNum].connections[i])
      {
        iConnCount++;
        fPinValSum += g_aPins[i].value;  
        Serial.print(i); Serial.print(" --> "); Serial.print(_iPinNum);
        Serial.print(" value: "); Serial.println(g_aPins[i].value);
      }    
    } 
  }
   
  if(fPinValSum > 1.0)
    fPinValSum = 1.0;
  
  if( 1 == iConnCount ) // No other connections found besides itself
    fPinValSum = g_aPins[_iPinNum].value;
  else
    g_aPins[_iPinNum].value = fPinValSum;
  
  Serial.print("Analog Pin #: "); Serial.print(_iPinNum); Serial.print(" Total Value: "); Serial.println(fPinValSum);
  
  return fPinValSum;
}

////////////////////////////////////////////////////////////////////////////
// If an output pin has connections, return the sum of all its connections
////////////////////////////////////////////////////////////////////////////
int getTotalPinDigitalValue(int _iPinNum)
{ 
  int iRetValue = 0;
  int iConnCount = 1; // Pins are always connected to themselves
    
  for(int i=0; i<TOTAL_NUM_OF_PINS; i++)
  {
    // Checking what pins are connected to iPinNum
    if( _iPinNum != i) // Ignore the value set to the pin and only use connected pin values
    {
      if(g_aPins[_iPinNum].connections[i])
      {
        iConnCount++;
        if(g_aPins[i].value >= DIGITAL_VOLTAGE_THRESHOLD) // Digital threshold
        {
          iRetValue = 1;
          break;      
        }
        else
          iRetValue = 0;          
      }    
    }
  }
  
  if( 1 == iConnCount ) // No other connections found besides itself
    iRetValue = g_aPins[_iPinNum].value;    
  else
   g_aPins[_iPinNum].value = iRetValue;    
  
  //g_aPins[_iPinNum].value = iRetValue;
  Serial.print("Digital Pin #: "); Serial.print(_iPinNum); Serial.print(" Total Value: "); Serial.println(iRetValue);
  
  return iRetValue;
}

////////////////////////////////////////////////
// Get the board state and return a JSON object
////////////////////////////////////////////////
aJsonObject* getJsonBoardState()
{

  char buff[100];
  
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

  // Create STATUS
  poStatus = aJson.createItem("OK");

  // Create PINS
  int iaPinConnects[TOTAL_NUM_OF_PINS];
  char caPinNumBuffer[10];
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

    // Populate sensitivity
    aJson.addItemToObject(apoPin[i],"sensitivity", aJson.createItem( g_aPins[i].sensitivity ) );
    
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

    // Push to JSON structure
    sprintf(caPinNumBuffer,"%d",i);
    aJson.addItemToObject(poPins,caPinNumBuffer,apoPin[i]);

  }
  
  /*
  {"status":<OK,ERROR>,  "pins":{"<0,1,…,13,A0,…,A5>":  {
  "label":"<label text>",  
	"is_analog":"<true,false>",  
	"is_input":"<true,false>", 
“sensitivity”:”<0.0,1.0>”, 
“is_inverted”:”<true,false>”, 
“is_visible”:”<true,false>”,
"value":"<0.0,1.0>",  },  ...,  }, 
"connections":[ {"source":"<0,1,…,13,A0,…,A5>","target":"<0,1,…,13,A0,…,A5>",”is_connected”:”<true,false>”},  ...,  ]  }
  */
  
  // Create CONNECTIONS
//  int iaConnections[10];
//  poConnections = aJson.createIntArray(iaConnections, 0); 
  
//  paConnections = 
 
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
  

  // Push to JSON object
  aJson.addItemToObject(poJsonBoardState,"status",poStatus);  
  aJson.addItemToObject(poJsonBoardState,"pins",poPins);
//  aJson.addItemToObject(poJsonBoardState,"connections",poConnections);
  aJson.addItemToObject(poJsonBoardState,"connections",paConnections);

  return poJsonBoardState;

}

void processMessage(char *_acMsg)
{
  
  Serial.print("Msg recvd: ");
  Serial.println(_acMsg);

  aJsonObject *poMsg = aJson.parse(_acMsg);

  if(poMsg != NULL)
  {
    aJsonObject *poStatus = aJson.getObjectItem(poMsg, "status");
    if (!poStatus) {
      Serial.println("ERROR: No Status data.");
      return;
    }
    else if ( strncmp(poStatus->valuestring,"OK",2) == 0 )
    {
      // Process if status is OK
      Serial.println("STATUS: OK");

      // Check if the message has pin data
      aJsonObject *pJsonPins = aJson.getObjectItem(poMsg, "pins");
      if( pJsonPins )  // Check if there is pin info
        procPinsMsg( pJsonPins );

      aJsonObject *pJsonConnections = aJson.getObjectItem(poMsg, "connections");
      if( pJsonConnections )  // Check if there is pin info
        procConnMsg( pJsonConnections );        
    }
    else if ( strncmp(poStatus->valuestring,"ERROR",5) == 0 )
    {
      // Process if status is ERROR
      Serial.println("STATUS: ERROR");
    }
    else
    {
      Serial.println("ERROR: Unknown status");      
    } 
  }
  else
  {
    Serial.println("Client message is NULL");
  }

  aJson.deleteItem(poMsg);
}

///////////////////////////////
// Process pin value messages
///////////////////////////////
void procPinsMsg( aJsonObject *_pJsonPins )
{
  Serial.println("Processing Pins");

  // Iterate all pins and check if we have data available
  for(int i=0; i<TOTAL_NUM_OF_PINS; i++)
  {
    char pinstr[3];
    snprintf(pinstr, sizeof(pinstr), "%d", i);    
    aJsonObject *poPinVals = aJson.getObjectItem(_pJsonPins, pinstr);
    if (poPinVals)
    {
      char sPinState[128];

      aJsonObject *poLabel = aJson.getObjectItem(poPinVals, "label");
      if (poLabel)
      {
        snprintf(g_aPins[i].label, PIN_LABEL_SIZE, "%s", poLabel->valuestring);
      }

      aJsonObject *poIsAnalog = aJson.getObjectItem(poPinVals, "is_analog");
      if (poIsAnalog)
      {
        g_aPins[i].is_analog = poIsAnalog->valuebool;
        
        /*
        if( 0 == strcmp(poIsAnalog->valuestring,"true") )
          g_aPins[i].is_analog = 1;
        else if( 0 == strcmp(poIsAnalog->valuestring,"false") )
          g_aPins[i].is_analog = 0;
        else
          g_aPins[i].is_analog = -1;
          */
      }       

      aJsonObject *poIsInput = aJson.getObjectItem(poPinVals, "is_input");
      if (poIsInput)
      {
        g_aPins[i].is_input = poIsInput->valuebool;
        /*
        if( 0 == strcmp(poIsInput->valuestring,"true") )
          g_aPins[i].is_input = 1;
        else if( 0 == strcmp(poIsInput->valuestring,"false") )
          g_aPins[i].is_input = 0;
        else
          g_aPins[i].is_input = -1;
          */
      }

      aJsonObject *poValue = aJson.getObjectItem(poPinVals, "value");
      if (poValue)
      {
//        g_aPins[i].value = atof(poValue->valuestring);
        g_aPins[i].value = poValue->valuefloat;
      }
    }    
  }
}

///////////////////////////////
// Process connection messages
///////////////////////////////
void procConnMsg( aJsonObject *_pJsonConnections )
{
  int uiSourcePin = -1;
  int uiTargetPin = -1;
  unsigned char ucNumOfConns = aJson.getArraySize(_pJsonConnections);
  
  Serial.print("Processing ");
  Serial.print(String(ucNumOfConns));
  Serial.println(" Connections");

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
        Serial.println("Source pin out of bounds");
        continue;
      }
    }
    else
    {
      Serial.println("No source in connections");
      continue;
    }
    
    // Get target
    aJsonObject* poTarget = aJson.getObjectItem(poItem, "target");
    if (poTarget)
    {
        uiTargetPin = atoi(poTarget->valuestring);
       if( poTarget->valueint < 0 || poTarget->valueint <= TOTAL_NUM_OF_PINS)
      {
        Serial.println("Target pin out of bounds");
        continue;
      }
    }
    else
    {
      Serial.println("No target in connections");
      continue;
    }
    
    Serial.print("Source: ");        
    Serial.println(String(uiSourcePin));        
    Serial.print("Target: "); 
    Serial.println(String(uiTargetPin));        
    
    // Toggle pin connection
    g_aPins[uiTargetPin].connections[uiSourcePin] = ~g_aPins[uiTargetPin].connections[uiSourcePin];
  
  } 
}

/////////////////////////////////////////////////////////////////////////////////
// Arduino standard funtions
/////////////////////////////////////////////////////////////////////////////////
void setup()
{
  // put your setup code here, to run once:
  Serial.begin(9600);
  Serial.println("Starting WebServer");
  system("/home/root/startAP");

  // Init Galileo HW
/*
  // Enable all digital pins as outputs
  for(int i=0; i<14; i++)
  {
    pinMode(i, OUTPUT);
  }

  // Init pins
  digitalWrite(0, LOW);
  digitalWrite(1, LOW);
  digitalWrite(2, LOW);
  digitalWrite(4, LOW);
  digitalWrite(7, LOW);
  digitalWrite(8, LOW);
  digitalWrite(12, LOW);
  digitalWrite(13, LOW);

  analogWrite(3, 0);
  analogWrite(5, 0);
  analogWrite(6, 0);
  analogWrite(9, 0);
  analogWrite(10, 0);
  analogWrite(11, 0);

  // Init pin state variables
  int i=0;
  for(i=0; i<TOTAL_NUM_Dx; i++)    
    g_abD[i] = false;
  for(i=0; i<TOTAL_NUM_Px; i++)
    g_aiP[i] = 0;   
*/
  // JSON protocol code
  initBoardState();

  Serial.println("Starting WebSocket");
  initWebsocket();  

}

char g_acMessage[1000];
int g_ToggleFlag = 0;
char g_acPin3_On[] = "{\"status\":\"OK\",\"pins\":{ \"13\":{\"label\":\"Pin 13\",\"is_analog\":\"false\",\"is_input\":\"false\",\"value\":\"1\"}, \"2\":{\"label\":\"Pin 2\",\"is_analog\":\"false\",\"is_input\":\"false\",\"value\":\"1\"}, \"4\":{\"label\":\"Pin 4\",\"is_analog\":\"false\",\"is_input\":\"false\",\"value\":\"1\"}, \"7\":{\"label\":\"Pin 7\",\"is_analog\":\"false\",\"is_input\":\"false\",\"value\":\"1\"}, \"8\":{\"label\":\"Pin 8\",\"is_analog\":\"false\",\"is_input\":\"false\",\"value\":\"1\"}, \"12\":{\"label\":\"Pin 12\",\"is_analog\":\"false\",\"is_input\":\"false\",\"value\":\"1\"}, \"3\":{\"label\":\"Pin 3\",\"is_analog\":\"true\",\"is_input\":\"false\",\"value\":\"0.9\"}, \"5\":{\"label\":\"Pin 5\",\"is_analog\":\"true\",\"is_input\":\"false\",\"value\":\"0.9\"}, \"6\":{\"label\":\"Pin 6\",\"is_analog\":\"true\",\"is_input\":\"false\",\"value\":\"0.9\"}, \"9\":{\"label\":\"Pin 9\",\"is_analog\":\"true\",\"is_input\":\"false\",\"value\":\"0.9\"}, \"10\":{\"label\":\"Pin 10\",\"is_analog\":\"true\",\"is_input\":\"false\",\"value\":\"0.9\"}, \"11\":{\"label\":\"Pin 11\",\"is_analog\":\"true\",\"is_input\":\"false\",\"value\":\"0.9\"}   }}";
char g_acPin3_Off[] = "{\"status\":\"OK\",\"pins\":{ \"13\":{\"label\":\"Pin 13\",\"is_analog\":\"false\",\"is_input\":\"false\",\"value\":\"0\"}, \"2\":{\"label\":\"Pin 2\",\"is_analog\":\"false\",\"is_input\":\"false\",\"value\":\"0\"}, \"4\":{\"label\":\"Pin 4\",\"is_analog\":\"false\",\"is_input\":\"false\",\"value\":\"0\"}, \"7\":{\"label\":\"Pin 7\",\"is_analog\":\"false\",\"is_input\":\"false\",\"value\":\"0\"}, \"8\":{\"label\":\"Pin 8\",\"is_analog\":\"false\",\"is_input\":\"false\",\"value\":\"0\"}, \"12\":{\"label\":\"Pin 12\",\"is_analog\":\"false\",\"is_input\":\"false\",\"value\":\"0\"}, \"3\":{\"label\":\"Pin 3\",\"is_analog\":\"true\",\"is_input\":\"false\",\"value\":\"0.1\"}, \"5\":{\"label\":\"Pin 5\",\"is_analog\":\"true\",\"is_input\":\"false\",\"value\":\"0.1\"}, \"6\":{\"label\":\"Pin 6\",\"is_analog\":\"true\",\"is_input\":\"false\",\"value\":\"0.1\"}, \"9\":{\"label\":\"Pin 9\",\"is_analog\":\"true\",\"is_input\":\"false\",\"value\":\"0.1\"}, \"10\":{\"label\":\"Pin 10\",\"is_analog\":\"true\",\"is_input\":\"false\",\"value\":\"0.1\"}, \"11\":{\"label\":\"Pin 11\",\"is_analog\":\"true\",\"is_input\":\"false\",\"value\":\"0.1\"}   }}";
//char g_acConnA0_TO_3[] = "{\"status\":\"OK\",\"connections\":[{\"source\":\"14\",\"target\":\"3\"},{\"source\":\"15\",\"target\":\"5\"}]}";

char g_acPin13_On_3_Analog[] = "{\"status\":\"OK\",\"pins\":{ \"13\":{\"label\":\"Pin 13\",\"is_analog\":\"false\",\"is_input\":\"false\",\"value\":\"1\"}, \"3\":{\"label\":\"Pin 3\",\"is_analog\":\"true\",\"is_input\":\"false\",\"value\":\"0.0\"}  }}";
char g_acConnA0_TO_3[] = "{\"status\":\"OK\",\"connections\":[{\"source\":\"14\",\"target\":\"3\"},{\"source\":\"15\",\"target\":\"3\"}]}";
int g_iDebugState = 0;

unsigned long last_print = 0;

void loop()
{

  if (millis() - last_print > 2000) {

    ///////////////////////////////////////
    // Connect/Disconnect
    /*
    switch(g_iDebugState)
    {
      case 0:
        snprintf(g_acMessage,sizeof(g_acMessage),g_acPin13_On_3_Analog);  
        processMessage(g_acMessage);
        g_iDebugState = 1;
      break;
      
      case 1:
        snprintf(g_acMessage,sizeof(g_acMessage),g_acConnA0_TO_3);  
        processMessage(g_acMessage);   
        g_iDebugState = 2;       
      break;
      
      default:
      
      break;
    }
   
    updateBoardState();
    */
    ///////////////////////////////////////

    //////////////////////////////////////////
    // Test sending message
    /*
    aJsonObject *msg = getJsonBoardState();
    aJson.print(msg, &serial_stream);
    Serial.println("");
    aJson.deleteItem(msg);
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
    Serial.println(g_iByte);

    switch (g_iByte)
    {
    case UP:
      g_iNewCode = 0;      
      break;

    case DOWN:
    case LEFT:
    case RIGHT:
      g_iNewCode = 1;            
      break;
    }
  }
}

















