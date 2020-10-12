# Bedienungsanleitung "Musikwürfel"

## Einschalten 

Der Musikwürfel kann entweder über den USB-micro Anschluss oder über den eingebauten Akku betrieben werden.
Im Akkubetrieb schaltet der Ein-/Ausschalter (nach vorne) das Gerät ein. Mit USB wird das Gerät mit Strom versorgt, sobald das Kabel eingesteckt ist.
Kurz nach dem Einschalten leuchten die 16 Tasten auf der Front in individuellen Farben auf. Das bedeutet, dass das Gerät bereit ist und die Musik ausgewählt werden kann.

Bemerkung: Je nach Konfiguration der Firmware leuchten die Fronttasten konstant, werden langsam auf-/abgeblendet oder wandern als Lauflicht.


## Musik wählen

Mit Druck auf eine der 16 Fronttasten kann das hinterlegte Musikstück bzw. das Album abgespielt werden. Bei einem Album kann durch erneuten Druck auf das nächste Stück gewechselt werden. Solange ein Musikstück spielt, blinkt die entsprechende Taste langsam.
Beim Abspielen eines Stücks kann durch eine beliebige Taste jederzeit auf das neue Album gewechselt werden.
Wenn ein Musikstück fertig ist, wird innerhalb eines Albums automatisch auf das nächste gewechselt. Nach dem letzten Stück eines Albums geht das Gerät in den Grundzustand (alle Tasten leuchten).

Falls einer Taste kein Musikstück/Album hinterlegt ist, geht das Gerät sofort wieder in den Grundzustand (alle Tasten leuchten).

Bemerkung: Innerhalb eines Albums kann nur vorwärts "geblättert" werden. Um wieder an den Anfang zu gelangen, muss man sich durch das ganze Album drücken, oder besser das Abspielen durch langes Drücken auf den Drehregler beenden und das Album erneut auswählen.

### Musik mit Figur auswählen

In der Konfiguration des Gerätes können NFC-Ids mit einem Musikstück bzw. Album verknüpft werden. Wenn eine Figur oder ein anderer Gegenstand mit einem entsprechenden NFC-Tag auf den Deckel des Musikwürfels gelegt wird, wird das entsprechende Stück abgespielt.

Bemerkung: Beim Abspielen eines Stücks über diese Funktion blinkt immer die erste Fronttaste. Bei einem Album kann mit der ersten Taste auf das nächste Musikstück gewechselt werden.


## Abspielen pausieren oder abbrechen

Durch kurzen Druck auf den Drehregler wird das aktuelle Stück pausiert. Das wird durch langsameres Blinken der aktuellen Fronttaste dargestellt. Ausserdem blinkt der Drehregler blau. Erneutes Drücken des Drehreglers spielt das Musikstück weiter ab.

Durch langer Druck auf den Drehregler (ca. 1 sek) wird das aktuelle Stück abgebrochen und das Gerät geht zurück in den Grundzustand (alle Tasten leuchten).

Bemerkung: Der Pausenzustand hat immer noch einen erhöhten Stromverbrauch, da der Musikdecoder immer läuft und das Musikstück geladen ist. Deshalb wird nach einer konfigurierbaren Zeit (aktuell 60 min) das Gerät in den Standby-Modus gesetzt, worauf eine Fortsetzung des Stücks nicht mehr möglich ist.


## Lautstärke regeln

Mit dem Drehregler kann die Lautstärke geregelt werden. Minimale und maximale Lautstärke können für Lautsprecher- und Kopfhörerbetrieb getrennt in der Konfiguration festgelegt werden.

Bemerkung: Wenn das Gerät ausgeschaltet oder in den Standby-Modus geht, wird die Lautstärke auf den initialen Wert zurückgesetzt.


## Gerät in Standby-Modus setzen

Im Grundmodus (alle Tasten leuchten) kann das Gerät durch langes Drücken (ca. 1 sek) auf den Drehregler in den Standby-Modus gesetzt werden. Nach einiger Zeit im Grundmodus (aktuell 15 min) schaltet sich das Gerät automatisch in den Standby-Modus. Hier werden alle Funktionen abgestellt und der Stromverbrauch wird reduziert. Im Standbymodus blinkt der Drehregler ca. alle 8 sek.

Das Abspielen eines Musikstücks wird ebenfalls durch langes Drücken (ca. 1 sek) gestoppt. Um nicht versehentlich die Funktion zu aktivieren, muss der Drehregler dann losgelassen und erneut lang gedrückt werden um in den Standby-Modus zu gelangen.

Durch Drücken einer beliebigen Fronttaste wird das Gerät wieder aktiviert und gelangt in den Grundmodus (alle Tasten leuchten). Dabei ist aber noch kein Album/Stück ausgewählt, dazu muss erneut eine der Fronttasten gedrückt werden.


## Akku aufladen

Um den Akku aufzuladen, muss das Gerät über einen USB-micro Anschluss mit Strom versorgt werden. *Achtung: Der Akku wird nur geladen, wenn das Gerät mit dem Hauptschalter eingeschalten ist!*

Es muss ein USB-micro Stecker mit einer langen Zunge verwendet werden um einen sicheren Kontakt zu gewährleisten.

Bemerkung: Der Ladestrom ist auf 100 mA begrenzt. Für einen grossen Akku (z.B. 2500 mAh) muss deshalb mit einer langen Ladezeit von z.B. 25 Std gerechnet werden.


## Musikstücke ablegen und verknüpfen

Dazu muss die MicroSD-Karte entnommen werden: Deckel auf der Rückseite entfernen, mit Finger auf die Karte *drücken* um sie zu entriegeln, dann entnehmen.

Die MicroSD kann direkt, über einen SD-Adapter oder ein Zusatzgerät gelesen/geschrieben werden.

- Musikstücke können in einem Verzeichnis als Album zusammengefasst und in beliebigen Unterverzeichnissen abgelegt sein.
- Alben/Musikstücke für die Tasten müssen in der Textdatei ```buttons.cfg``` festgelegt werden. 
- Alben/Musikstücke für Figuren mit NFC-Tags müssen mit ihrer NFC Id in der Textdatei ```nfc.cfg``` festgelegt werden.
- Die physische Reihenfolge der Musikstücke ist entscheidend, nicht die Sortierung nach Namen. Um das sicherzustellen, wird am besten ein Musikstück nach dem andern einzeln auf die SD-Karte geschrieben.
- Beim Einlesen eines neuen NFC-Tags wird die unbekannte Id in die Textdatei ```nfc.cfg``` geschrieben.

Bemerkung: In der aktuellen Version können nur max. 8 Zeichen pro Verzeichnis/Stück angegeben werden. Entweder wird der Name des Stücks auf 8 Zeichen gekürzt oder es wird der Kurzname des längeren Stücks ermittelt (Windows/Cmd mit ```dir /x```). Die Längenbeschränkung gilt auch für einzelne Verzeichnisnamen.

### Beispiel Konfigurationsfiles 
NFC Konfiguration ```nfc.cfg```:
```
6738BC0E=MIGROS/WICHTEL/ELI.MP3
B85D2D35=MIGROS/WICHTEL/FINN.MP3
```

Button Konfiguration ```buttons.cfg```:
```
3=GLOBI/FEUER
4=JAMADU
```

### Unterstützte Audio Decoder Formate

* MP3 CBR+VBR+ABR bis 320 kbps, 48 kHz
* Ogg Vorbis bis 48 kHz, 500 kbps
* AAC VBR, CBR bis 48 kHz, 576 kbps
* WMA (4.0/4.1/7/8/9) bis 48 kHz, 384 kbps
* FLAC bis 48 kHz, 24bit
* WAV (PCM, IMA_ADPCM, MPEGLAYER3)
* MIDI Format 0, max. Polyphonie 64

MP3 sollte ohne die Option 'joint stereo' kodiert werden.

MIDI Instrumente siehe [VS1053 Datenblatt](https://cdn-shop.adafruit.com/datasheets/vs1053.pdf) Seite 32


## Weitere Konfigurationen
- settings.cfg - wird im Moment nicht unterstützt
