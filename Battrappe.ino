
//Libraries für OLED
#include <SPI.h>
#include <Wire.h>

//library für dfplayer mini
#include "Arduino.h"
#include "SoftwareSerial.h"
#include "DFRobotDFPlayerMini.h"
#include <U8g2lib.h>

//erstelle dfplayer objekt und softwareSerial
SoftwareSerial mySoftwareSerial(11, 12); // RX, TX
DFRobotDFPlayerMini myDFPlayer;
void printDetail(uint8_t type, int value);

//Pin zum auslesen des EncoderButtons
const int encoderButtonPin = 10;
//Pin zum auslesen des EncoderButtons
const int introButtonPin = 5;
// Rotary Encoder Inputs
const int CLK = 2;
const int DT  = 3;

//variablen für den Encoder
int counter = 0;
int currentStateCLK;
int lastStateCLK;

//fake kabel pins
int fakeCables[] = {9, 8, 7};
int fakeCablesLength = 3;
//deaktivierungskabel pins
int deactivateCable = 6;


//welche Minutenwerte stehen zur auswahl?
int minuteValues[] = {1, 20, 30};
//Wie viele minutenwerte sind es?
int minuteValuesLength = 3;
//welcher minutenwert ist gerade ausgewählt?
int currentSelection = 0;
//wurde der timer gestartet?
bool timerStarted = false;
//wenn der timer gestartet wurde enthalt diese variable die endzeit. in millisekunden
long endTime = 0 ;

//ist das spiel gewonnen oder verloren
bool gameOverState = false;
bool gameWonState = false;
//hält die volume
int vol = 30;
//wurde die Vol vor kurzem verändert, soll ein spezieller screen gezeigt werden
bool volChanged = false;
// wann endet die anzeige der time anzeige
long volChangeEnd;
//wie lange wird die volchange message gezeigt
int volChangeDur = 1500;

//instanziiere objekt für display
U8G2_SSD1306_128X64_NONAME_1_SW_I2C u8g2(U8G2_R0, /* clock=*/ SCL, /* data=*/ SDA, /* reset=*/ U8X8_PIN_NONE);   // All Boards without Reset of the Display





//wenn alle kabel korrekt verbunden sind, ist cablestate true, sonst false
bool cableState = true;


void setup() {

  Serial.begin(115200);
  Serial.println("B-Attrappe von MM 2023");

  u8g2.begin();

  pinMode(encoderButtonPin, INPUT_PULLUP);
  pinMode(introButtonPin, INPUT_PULLUP);

  for (int i = 0; i < fakeCablesLength; i++) {
    pinMode(fakeCables[i], INPUT_PULLUP);
  }
  pinMode(deactivateCable, INPUT_PULLUP);


  pinMode(CLK, INPUT);
  pinMode(DT, INPUT);
  lastStateCLK = digitalRead(CLK);

  attachInterrupt(digitalPinToInterrupt(2), updateEncoder, CHANGE);
  attachInterrupt(digitalPinToInterrupt(3), updateEncoder, CHANGE);

  //starte softwareserial und dfplayer
  mySoftwareSerial.begin(9600);
  if (!myDFPlayer.begin(mySoftwareSerial)) {
    //halte system an wenn dfplayer nicht findbar
    Serial.println("DFPLAYER error");
    //zeige infos dazu auf dem screen
    showDFPlayerError();
    while (1);
  }
  Serial.println(F("DFPlayer Mini online."));

  myDFPlayer.volume(vol);




}
long oldPosition  = -999;

void loop() {
  //zeige sachen auf dem Display an

  u8g2.firstPage();
  do {
    showDisplay();
  } while (u8g2.nextPage());

  //Prüfe werte des Drehimpulsgebers
  readEncoder();
  //schaue ob ein knopf gedrückt wurde
  readButtons();
  //gucke, welche zeitbasierten funktionen
  checkTimes();
  //prüfe ob die kabel durchgeschnitten wurden
  checkCables();



}

//guckt ob alle kabel verbunden sind bzw welche kabel nicht verbunden sind
void checkCables () {

  //Cablestate wird true oder false, je nachdem ob alle kabel verbunden sind
  //wir machen das hier, damit das hauptmenü anzeigen kann ob das system bereit ist
  cableState = allCablesConnected();

  int cableres;
  //checke jedes fake kabel
  for (int i = 0; i < fakeCablesLength; i++) {
    //lese den pin
    cableres = digitalRead(fakeCables[i]);
    //wenn fake-kabel durchtrennt dann game over
    if (cableres == HIGH) {
      if (timerStarted) {
        gameOver();
        Serial.println("Fake cable severed");

      }
    }
  }
  //checke deaktivierungskabel
  cableres = digitalRead(deactivateCable);
  //wenn deaktivierungskabel getrennt dann gewonnen
  if (cableres == HIGH) {
    if (timerStarted) {
      gameWon();
      Serial.println("Correct cable severed");

    }
  }

}

//gibt wahr zurück, wenn alle kabel verbunden sind
//gibt falsch zurück, wenn nicht alle kabel verbunden
bool allCablesConnected() {
  int result = 0;
  //checke jedes fake kabel
  for (int i = 0; i < fakeCablesLength; i++) {
    //lese den pin
    result = digitalRead(fakeCables[i]);

    Serial.print("Fake at: ");
    Serial.print(fakeCables[i]);
    Serial.print(" - ");
    Serial.println(result);
    if (result == HIGH) {
      if (timerStarted) {
        result++;
      }
    }
  }
  //checke deaktivierungskabel
  result = digitalRead(deactivateCable);
  Serial.print("Real  at: ");
  Serial.print(deactivateCable);
  Serial.print(" - ");
  Serial.println(result);
  if (result == HIGH) {
    if (timerStarted) {
      result++;
    }
  }
  if (result != 0) {

    return false;
  }
  else {
    return true;
  }
}



void readEncoder() {

  //leseEncoder
  long newPosition = counter;

  //wenn position anders.
  if (newPosition != oldPosition) {
    //wenn kleiner als letze
    if (newPosition < oldPosition) {
      Serial.println("Kleiner");
      //wenn timer gestarted inkrementiere currentselection - wählt menüitems aus
      if (!timerStarted) {
        currentSelection++;
      }
      //wenn timer schon gestartet kann der encoder als lautstärkeregler verwendet werden
      else {
        //erhöhe lautstärke
        increaseVol();
      }
    }
    //wenn position größer als letze
    else if (newPosition > oldPosition) {
      Serial.println("grosser");
      //wenn timer gestartet dekrementiere currentselection - wählt menüitems aus
      if (!timerStarted) {
        currentSelection--;
      }
      //veringere Lautstärke
      else {
        decreaseVol();
      }
    }
  }
  //limitiere werte die currentselection annehmen kann
  if (currentSelection < 0) {
    currentSelection = minuteValuesLength - 1;
  }
  else if (currentSelection >= minuteValuesLength) {
    currentSelection = 0;
  }
  oldPosition = newPosition;
}

//prüft verschiedene Zeiten
void checkTimes() {
  //als erstes die zeit holen - wir wollen millis nicht immer wieder abrufen
  long current = millis();
  //nur wenn spiel nicht vorüber
  if (!gameWonState && !gameOverState) {
    //wenn timer gestarted, prüfe ob zeit abgelaufen
    if (timerStarted) {
      //wenn zeit jetzt größer als endzeit dann ist game over
      if (current > endTime) {
        Serial.println("Zeit abgelaufen!");
        gameOver();
      }
    }
  }

  if (volChanged) {
    if (current > volChangeEnd) {
      Serial.println("Ende der Volume Anzeige");
      volChanged = false;

    }


  }
}
//erhöht lautstärke
void increaseVol() {
  if (vol < 30) {

    vol++;
    myDFPlayer.volume(vol);
    volChanged = true;
    volChangeEnd = millis() + volChangeDur;
    Serial.print("Lautstarke geandert: " );
    Serial.println(vol);

  }
}
//verringert lautstärke
void decreaseVol() {
  if (vol > 5) {
    vol--;
    myDFPlayer.volume(vol);
    volChanged = true;
    volChangeEnd = millis() + volChangeDur;
    Serial.print("Lautstarke geandert: " );
    Serial.println(vol);
  }
}

//aufgerufen wenn falsches kabel getrennt oder zeit abgelaufen
void gameOver() {
  if (!gameOverState && !gameWonState) {
    //  fillcircle();
    gameOverState = true;
    playBoom();
    Serial.println("game over");
  }
}
//aufgerufen wenn korrektes kabel getrennt
void gameWon() {
  if (!gameOverState && !gameWonState) {
    gameWonState = true;
    playFanfare();
    Serial.println("game won");
  }
}

//diese funktion reagiert auf drücken des encoders oder des intro buttons
void readButtons() {
  int introRes = digitalRead(introButtonPin);


  int encRes = digitalRead(encoderButtonPin);
  long timestamp = millis();

  //    Serial.print("EncoderButton:");
  //    Serial.println(encRes);
  //      Serial.print("Intro:");
  //    Serial.println(introRes);
  if (!gameOverState && !gameWonState) {

    //nur button eingaben akkzeptieren wenn wenn timer gestartet und alle Kabel korrekt verbunden
    if (!timerStarted && cableState) {
      //eingabe zum timer start mit dem encoder-knopf
      if (encRes == LOW) {
        timerStarted = true;
        endTime = timestamp + (minuteValues[currentSelection] * 60000);
        Serial.print("Timer gestartet um: ");
        Serial.print(timestamp);
        Serial.print(". Timer gestartet mit: ");
        Serial.print( minuteValues[currentSelection] );
        Serial.print("Ende: ");
        Serial.println(endTime);
      }
      //eingabe zum timer start mit dem Intro knopf
      if (introRes == LOW) {
        timerStarted = true;
        endTime = timestamp + (minuteValues[currentSelection] * 60000);
        Serial.print("Timer + Intro abspielen gestartet um: ");
        Serial.print(timestamp);

        playIntro();
      }
    }
    else {
      //wenn timer schon läuft spielt der intro knopf nur das intro
      if (introRes == LOW) {
        Serial.print(" Intro abspielen gestartet um: ");
        Serial.print(timestamp);
        Serial.print("Ende: ");
        Serial.println(endTime);
        playIntro();
      }
    }
  }
  //wenn gameover oder gamewon resetten wir die variablen dafür, und somit das system
  else {
    if (encRes == LOW) {
      gameOverState = false;
      gameWonState = false;
      timerStarted = false;
      Serial.println("reset");
      showReset();
      delay(2000);
    }

    if (introRes == LOW) {

      gameOverState = false;
      gameWonState = false;
      timerStarted = false;
      Serial.println("reset");
      showReset();
      delay(2000);
    }
  }
}

//diese funktion regelt die anzeige auf dem Bildschirm
void showDisplay() {
  //zeige splashscreen wenn system gestartet wird
  if (millis() < 3000) {
    showSplash();
  } else {
    //wenn das spiel nicht im gamover oder gewon zustand ist
    if (!gameOverState && !gameWonState) {
      //wenn der timer nicht gestartet ist, zeige hauptmenü
      if (!timerStarted) {
        //zeichne minutenwert optionen
        for (int i = 0; i < minuteValuesLength; i++) {

          u8g2.setFont(u8g2_font_ncenB10_tr);
          u8g2.setCursor(10, 20 + (i * 20));
          u8g2.print(minuteValues[i]);

        }
        //zeichne auswahlpfeil an der richtigen position, basierend auf der currentselection variabel
        u8g2.setCursor(0, 20 + ( currentSelection * 20));
        u8g2.print(">");

        //zeige an, ob die Kabel korrekt verbunden sind
        if (!cableState) {
          u8g2.setFont(u8g2_font_5x7_tr);

          u8g2.setCursor(40, 60 );
          u8g2.print("KABELFEHLER");
        }
        else {
          u8g2.setFont(u8g2_font_5x7_tr);

          u8g2.setCursor(40, 60 );
          u8g2.print("KABEL OKAY");
        }

      }
      //wenn der timer gestartet wurde, zeige die verbleibende zeit an
      else {
        if (!volChanged) {
          long remTime = endTime - millis();
          long remMinutes = remTime / 60000;
          long remSeconds = remTime / 1000 % 60;
          u8g2.setFont(u8g2_font_7Segments_26x42_mn);
          u8g2.setCursor(0, 42);
          u8g2.print(remMinutes);
          u8g2.setCursor(58, 42);
          u8g2.print(":");
          u8g2.setCursor(70, 42);
          u8g2.print(remSeconds);
        }
        //es sei denn, die lautstärke wurde kürzlich geändert: dann zeige lautstärke an
        else {
          u8g2.setFont(u8g2_font_ncenB10_tr);
          u8g2.drawStr(0, 20 ,  "VOL");
          u8g2.drawStr(64, 20 ,  String(vol).c_str());
        }
      }
    }
    //folgender Code zeigt informationen an, wenn spiel gewonnen oder verloren
    else if (gameOverState && !gameWonState) {
      u8g2.setFont(u8g2_font_ncenB10_tr);
      u8g2.drawStr(0, 30 , "Game Over");
      u8g2.setFont(u8g2_font_5x7_tr);
      u8g2.drawStr(0, 50 , "Knopf zum Reset ");
    }
    else if (gameWonState && !gameOverState) {
      u8g2.setFont(u8g2_font_ncenB10_tr);
      u8g2.drawStr(0, 20 , "GEWONNEN!");
      u8g2.setFont(u8g2_font_5x7_tr);
      u8g2.drawStr(0, 40 , "Saubere Arbeit!  ");
      u8g2.drawStr(0, 60 , "Knopf zum Reset ");
    }
    //wenn spiel gleichzeitig gewonnen und verloren ist, zeige ERROR an
    else if (gameWonState && gameOverState) {
      u8g2.setFont(u8g2_font_ncenB10_tr);
      u8g2.drawStr(0, 30 , "ERROR");
      u8g2.setFont(u8g2_font_5x7_tr);
      u8g2.drawStr(0, 50 , "Knopf zum Reset ");
    }





  }
}

//playfunktionen kapseln das abspielen bestimmter tracks auf dem DFPlayer
void playIntro() {
  myDFPlayer.playFolder(01, 01);
}
void playBoom() {
  myDFPlayer.playFolder(02, 01);
}
void playFanfare() {
  myDFPlayer.playFolder(03, 01);
}

//diese funktion wird als callback aufgerufen, wenn der drehimpulsgeber bewegt wird
//sie regelt die interne logik des encoders
void updateEncoder() {
  // Read the current state of CLK
  currentStateCLK = digitalRead(CLK);

  // If last and current state of CLK are different, then pulse occurred
  // React to only 1 state change to avoid double count
  if (currentStateCLK != lastStateCLK  && currentStateCLK == 1) {

    // If the DT state is different than the CLK state then
    // the encoder is rotating CCW so decrement
    if (digitalRead(DT) != currentStateCLK) {
      counter --;

    } else {
      // Encoder is rotating CW so increment
      counter ++;

    }


    // Serial.print(" | Counter: ");
    //Serial.println(counter);
  }

  // Remember last CLK state
  lastStateCLK = currentStateCLK;
}

void showSplash() {
 
  u8g2.setFont(u8g2_font_ncenB10_tr);
  u8g2.drawStr(0, 24, "Bomb!");
  u8g2.drawStr(0, 48, "MM 2023");

}
void showReset() {
  
  u8g2.setFont(u8g2_font_ncenB10_tr);
  u8g2.drawStr(0, 24, "RESET!");
  u8g2.drawStr(0, 48, "MM 2023");
}

void showDFPlayerError() {
  u8g2.firstPage();
  do {
    u8g2.setFont(u8g2_font_ncenB10_tr);
    u8g2.drawStr(0, 20 , "ERROR");
    u8g2.setFont(u8g2_font_5x7_tr);
    u8g2.drawStr(0, 30 , "DFPlayer-Problem ");
    u8g2.drawStr(0, 30 , "Checke SDCard");
  } while (u8g2.nextPage());

}
