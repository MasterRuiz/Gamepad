// DEFINICIÓN DE LIBRERÍAS A UTILIZAR
#include <avr/pgmspace.h> // es para utilizar memoria estática en los mensajes (flash) en lugar de memoria dinámica (RAM)
#include <nRF24L01.h>
#include <RF24.h>
#include <RF24_config.h>
#include <SPI.h>
#include <DS1307RTC.h>
#include <Wire.h>   // para LCD
#include <LiquidCrystal_I2C.h>
LiquidCrystal_I2C lcd(0x3F, 16, 2); // si no funciona con 3F, poner 27


// Configuraciones y variables para el NRF24L01
RF24 radio(9, 10); // CE = 9; CSN = 10;
const byte addresses[][6] = {"00001", "00002"};
char recepcion[32] = "";


// Configuraciones y variables para el Modulo de reloj
tmElements_t tm;
bool q=0;
char *meses[] = {"", "ENE", "FEB", "MAR", "ABR", "MAY", "JUN", "JUL", "AGO", "SEP", "OCT", "NOV", "DIC"};
String cadena="";


// Configuraciones y variables para el Bluetooth
bool seleccion=0;
bool busqueda=0;
bool aux=0;
bool ok=0;
bool listo=0;
char c = ' ';
char recibido="";
byte temp="";
byte otro=0;
byte k=1;
byte selector=0;
char a=0;
char pos1=-1;
char pos2=-1;
char pos3=-1;
String capturando[7];
String nombres[7];
String transferencia="";
String temporal="";
String device="";


// Configuraciones y variables generales y entradas físicas
byte modo=0;
unsigned int Joystick=0;
String linea="";
String adquisicion[7];


// Configuraciones y variables para LCD
byte number=0;
const char string_0[] PROGMEM  = "Starting System "; //  0
const char string_1[] PROGMEM  = " Running DEMO   ";
const char string_2[] PROGMEM  = "Ajuste la fecha ";
const char string_3[] PROGMEM  = " y hora actual  "; //  3
const char string_4[] PROGMEM  = "Date ";
const char string_5[] PROGMEM  = "Time ";
const char string_6[] PROGMEM  = "Elija el modo de"; //  6
const char string_7[] PROGMEM  = "  Comunicacion  "; 
const char string_8[] PROGMEM  = "Verde = NRF24L01";
const char string_9[] PROGMEM  = "Azul = BlueTooth"; //  9
const char string_10[] PROGMEM = "Configurando BT ";
const char string_11[] PROGMEM = "Elegido NRF24L01";
const char string_12[] PROGMEM = ""; // 12
const char string_13[] PROGMEM = "                ";
const char string_14[] PROGMEM = "Conexion exitosa"; // 39
const char string_15[] PROGMEM = "Conexion fallida"; // 30
const char string_16[] PROGMEM = "Terminando pulse";
const char string_17[] PROGMEM = "Verde p/ avanzar";
const char string_18[] PROGMEM = "Mensaje recibido"; // 18
const char string_19[] PROGMEM = "/";
const char string_20[] PROGMEM = ":";
const char string_21[] PROGMEM = " ";                // 21
const char string_22[] PROGMEM = "am";
const char string_23[] PROGMEM = "pm";
const char string_24[] PROGMEM = "Buscando";         // 24
const char string_25[] PROGMEM = " Enlace Fallido "; // 28
const char string_26[] PROGMEM = " Enlace Exitoso "; // 29
const char string_27[] PROGMEM = "    Buscando    "; // 30
const char string_28[] PROGMEM = "  Dispositivos  "; // 31
const char string_29[] PROGMEM = "Rojo -> Busqueda"; // 33
const char string_30[] PROGMEM = "Verde -> Elegir "; // 34
const char string_31[] PROGMEM = "Rojo  -> Esclavo"; // 36
const char string_32[] PROGMEM = "Verde -> Maestro"; // 37



const char *const texto[] PROGMEM ={string_0, string_1, string_2, string_3, string_4, string_5, 
                                    string_6, string_7, string_8, string_9, string_10, string_11,
                                    string_12, string_13, string_14, string_15, string_16, string_17,
                                    string_18, string_19, string_20, string_21, string_22, string_23, 
                                    string_24, string_25, string_26, string_27, string_28, string_29,
                                    string_30, string_31, string_32};
char comandoBuffer[18];




void setup(void)
{
  Serial.begin(38400);
  Wire.begin();
  pinMode(2,INPUT_PULLUP); // Botón azul
  pinMode(3,INPUT_PULLUP); // Botón amarillo
  pinMode(4,INPUT_PULLUP); // Botón verde
  pinMode(5,INPUT_PULLUP); // Botón rojo
  pinMode(6,INPUT_PULLUP); // Botón joystick derecho
  pinMode(7,INPUT_PULLUP); // Botón joystick central
  pinMode(8,INPUT_PULLUP); // Botón Joystick izquierdo
  lcd.init(); // si el LCD es 20x4 la instrucción cambia a lcd.begin(20,4);
  lcd.begin (16,2);
  lcd.backlight();
  transferencia.reserve(20);
  temporal.reserve(30);
  linea.reserve(18);


  // AJUSTE DE RELOJ
  

  if(digitalRead(5) == HIGH) // Si el botón del joystick izquierdo está oprimido, se salta el ajuste de hora
  {
    lcd.setCursor(0,0); number=0; pantalla(); 
    lcd.setCursor(0,1); number=1; pantalla(); 
    delay(1500);
    lcd.setCursor(0,0); number=2; pantalla(); 
    lcd.setCursor(0,1); number=3; pantalla(); 
    delay(1500);
    lcd.setCursor(0,0); number=16; pantalla();
    lcd.setCursor(0,1); number=17; pantalla();
    delay(1500);
    
    lcd.setCursor(0,0); number=4; pantalla(); // date
    lcd.setCursor(0,1); number=5; pantalla(); // time
    RTC.read(tm); 
    mostrarFecha();
    mostrarHora();

    while(digitalRead(4) == HIGH) // mientras no oprimas verde
    // AQUÍ SE AJUSTA FECHA Y HORA
      {
        RTC.read(tm); mostrarFecha(); mostrarHora();
        byte hora=(a2Digitos(tm.Hour)).toInt();
        byte minutos=(a2Digitos(tm.Minute)).toInt();
        byte segundos=(a2Digitos(tm.Second)).toInt();
        byte dia=(a2Digitos(tm.Day)).toInt();
        byte mes=tm.Month;
        int anio=(tmYearToCalendar(tm.Year));
        aux=0;
  
        // AJUSTE DE HORA
        if ((Joystick = analogRead(0)) == 0)
          { aux=1; segundos=segundos-1;}
        if ((Joystick = analogRead(0)) >= 1020)
          { aux=1; segundos=segundos+1;}
        if ((Joystick = analogRead(2)) == 0)
          { aux=1; minutos=minutos-1;}
        if ((Joystick = analogRead(2)) >= 1020)
          { aux=1; minutos=minutos+1;}
        if ((Joystick = analogRead(6)) == 0)
          { aux=1; hora=hora-1;}
        if ((Joystick = analogRead(6)) >= 1020)
          { aux=1; hora=hora+1;}
  
        // AJUSTE DE FECHA
        if ((Joystick = analogRead(1)) == 0)
          { aux=1; anio=anio-1;}
        if ((Joystick = analogRead(1)) >= 1020)
          { aux=1; anio=anio+1;}
        if ((Joystick = analogRead(3)) == 0)
          { aux=1; mes=mes-1;}
        if ((Joystick = analogRead(3)) >= 1020)
          { aux=1; mes=mes+1;}
        if ((Joystick = analogRead(7)) == 0)
          { aux=1; dia=dia-1;}
        if ((Joystick = analogRead(7)) >= 1020)
          { aux=1; dia=dia+1;}
  
        if (aux==1)
        {setTime(hora, minutos, segundos, dia, mes, anio);
         RTC.set(now()); delay(100);
        }
      }

    while (digitalRead(4) == LOW){} // mientras está oprimido
  }

  

  
  lcd.setCursor(0,0); number=6; pantalla(); // Elija el modo de comunicación
  lcd.setCursor(0,1); number=7; pantalla(); // 
  delay(1500);
  lcd.setCursor(0,0); number=8; pantalla(); // NRF24L01 O BLUETOOTH
  lcd.setCursor(0,1); number=9; pantalla(); // 


  while(digitalRead(4) == HIGH && digitalRead(2) == HIGH) {} // 5,4,2 mientras no oprimas ninguna opción

  
                              // NRF24L01
  if (digitalRead(4) == LOW) 
    { modo=0; 
    lcd.setCursor(0,0); number=11; pantalla(); 
    SPI.begin();  SPI.setBitOrder(MSBFIRST);
    lcd.setCursor(0,1); number=13; pantalla();
    delay(1000);
    radio.begin();
    radio.openWritingPipe(addresses[1]); // 00002
    radio.openReadingPipe(1, addresses[0]); // 00001
    radio.setPALevel(RF24_PA_MIN);
    radio.startListening();
    } 
  

                            // BLUETOOTH
  if (digitalRead(2) == LOW) // 2
    {
      lcd.setCursor(0,0); number=31; pantalla(); // 
      lcd.setCursor(0,1); number=32; pantalla(); //
      while(digitalRead(4) == HIGH && digitalRead(5) == HIGH){}
      if (digitalRead(5) == LOW) // modo esclavo
        {modo=2;} 
      if (digitalRead(4) == LOW) // modo maestro
        {modo=1;} 
      lcd.setCursor(0,0); number=10; pantalla(); // CONFIGURANDO BLUETOOTH
      lcd.setCursor(0,1); number=13; pantalla(); 
      
      if (modo==2) // modo esclavo no hagas nada
      { 
        Serial.println("AT+ROLE=0"); delayAndRead();
        Serial.println("AT+INIT"); delayAndRead();
        Serial.println("AT+INQ"); delayAndRead();
      } 
      if (modo==1)
      {
        inicializar();
        while (seleccion==0) {select();}
        seleccion=0;
        busqueda=0;
      }
    }
}




void loop(void)
{
  if (digitalRead(8) == HIGH) // joystick verde sin oprimir lecturas Analógicas
  {
    for (byte a=0; a<6; a++){adquisicion[a]="";}
    linea="";
    Joystick = analogRead(6); delayMicroseconds(1);   adquisicion[0]="Eje X izquierdo: "; adquisicion[0] += Joystick;
    line();
    Joystick = analogRead(2); delayMicroseconds(1); adquisicion[1]="Eje X central: "; adquisicion[1] += Joystick;
    line();
    Joystick = analogRead(0); delayMicroseconds(1);  adquisicion[2]="Eje X derecho: "; adquisicion[2] += Joystick;
    line(); linea+=" ";
    lcd.setCursor(0,0); lcd.print(linea);
    
    linea="";
    Joystick = analogRead(7); delayMicroseconds(1);   adquisicion[3]="Eje Y izquierdo: "; adquisicion[3] += Joystick;
    line();
    Joystick = analogRead(3); delayMicroseconds(1); adquisicion[4]="Eje Y central: "; adquisicion[4] += Joystick;
    line();
    Joystick = analogRead(1); delayMicroseconds(1);  adquisicion[5]="Eje Y derecho: "; adquisicion[5] += Joystick;
    line();
    linea+=" ";
    lcd.setCursor(0,1); lcd.print(linea);

    delay(100);
    q=0;
  }

  if (digitalRead(8) == LOW) // joystick verde oprimido lecturas digitales
  {
    for (byte a=6; a<=7; a++){adquisicion[a]="";}
    
    bool boton=HIGH;
    adquisicion[6]+="BTN: ";
    adquisicion[7]+="JYK: ";
    boton = digitalRead(2); delayMicroseconds(1);
    if (boton==LOW){adquisicion[6]+="B1 ";} delay(1);
    boton = digitalRead(3);delayMicroseconds(1);
    if (boton==LOW){adquisicion[6]+="B2 ";} delay(1);
    boton = digitalRead(4); delayMicroseconds(1);
    if (boton==LOW){adquisicion[6]+="B3 ";} delay(1);
    boton = digitalRead(5); delayMicroseconds(1);
    if (boton==LOW){adquisicion[6]+="B4 ";} delay(1);
    boton = digitalRead(6); delayMicroseconds(1);
    if (boton==LOW){adquisicion[7]+="Der ";} delay(1);
    boton = digitalRead(7); delayMicroseconds(1);
    if (boton==LOW){adquisicion[7]+="Cen ";} delay(1);
    boton = digitalRead(8); delayMicroseconds(1);
    if (boton==LOW){adquisicion[7]+="Izq ";} delay(1);
    
    if (digitalRead(5) == HIGH) // si el boton rojo no está oprimido
    { 
      lcd.setCursor(0,0); lcd.print(adquisicion[6]); number=13; pantalla(); 
      lcd.setCursor(0,1); lcd.print(adquisicion[7]); number=13; pantalla();
      q=1;
    }
    
    if (digitalRead(5) == LOW) // si el boton rojo está oprimido muestras fecha y hora
    {
      if (q==1)
      {
        lcd.setCursor(0,0); number=4; pantalla(); // Date
        lcd.setCursor(0,1); number=5; pantalla(); // Time
        q=0;
      }
      adquisicion[6]=("."); adquisicion[7]=(".");
      RTC.read(tm); mostrarFecha(); mostrarHora();
    }
    if (adquisicion[6].equals("")){lcd.setCursor(5,0); number=13; pantalla();}
    if (adquisicion[7].equals("")){lcd.setCursor(5,1); number=13; pantalla();}
  }
  

                            // NRF24L01
  if (modo==0)
  {
    if (digitalRead(8) == LOW && digitalRead(4) == LOW) // si izquierdo y verde se oprimen, envías botones 
    {
      radio.stopListening();
      radio.openWritingPipe(addresses[1]); // 00002
      for(byte i=6; i<8; i++)   // mandamos botones
      {
        byte str_len = adquisicion[i].length() + 1; 
        char info[str_len];
        adquisicion[i].toCharArray(info, str_len);
        radio.write(&info, sizeof(info));
        delay(10);
      }
      radio.openReadingPipe(1, addresses[0]); // 00001
      radio.startListening();
      delay(500);
    }
    if (digitalRead(8) == HIGH && digitalRead(4) == LOW) // si solo verde, enviamos analógicos
    {
      radio.stopListening();
      radio.openWritingPipe(addresses[1]); // 00002
      for(byte i=0; i<6; i++)   // mandamos analógicos
        {
          byte str_len = adquisicion[i].length() + 1; 
          char info[str_len];
          adquisicion[i].toCharArray(info, str_len);
          radio.write(&info, sizeof(info));
          delay(10);
        }
      radio.openReadingPipe(1, addresses[0]); // 00001
      radio.startListening();
      delay(500);
    }

    
    if (radio.available()) 
    {
      char recepcion[18] = "";
      while (radio.available()) 
      { radio.read(&recepcion, sizeof(recepcion)); }
      lcd.setCursor(0,0); number=18; pantalla();
      lcd.setCursor(0,1); number=13; pantalla();
      lcd.setCursor(0,1); lcd.print(recepcion);
      delay(1500);
    }
  }


                            // HC05 BLUETOOTH
  if (modo==1 || modo==2)
  {
    if(Serial.available())  // BTserial.available()
    {
      transferencia="";
      c="";
      c=Serial.read(); // BTserial.read();
      transferencia+=c;
      while (c!='\n'){c=Serial.read(); transferencia+=c;} 
      transferencia.remove(transferencia.length()-2);
      lcd.setCursor(0,0); number=18; pantalla(); 
      lcd.setCursor(0,1); number=13; pantalla(); lcd.setCursor(0,1); lcd.print(transferencia); 
      delay(1500);
    }
    
    if (digitalRead(8) == LOW && digitalRead(4) == LOW) // si el botón verde se oprime
    { for (byte i=6; i<8; i++){Serial.println(adquisicion[i]);} } // mandamos botones
    if (digitalRead(8) == HIGH && digitalRead(4) == LOW)
    { for (byte i=0; i<6; i++){Serial.println(adquisicion[i]);} } // mandamos analogicos
  }
}


void delayAndRead()
{
  delay(50);
  while(Serial.available()) // BTserial.available()
  { c = Serial.read(); } // BTserial.read();
  delay(800);
}


String a2Digitos(byte numero) 
{
  if (numero >= 0 && numero < 10) 
  {return "0" + String(numero);}
  else {return String(numero);}
}

String to2Digitos(byte numero) 
{
  if (numero > 12) 
  {numero=numero-12;}
  if (numero >= 0 && numero < 10) 
  {return "0" + String(numero);}
  else {return String(numero);}
}
void mostrarFecha()
{
  lcd.setCursor(5,0);
  lcd.print(a2Digitos(tm.Day));
  lcd.setCursor(7,0);
  number=19; pantalla();
  lcd.setCursor(8,0);
  lcd.print(meses[tm.Month]);
  lcd.setCursor(11,0);
  number=19; pantalla();
  lcd.setCursor(12,0);
  lcd.print(tmYearToCalendar(tm.Year));
}


void mostrarHora()
{
  lcd.setCursor(5,1);
  lcd.print(to2Digitos(tm.Hour));
  lcd.setCursor(7,1);
  number=20; pantalla();
  lcd.setCursor(8,1);
  lcd.print(a2Digitos(tm.Minute));
  lcd.setCursor(10,1);
  number=20; pantalla();
  lcd.setCursor(11,1);
  lcd.print(a2Digitos(tm.Second));
  lcd.setCursor(13,1);
  number=21; pantalla();
  lcd.setCursor(14,1);
  if (tm.Hour<12) {number=22; pantalla();}
  else {number=23; pantalla();}
}




bool buscar()
{
                            // ESTA RUTINA BUSCA LAS DIRECCIONES MAC DISPONIBLES
  {                
    lcd.setCursor(0,0); number=27; pantalla(); //Serial.println("Buscando");
    lcd.setCursor(0,1); number=28; pantalla(); //Serial.println("Dispositivos");
    String nombre="";
    nombre.reserve(200); // AGREGAR 30 POR CADA dispositivo permitido
    Serial.println("AT+INQ");
    int incoming=-1;
    incoming=Serial.read();
    while(incoming==-1){incoming=Serial.read();}
    nombre+=(char)incoming;
    while(incoming!='K') // mientras llegan todas las direcciones y acaba con OK
    {
      incoming=Serial.read();
      if (incoming==43 || incoming==44 || (incoming>47 && incoming<59) || (incoming>64 && incoming<71)) // si es caracter visible
      { nombre+=(char)incoming; } // + , 0 1 2 3 4 5 6 7 8 9 : A B C D E F
    }
    incoming=0;
    nombre+=(char)incoming; // se le agrega el TERMINADOR NULL para que pueda ser leída y utilizada
    


                            // ESTA RUTINA CREA UN ARREGLO DE STRINGS
                            // SOLO PERMITE CAPTURAR 8 DISPOSITIVOS POR FALTA DE MEMORIA

    char z=0;
    for (byte a=0; z>=0; a++) // crea el array de strings, eliminando comas y cambiando : por ,
    {
      z=nombre.indexOf('+',1); // busca el caracter + a partir de y+1
      if (z>0)
      {
        capturando[otro]=nombre.substring(2,z); // extrae a partir de posición 2 hasta donde encontraste z
        nombre.remove(0,z); // remueve lo extraído del nombre
        z=0;
      }
      if (z==-1) {capturando[otro]=nombre.substring(2);}
      capturando[otro].remove(capturando[otro].indexOf(',')); // quita a partir de las comas
      capturando[otro].replace(':',','); // remplaza los : por ,
      otro++;
    }
  }
 


                            // RUTINA PARA ELIMINAR LAS DIRECCIONES REPETIDAS
                       
  String nombre="";                   
  nombre.reserve(20);                 
  aux=0;
  k=1; 
  for (byte a=1; a<=otro; a++) 
  {
    if (capturando[a].length()<6){capturando[a]=capturando[a-1];} 
    nombre=capturando[a];
    for (byte j=0; j<a; j++) 
    { if (nombre==capturando[j]){j=a; aux=1;}  }
    if (aux==0){capturando[k]=nombre; k++; nombre="";} 
    if (aux==1){aux=0;} //AGREGADO LINEA DE ARRIBA
  }
  int incoming;
  while(Serial.available()){incoming=Serial.read();}

  
                    // RUTINA PARA SOLICITAR NOMBRES DE DISPOSITIVOS NO REPETIDOS
  for(byte a=0; a<k; a++) // si k=0, si no se cumple la condición, ni si quiera va a entrar al for
  {
    temporal="AT+RNAME?";
    nombre="";
    incoming=""; // cambio a vacio
    listo=0;
    pos1=-1;
    pos2=-1;
    pos3=-1;
    temporal+=capturando[a];
    Serial.println(temporal);
    while (Serial.available()==0){}// mientras no tengas datos por recibir, aquí te quedas
    delay(50);
    
    while(Serial.available()>0)
    {incoming=Serial.read(); nombre+=(char)incoming;}
    
    pos1=nombre.indexOf("OK\r\n");
    pos2=nombre.indexOf("FAIL");
    pos3=nombre.indexOf("ERROR:(0)");
    if (pos2>=0 || pos3>=0) { nombres[a]="No disponible"; listo=1;} // SI FALLÓ LA RECEPCIÓN
    if (pos1>=0) // SI OK QUITA    OK\r\n    y    +RNAME:
    { nombre.remove(nombre.length()-6); nombre.remove(0,7); nombres[a]=nombre; listo=1; }
  }
  nombre="";
  lcd.setCursor(0,0); number=29; pantalla(); 
  lcd.setCursor(0,1); number=30; pantalla(); 
  delay(1500);

                
  a=0;                        //RUTINA PARA MOSTRAR DISPOSITIVOS EN LCD
  char lasta=1;
  while (digitalRead(5) == HIGH && digitalRead(4) == HIGH)
  {
    if (a!=lasta)
    {
      lcd.setCursor(0,0); number=13; pantalla(); lcd.setCursor(1,0); lcd.print(nombres[a]);
      lcd.setCursor(0,1); number=13; pantalla(); lcd.setCursor(1,1); lcd.print(nombres[a+1]);
      lasta=a; 
      if (a!=k) {lcd.setCursor(0,0); lcd.print(">");}
      if (a==k) {lcd.setCursor(0,1); lcd.print(">");}
    }
    if ((Joystick = analogRead(0)) == 0)
      {
        a=a+1;
        if (a==k){a--;}
        delay(200);
      }
    if ((Joystick = analogRead(0)) >= 1020)
      {
        a=a-1;
        if (a<0){a=0;}
        delay(200);
      }
  }
  if (digitalRead(5) == LOW){busqueda=0; inicializar();} // si reescanear
  if (digitalRead(4) == LOW){busqueda=1;} // si avanzar
}


bool select()
{
  while (busqueda==0)
    { buscar(); }
    temp="";
    //selector=a;
    temporal="AT+PAIR=";
    temporal+=capturando[a]; // a-->selector
    temporal+=",15"; // límite de segundos para conectarse
    Serial.println(temporal); // BTserial.println(temp2);
    device="";
    recibido="";
    ok=0;
    pos2=-1;
    pos3=-1;
    
    while(ok==0)  // mientras que no hayas acabado de recibir
    {
      recibido=Serial.read(); 
      while (recibido==-1 || recibido=='\n'){recibido=Serial.read();} 
      if (recibido!=-1){device+= String(recibido);} 
      pos2=device.indexOf("OK");
      pos3=device.indexOf("FAIL");
      if (pos2>=0){lcd.setCursor(0,0); number=26; pantalla(); ok=1; seleccion=1; }
      if (pos3>=0){lcd.setCursor(0,0); number=25; pantalla(); ok=1; seleccion=0; } 
      if (ok==1){lcd.setCursor(0,1); number=13; pantalla();}
    }
    conectar();
}

void conectar()
{
    ok=0;
    pos2=-1;
    pos3=-1;
    temporal="";
    temporal="AT+LINK=";
    temporal+=capturando[a];
    Serial.println(temporal); 
    recibido=-1;
    device="";
    
    while(ok==0)  
    {
      recibido=Serial.read(); 
      while (recibido==-1 || recibido=='\n'){recibido=Serial.read();} 
      device+= String(recibido);
      pos2=device.indexOf("OK");
      pos3=device.indexOf("FAIL");
      if (pos2>=0)
      {
        lcd.setCursor(0,1); number=14; pantalla(); ok=1; seleccion=1; busqueda=1; // Conexión exitosa
        delay(1500);
      }
      if (pos3>=0)
      {
        lcd.setCursor(0,1); number=15; pantalla(); ok=1; seleccion=0; busqueda=0; // Conexión fallida
        delay(1500);
        lcd.setCursor(0,0); number=13; pantalla();
        lcd.setCursor(0,1); number=13; pantalla();
      }
    }
    if (busqueda==0){inicializar(); buscar();}
}




void pantalla()
{
  strcpy_P(comandoBuffer, (char *)pgm_read_word(&(texto[number])));
  lcd.print(comandoBuffer); 
}


void inicializar()
{
  Serial.println("AT+ROLE=1"); delayAndRead();
  Serial.println("AT+INQM=1,7,16"); delayAndRead();
  Serial.println("AT+INIT"); delayAndRead();
  Serial.println("AT+INQC"); delayAndRead();
}

void line()
{
  if (Joystick<1000){linea+=" ";} 
  if (Joystick<100){linea+=" ";} 
  if (Joystick<10){linea+=" ";}
  linea+= (String)Joystick; 
  linea+=" ";
}
