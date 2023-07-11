//Library für Encoder
#include <Encoder.h>
//Libraries für OLED
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
//library für dfplayer mini
#include <DFMiniMp3.h>
#include <SoftwareSerial.h>
//definiere die vom dfplayer benötigte mp3notify klasse
class Mp3Notify;

//definiere OLED-Größe
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
//kein resetpin an oled
#define OLED_RESET     -1

//erstelle Oled display um das display zu steuern
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

//Encoder Objekt für einfaches auseles des Encoders
Encoder myEncoder(2, 3);
//Pin zum auslesen des EncoderButtons
int encoderButtonPin = 4;
//Pin zum auslesen des EncoderButtons
int introButtonPin = 5;
//fake kabel pins
int fakeCables[] = {5, 6, 7};
int fakeCablesLength = 3;
//deaktivierungskabel pins
int deactivateCable = 8;
//welche Minutenwerte stehen zur auswahl?
int minuteValues[] = {30, 45, 60};
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
int vol = 25;
//wurde die Vol vor kurzem verändert, soll ein spezieller screen gezeigt werden
bool volChanged = false;
// wann endet die anzeige der time anzeige
long volChangeEnd;
//wie lange wird die volchange message gezeigt
int volChangeDur = 1500;
//dfplayer
SoftwareSerial secondarySerial(10, 11); // RX, TX
typedef DFMiniMp3<SoftwareSerial, Mp3Notify> DfMp3;
DfMp3 dfmp3(secondarySerial);


void setup() {
  Serial.begin(115200);
  Serial.println("B-Attrappe von MM 2023");
  pinMode(encoderButtonPin, INPUT_PULLUP);
  pinMode(introButtonPin, INPUT_PULLUP);
  for (int i = 0; i < fakeCablesLength; i++) {
    pinMode(fakeCables[i], INPUT);
  }
  pinMode(deactivateCable, INPUT);
  dfmp3.begin();
  dfmp3.setVolume(vol);


}
long oldPosition  = -999;

void loop() {
  readEncoder();
  readButtons();
  checkTimes();
  checkCables();
  showDisplay();


}

void checkCables () {
  int result;
  //checke jedes fake kabel
  for (int i = 0; i < fakeCablesLength; i++) {
    //lese den pin
    result = digitalRead(fakeCables[i]);

    if (result == LOW) {
      if (timerStarted) {
        gameOver();

      }
    }
  }
  result = digitalRead(fakeCables);
  if (result == LOW) {
    if (timerStarted) {
      gameWon();

    }
  }
}

bool allCablesConnected() {
  int result = 0;
  //checke jedes fake kabel
  for (int i = 0; i < fakeCablesLength; i++) {
    //lese den pin
    result = digitalRead(fakeCables[i]);

    if (result == LOW) {
      if (timerStarted) {
        result++;
      }
    }
  }
  result = digitalRead(fakeCables);
  if (result == LOW) {
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
  long newPosition = myEncoder.read();
  //wenn position anders.
  if (newPosition != oldPosition) {
    //wenn kleiner als letze
    if (newPosition < oldPosition) {
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
    else {
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
    dfmp3.setVolume(vol);
    volChanged = true;
    volChangeEnd = millis() + volChangeDur;
    Serial.println("Lautstarke geandert: " + vol);

  }
}
//verringert lautstärke
void decreaseVol() {
  if (vol > 5) {
    vol--;
    dfmp3.setVolume(vol);
    volChanged = true;
    volChangeEnd = millis() + volChangeDur;
    Serial.println("Lautstarke geandert: " + vol);
  }
}

void gameOver() {
  if (!gameOverState && !gameWonState) {
    gameOverState = true;
    playBoom();
  }
}

void gameWon() {
  if (!gameOverState && !gameWonState) {
    gameOverState = true;
    playFanfare();
  }
}


void readButtons() {
  if (!gameOverState || !gameWonState) {

    //nur button eingaben akkzeptieren wenn wenn
    if (!timerStarted && allCablesConnected()) {
      if (digitalRead(encoderButtonPin) == LOW) {
        timerStarted = true;
        endTime = millis() + minuteValues[currentSelection] * 60 * 1000;
        Serial.print("Timer gestartet um: ");
        Serial.print(millis());
        Serial.print(". Timer gestartet mit: ");
        Serial.println( minuteValues[currentSelection] + " MIN");
        Serial.print("Ende: ");
        Serial.println(endTime);
      }

      if (digitalRead(introButtonPin) == LOW) {
        timerStarted = true;
        endTime = millis() + minuteValues[currentSelection] * 60 * 1000;
        Serial.print("Timer + Intro abspielen gestartet um: ");
        Serial.print(millis());
        Serial.print(". Timer gestartet mit: ");
        Serial.println( minuteValues[currentSelection] + " MIN");
        Serial.print("Ende: ");
        Serial.println(endTime);
        playIntro();
      }
    }
    else {
      if (digitalRead(introButtonPin) == LOW) {
        Serial.print(" Intro abspielen gestartet um: ");
        Serial.print(millis());
        Serial.print("Ende: ");
        Serial.println(endTime);
        playIntro();
      }
    }
  }
  //wenn gameover oder gamewon resetten wir die variablen dafür, und somit das system
  else {
    if (digitalRead(encoderButtonPin) == LOW) {
      gameOverState = false;
      gameWonState = false;
    }

    if (digitalRead(introButtonPin) == LOW) {
      gameOverState = false;
      gameWonState = false;
    }
  }
}
void showDisplay() {
  display.clearDisplay();
  if (!gameOverState || !gameWonState) {
    if (!timerStarted) {
      //zeichne minutenwert optionen und auswahlpfeil
      for (int i = 0; i < minuteValuesLength; i++) {
        display.setTextColor(SSD1306_WHITE);
        display.setCursor(20, i * 10 );
        display.setTextSize(0.5);
        display.print(minuteValues[i] + "MIN");
      }
      display.setCursor(0, currentSelection * 10 );
      display.setTextSize(0.5);
      display.print(" > ");
      if (!allCablesConnected()) {
        display.setCursor(0, 50 );
        display.setTextSize(0.5);
        display.print("!KABEL CHECKEN!");
      }
      else {
        display.setCursor(0, 50 );
        display.setTextSize(0.5);
        display.print("KABEL OKAY");
      }

    }
    else {
      if (!volChanged) {
        long remTime = endTime - millis();
        long remMinutes = remTime / 60000;
        long remSeconds = remTime / 1000 % 60;
        display.setTextColor(SSD1306_WHITE);
        display.setCursor(0, 0);
        display.setTextSize(2);
        display.print(remMinutes + "MIN");
        display.setCursor(0, SCREEN_HEIGHT / 2);
        display.setTextSize(2);
        display.print(remSeconds + "SEK");
      }
      else {
        display.setTextColor(SSD1306_WHITE);
        display.setCursor(0, 0);
        display.setTextSize(2);
        display.print("VOL: " + vol);
      }
    }
  }
  else if (gameOverState && !gameWonState) {
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(0, 0);
    display.setTextSize(2);
    display.print("GEWONNEN");
  }
  else if (gameWonState && !gameOverState) {
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(0, 0);
    display.setTextSize(2);
    display.print("VERLOREN!");
  }
  else if (gameWonState && gameOverState) {
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(0, 0);
    display.setTextSize(2);
    display.print("ERROR!");
  }

  display.display();
}

void playIntro() {
  dfmp3.stop();
  delay(300);
  dfmp3.playMp3FolderTrack(1);
}
void playBoom() {
  dfmp3.stop();
  delay(100);
  dfmp3.playMp3FolderTrack(2);
}
void playFanfare() {
  dfmp3.stop();
  delay(100);
  dfmp3.playMp3FolderTrack(3);
}
