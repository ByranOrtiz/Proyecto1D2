#include "config.h"

//variables globales
int velBanda = 0;
int SacosR = 0; //CANTIDAD DE SACOS ROJOS
int SacosV = 0; //CANTIDAD DE SACOS VERDES 
int SacosA = 0; //CANTIDAD DE SACOS AZULES 
int PesoS =0; //PESO DE TODOS LOS PESOS REGISTRADOS
// set up the feeds
AdafruitIO_Feed *velocidadFeed = io.feed("Velocidad");
AdafruitIO_Feed *onoffFeed = io.feed("OnOff");
AdafruitIO_Feed *CantRFeed = io.feed("CCRojo");
AdafruitIO_Feed *CantVFeed = io.feed("CCVerde");
AdafruitIO_Feed *CantAFeed = io.feed("CCAzul");
AdafruitIO_Feed *PesSacosFeed = io.feed("PesoSacos");
void setup() {

  // start the serial connection
  Serial.begin(115200);

  // wait for serial monitor to open
  while(! Serial);

  Serial.print("Connecting to Adafruit IO");

  // connect to io.adafruit.com
  io.connect();

  // set up a message handler for the count feed.
  // the handleMessage function (defined below)
  // will be called whenever a message is
  // received from adafruit io.
  onoffFeed->onMessage(handleMessage);    //recibidor de mensaje

  // wait for a connection
  while(io.status() < AIO_CONNECTED) {
    Serial.print(".");
    delay(500);
  }

  // we are connected
  Serial.println();
  Serial.println(io.statusText());

   // Because Adafruit IO doesn't support the MQTT retain flag, we can use the
  // get() function to ask IO to resend the last value for this feed to just
  // this MQTT client after the io client is connected.
  onoffFeed->get();

}

void loop() {

  // io.run(); is required for all sketches.
  // it should always be present at the top of your loop
  // function. it keeps the client connected to
  // io.adafruit.com, and processes any incoming data.
  io.run();
  velBanda = analogRead(34);
  velBanda = map(velBanda, 0, 4095, 0, 5);
  SacosR = analogRead(35);
  SacosR = map(SacosR, 0, 4095, 0, 7);
  SacosV = analogRead(39);
  SacosV = map(SacosV, 0, 4095, 0, 7);
  //SacosV = 4;
  SacosA = analogRead(32);
  SacosA = map(SacosA, 0, 4095, 0, 7);
  PesoS = analogRead(33);
  PesoS = map(PesoS, 0, 4095, 0, 45);
  //velBanda = 3;

  // save count to the 'counter' feed on Adafruit IO
  Serial.print("sending -> ");
  Serial.println(velBanda);
  velocidadFeed->save(velBanda);
  Serial.println(SacosR); //Va a mostrar en el puerto serial
  CantRFeed->save(SacosR); //rojos
  Serial.println(SacosV);
  CantVFeed->save(SacosV); //verdes
  Serial.println(SacosA);
  CantAFeed->save(SacosA);//azul
  Serial.println(PesoS);
  PesSacosFeed->save(PesoS);//azul
  // Adafruit IO is rate limited for publishing, so a delay is required in
  // between feed->save events. In this example, we will wait three seconds
  // (1000 milliseconds == 1 second) during each loop.
  delay(3000);

}

//Funciones
void handleMessage(AdafruitIO_Data *data) {
  Serial.print("received <- ");
  Serial.println(data->value());
}
