# B-Attrappe
## Brief
Software for timer/Media playback device used in an escape room. 

# Funktionsübersicht

Die Spielleitung kann mit einem Drehimpulsgeber zwischen 3 Spielzeiten wählen. Hierfür wird ein Menü auf dem OLED angezeigt. Mit dem Knopf des Drehimpulsgebers wird das Spiel gestartet. Alternativ kann das Spiel mit dem Intro-Knopf gestartet werden. Dieser erlaubt den Teilnehmenden weiterhin, jederzeit eine Intro-Nachricht mit dem DFplayer abzuspielen. Wird das Spiel gestartet, zeigt der OLED die verbleibende Spielzeit an. Während des Spiels kann mit dem Drehimpulsgeber die Lautstärke des DFplayer manipuliert werden. Die Teilnehmenden müssen herausfinden, welches der vier Kabel zertrennt werden muss, um das Gerät zu entschärfen. Wird das korrekte Kabel gekappt, wird eine Gratulationsnachricht abgespielt. Läuft die Zeit ab, oder wird das falsche Kabel gekappt, wird ein Explosionsgeräusch abgespielt. Wenn das Spiel gewonnen oder verloren wird, kann das Gerät mit den Knöpfen resettet werden. Das Gerät prüft vor Spielstart, ob alle Kabel korrekt verbunden sind. 

# Verwendete Komponenten

Arduino Nano 
DFPlayer Mini
Drehimpulsgeber mit Knopf - Breakout Board
OLED Screen 128x64
Knopf 
PinHeader DuPont Style - Männlich 
2x 1000k Resistor
Micro-SD Karte 
Schraubverbinder 5.08mm (4x2er)
Lochrasterplatine 

# Verbindungen Arduino

| ### Pin Arduino Nano  | ### Pin andere Komponente                       | ### Notizen                              |
|-----------------------|-------------------------------------------------|------------------------------------------|
| 5V                    | OLED-Pinheader (OLED-VCC)                       |                                          |
| 5V                    | Drehimpulsgeber Pinheader (Drehimpulsgeber +)   |                                          |
| 5V                    | DFPlayer VCC                                    |                                          |
| GND                   | OLED-Pinheader (OLED-GND)                       |                                          |
| GND                   | Drehimpulsgeber Pinheader (Drehimpulsgeber GND) |                                          |
| GND                   | Introbutton Pinheader                           |                                          |
| GND                   | Kabel Schraubterminal Links                     |                                          |
| GND                   | DFPlayer GND                                    |                                          |
| A4                    | OLED-Pinheader (SDA)                            | Hardware i2c - nicht änderbar            |
| A5                    | OLED-Pinheader (SCL)                            | Hardware i2c - nicht änderbar            |
| D11                   | DFPlayer RX (via 1k Resistor)                   | 1k Resistor unterdrückt Noise            |
| D12                   | DFPlayer TX (via 1k Resistor)                   | 1k Resistor unterdrückt Noise            |
| D10                   | Drehimpulsgeber Pinheader (Drehimpulsgeber SW)  | Button des Impulsgebers                  |
| D2                    | Drehimpulsgeber Pinheader (Drehimpulsgeber CLK) | D2 unterstützt Interrupts - nicht ändern |
| D3                    | Drehimpulsgeber Pinheader (Drehimpulsgeber DT)  | D3 unterstützt Interrupts - nicht ändern |
| D5                    | Introbutton Pinheader                           | Eingang des Introbuttons                 |
| D9, D8, D7            | Kabel Schraubterminal Rechts                    | Eingänge für falsche Kabel               |
| D6                    | Kabel Schraubterminal Rechts                    | Eingang Deaktivierungskabel              |


