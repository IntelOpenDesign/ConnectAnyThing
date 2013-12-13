/*

Copyright (c) 2013 - Philippe Laulheret, Second Story [http://www.secondstory.com]

This code is licensed under MIT. 
For more information visit  : https://github.com/secondstory/LYT

*/

#include <Dhcp.h>
#include <Dns.h>
#include <Ethernet.h>
#include <EthernetUdp.h>

#include <OSCBundle.h>
#include <OSCData.h>
#include <OSCMatch.h>
#include <OSCMessage.h>

#include <libwebsockets.h>

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <getopt.h>
#include <string.h>
#include <sys/time.h>
#include <assert.h>


#include <syslog.h>


#include <signal.h>


#define N_PANNEL 3
IPAddress ip[N_PANNEL]; 
EthernetUDP udp[N_PANNEL];

int udpPort=3333;
int led = 13; 
boolean currentLED = false;
int sensor1 = 0;
int sensor1Pin = A0;    // select the input pin for the potentiometer


EthernetUDP _udpIn;
//char colors[8*N_PANNEL] = "#000000,#000000,#000000";
char  retMessage[(1+1+4+1)]="0,1023";

//-----------------------------------------------------------
// Process recieved message here.... 
//------------------------------------------------------------
void processTouch(char* in) {
  
  Serial.print("input: "); 
  Serial.println(in);
  if(String(in) == "trigger"){
      Serial.println("Trigger found");
      if(currentLED){
        digitalWrite(led, LOW);
        currentLED = false;
      }else{
        digitalWrite(led, HIGH);
        currentLED = true;
      } 
      //delay(500);
  }

}

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
	{ "/favicon.ico", "image/x-icon" },
	{ "/img/2s.png", "image/png" },
        { "/img/splash.png", "image/png" },
        { "/img/splash2.png", "image/png" },
        { "/img/placeholder.png", "image/png" },
        { "/img/placeholder-on.png", "image/png" },
        { "/css/phone.css", "text/css" },
        { "/js/jquery-2.0.3.min.js", "application/javascript" },
        { "/js/phone.js", "application/javascript" },
        { "/js/pixel.js", "application/javascript" },
        { "/js/pixelView.js", "application/javascript" },
        { "/js/socketController.js", "application/javascript" },

	/* last one is the default served if no match */
	{ "/index.html", "text/html" },
};

/* this protocol server (always the first one) just knows how to do HTTP */

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

/* lyt_protocol*/
static int
callback_lyt_protocol(struct libwebsocket_context *context,
			struct libwebsocket *wsi,
			enum libwebsocket_callback_reasons reason,
					       void *user, void *in, size_t len)
{
  
        //Serial.println("callback_lyt_protocol()");
        // WE ARE ALWAYS HITTING THIS POINT
        
	int n;
	unsigned char buf[LWS_SEND_BUFFER_PRE_PADDING + 512 + LWS_SEND_BUFFER_POST_PADDING];
	unsigned char *p = &buf[LWS_SEND_BUFFER_PRE_PADDING];
	

  
	switch (reason) {


	case LWS_CALLBACK_SERVER_WRITEABLE:
                sensor1 = analogRead(sensor1Pin);
		n = sprintf((char *)p, "%d,%d", currentLED, sensor1);//colors);
                Serial.print("Output Message:");
                Serial.println(n);
		n = libwebsocket_write(wsi, p, n, LWS_WRITE_TEXT);
		if (n < 0) {
			lwsl_err("ERROR %d writing to socket\n", n);
			return 1;
		}

		break;

	case LWS_CALLBACK_RECEIVE:
		
         processTouch((char*) in); 		


		break;


	default:
		break;
	}

	return 0;
}



/* list of supported protocols and callbacks */

static struct libwebsocket_protocols protocols[] = {
	/* first protocol must always be HTTP handler */

	{
		"http-only",		/* name */
		callback_http,		/* callback */
		0,			/* per_session_data_size */
		0,			/* max frame size / rx buffer */
	},
	{
		"touchserver-protocol",
		callback_lyt_protocol,
		0,
		128,
	},

	{ NULL, NULL, 0, 0 } /* terminator */
};

void sighandler(int sig)
{
	force_exit = 1;
}

static struct option options[] = {

	{ NULL, 0, 0, 0 }
};

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





void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  Serial.println("Starting WebServer");
  system("/home/root/startAP");
  //delay(5000);  
  //ip[0] = IPAddress(192, 168, 0, 11);
  //ip[1] = IPAddress(192, 168, 0, 12);
  //ip[2] = IPAddress(192, 168, 0, 13) ;
  
  pinMode(led, OUTPUT);
  digitalWrite(led, LOW);
  //for (int i=0; i < N_PANNEL; i++)
  //{
  //  udp[i].begin(8888+i);
  // 
  //}

  
  _udpIn.begin(3333);
  _udpIn.listen();
  
  //pthread magic !
   pthread_t oscThread;
   pthread_create(&oscThread,NULL,osc_loop,0);
  
  //pthread magic end
  
  Serial.println("Starting WebSocket");
  initWebsocket();  
    
    
}


int getNextMessage(OSCBundle &b, int* from)
{
  
  int bytes  = _udpIn.parsePacket();
  if (bytes)
  {
    unsigned char *buffer = new unsigned char[bytes];
    bytes = _udpIn.read(buffer,bytes);
    //Serial.println(bytes);
    if (bytes > 1024) { 
      Serial.println("Overload warning !");
      Serial.println(bytes);
    }
    
    *from = -1;
    
    for (int i=0; i < N_PANNEL; i++)
    {
        if (ip[i] == _udpIn.remoteIP()) { *from = i; break; }
    }
    
    if (bytes >0) 
    {
      if (buffer[0] == '#') b.fill(buffer,bytes);
      else 
      {
        OSCMessage *m = new OSCMessage(); 
        //Serial.print(bytes);
        m->fill(buffer,bytes);
        if (!m->hasError()) b.add(*m);
        else { 
          Serial.println("OSC Message seems to have Error ");
          delete m;
          delete[] buffer;
          return 0;
        }
        
      }
    }
     
    delete[] buffer;
    delay(10);
    if (!b.hasError()) 
    { 
      
      return 1;
      }
      
    Serial.println("OSC Message seems to have Error ");
    return 0;
    
  }
  return 0;
}


//This is an hackish way of processing OSC Message
// There is no synchronization between the main thread and this one
// So... If it looks bad on the phone, don't use it :)
void *osc_loop(void *dummy)
{
  
  while (!force_exit) {
  //OSCBundle b;
  //OSCMessage *m;
  
  /*int from = -1;
  while (getNextMessage(b,&from))
  {
    if (m = b.getOSCMessage(0))
    {
      from = m->getInt(0);
      if (from != -1) { 
      
       int index = 8*from;
      sprintf(colors + index+1,"%06x",m->getInt(1));
      if (from != N_PANNEL -1) colors[index+7] = ',';
     // sleep(5);
       
      }
    }
    b.empty();
  }
  */
  sensor1 = analogRead(sensor1Pin);
  
  sleep(500);
  }
  
  return 0;
}


void loop()
{
  

  
}

