#include <Arduino.h>

/* TODO: Buzzer
   Pushbutton --> interrupt --> messung auf true setzen
   test blabla
*/
#include <LiquidCrystal_I2C.h> // https://bitbucket.org/fmalpartida/new-liquidcrystal/downloads
#include <Wire.h> // Comes with Arduino IDE
//
LiquidCrystal_I2C lcd(0x3F, 2, 1, 0, 4, 5, 6, 7, 3, POSITIVE);

int lcd_width = 16; // oder 20,4 bei großem Display
int lcd_hight = 2;

// Button
const byte button = 2;
const byte redLED = 5;
const byte greenLED = 6;
volatile unsigned long alteZeit = 0, entprellZeit = 200;

// Analogeingänge zur Erfassung der IR-LED Lichts
const int pin_schranke_1 = A0;
const int pin_schranke_2 = A1;

// Messwerte
boolean messung = true;
int messwert_1;
int messwert_2;
// Standardwerte vor der Kalibrierung
unsigned int schwellwert = 200;

// Zeitmessung
unsigned long startzeit;
unsigned long stoppzeit;
float zeitspanne;

void Werte_ausgeben(int schranken_nummer, int messwert) {
  Serial.print("Schranke ");
  Serial.print(schranken_nummer);
  Serial.print(": ");
  Serial.println(messwert);
}

void Stoppuhr(unsigned long n) {
  float zeit = float(millis() - n) / 1000;
  Serial.print(zeit, 3);
  Serial.println(" s");
  lcd.setCursor(0, 0);
  lcd.print(">>> Gestartet <<");
  lcd.setCursor(0, 1);
  lcd.print(zeit, 3);
  lcd.print("s");
}

void Check_Resetbutton() {
  if (digitalRead(button) == LOW) {
    if ((millis() - alteZeit) > entprellZeit) {
      // innerhalb der entprellZeit nichts machen
      messung = !messung;
      alteZeit = millis();
      Serial.println("Messung starten/stoppen");
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print(" Lichtschranke ");
      lcd.setCursor(0, 1);
      lcd.print("0.000s");
      if (messung == false) {
        digitalWrite(redLED, HIGH);
        lcd.print(" >>HALT<<");
      }
    }
  }
}

void setup() {
  Serial.begin(9600);

  pinMode(greenLED, OUTPUT);
  pinMode(redLED, OUTPUT);
  pinMode(button, INPUT_PULLUP);

  lcd.begin(lcd_width, lcd_hight);
  lcd.setCursor(0, 0);
  lcd.print(" Lichtschranke ");
  lcd.setCursor(0, 1);
  lcd.print("0.000s");
}

void loop() {
  // werte_auslesen();
  Check_Resetbutton();
  if (messung == true) {
    digitalWrite(greenLED, LOW);
    digitalWrite(redLED, LOW);
    messwert_1 = analogRead(pin_schranke_1);
    Werte_ausgeben(1, messwert_1);
    if (messwert_1 < 1023 - schwellwert) {
      startzeit = millis();
      digitalWrite(greenLED, HIGH);
      Werte_ausgeben(1, messwert_1);
      lcd.clear();
      while (messung == true) {
        Check_Resetbutton();
        messwert_2 = analogRead(pin_schranke_2);
        Stoppuhr(startzeit);
        // Serielle_Ausgabe(2, messwert_2);
        if (messwert_2 < 1023 - schwellwert) {
          stoppzeit = millis();
          digitalWrite(greenLED, LOW);
          digitalWrite(redLED, HIGH);
          zeitspanne = float(stoppzeit - startzeit) / 1000;
          Serial.print("Dauer: ");
          Serial.print(zeitspanne, 3);
          Serial.println(" s");

          lcd.setCursor(0, 0);
          lcd.print(">>> Gestoppt <<<");
          lcd.setCursor(0, 1);
          lcd.print(zeitspanne, 3);
          lcd.print("s");

          messung = false;
        }
      }
    }
  }
}
