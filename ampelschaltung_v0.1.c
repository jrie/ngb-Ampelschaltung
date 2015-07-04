/*
 *
 *  Ampelschaltung v0.1 by theSplit @ 23.04.15
 *  for the new gulli board coding competition
 *
 */
 
// C-Header Datei Import
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
 
// Funktions Deklaration
void schliesseSchaltung();
void ampelStatus(unsigned int ampelIndex);
 
void erstelleAmpel(unsigned int richtung);
void verbindeAmpel(unsigned int ampelIndex1, unsigned int ampelIndex2);
 
 
// Definition der Ampelzustände
enum ampelZustand {
    AUSGESCHALTET = 0,
    GRUEN = 1,
    GELB = 2,
    ROT = 3,
    DEFEKT_GRUEN = -1,
    DEFEKT_GELB = -2,
    DEFEKT_ROT = -3,
    DEFEKT = -4
};
 
// Ampel Phasen-Dauer Definition, sollten durch 500 teilbar sein, Angabe in Sekunden
// aufgrund der Berechnunsschrittgröße
enum ampelZeiten {
    PHASE_GRUEN = 6000,
    PHASE_GELB = 2500,
    PHASE_ROT = 7000,
    PHASE_ROT_ZUSATZ = 1500 // Die Extra Zeit die eine Ampel nach einer aktiven Rotphase warten soll bevor die Straße freigegeben wird
};
 
// Definition der Himmelsrichtungen für Erstellung, nur Orientierungsangabe
enum ampelRichtung {
    NORD = 0,
    OST = 1,
    SUED = 2,
    WEST = 3
};
 
// Ampel Definition
typedef struct ampel {
    int phase = 3;
    int vorgaengerPhase = 3;
    unsigned int index = 0;
    unsigned int gruenPhase = 0;
    unsigned int gelbPhase = 0;
    unsigned int rotPhase = 0;
    unsigned int warteZeit = 0;
    unsigned int warteZeitLimit = 0;
    unsigned int prioritaet = 0;
    unsigned int richtung = 0;
    int autosInWarteschlange = 0;
    unsigned int inBetriebnahmen = 0;
    struct ampel* partnerAmpel;
};
 
// Variablen Deklaration
static unsigned int ampelAnzahl = 0;
static ampel *ampelListe;
static int aktiveAmpel = -1;
static int aktuelleRunde = 0;
static int berechneRunden;
 
int main() {
    // Funktion aufrufen die Speicher freigibt
    atexit(schliesseSchaltung);
 
    printf("Verkehrsampeln in Erstellung: %d\n", 4);
    erstelleAmpel(NORD);
    erstelleAmpel(OST);
    erstelleAmpel(SUED);
    erstelleAmpel(WEST);
 
    // Die Partner-Ampel verbinden
    verbindeAmpel(0, 2);
    verbindeAmpel(1, 3);
 
    // Hauptschleife
    while (true) {
 
        // Eingabe der Schritte der Berechnung in 500ms Schritten * 2
        printf("Wieviele Runden berechnen (0 = Abbruch)\n");
        scanf("%d", &berechneRunden);
        printf("Runden: %d\n", berechneRunden);
 
        // Wenn 0 beenden      
        if (berechneRunden <= 0) {
            break;
        }
 
        aktuelleRunde = 0;
 
        // Berechnungsschleife und Ampeln durchschalten, Zeiten erhöhen, Logik
        while (aktuelleRunde < (berechneRunden * 2)) {
 
            // Fahrzeuge bei Ampeln erhöhen nach Zufall und
            // Wartezeit um 500ms erhöhen
            for (int i = 0; i < ampelAnzahl; i++) {
 
                // Überlauf verhindern, nicht weiter relevant
                if (ampelListe[i].autosInWarteschlange > 5000) {
                    ampelListe[i].autosInWarteschlange = 0;
                }
 
                // Fahrzeuge zufällig an Ampel erhöhen
                ampelListe[i].autosInWarteschlange += (rand() % 2);
                ampelListe[i].warteZeit += 500;
 
            }
 
            int hoechsterIndex = 0;
            int hoechstePrioritaet = ampelAnzahl > 0 ? ampelListe[0].prioritaet : 0;
            for (int i = 0; i < ampelAnzahl; i++) {
                if (ampelListe[i].prioritaet > hoechstePrioritaet) {
                    hoechsterIndex = i;
                    hoechstePrioritaet = ampelListe[i].prioritaet;
                }
            }
           
            for (int i = 0; i < ampelAnzahl; i++) {
                if (aktiveAmpel == -1 || ampelListe[i].warteZeit >= ampelListe[i].warteZeitLimit) {
 
                    // Wenn wir eine aktive Ampel haben, ist dies die aktive bzw. deren Partnerampel?
                    if (aktiveAmpel != -1 && (aktiveAmpel != i && (ampelListe[i].partnerAmpel != NULL && ampelListe[i].partnerAmpel->index != i))) {
                        ampelListe[i].prioritaet++;
                        continue;
                    }
 
                    // Keine aktive Ampel, ist dies eine der Ampeln mit der höchsten Priorität bzw. deren Partnerampel falls vorhanden?
                    if (aktiveAmpel == -1 && (hoechsterIndex != i && (ampelListe[i].partnerAmpel != NULL && ampelListe[i].partnerAmpel->index != hoechsterIndex))) {
                        ampelListe[i].prioritaet++;
                        continue;
                    }
 
                    // Aktive Ampel setzen und eventuelle Partnerampel
                    // einspannen
                    aktiveAmpel = i;
                    ampelListe[i].warteZeit = 0;
                    ampelListe[i].prioritaet = 0;
                   
                    if (ampelListe[i].partnerAmpel != NULL) {
                        ampelListe[i].partnerAmpel->warteZeit = 0;
                        ampelListe[i].partnerAmpel->prioritaet = 0;
                    }
 
                    // Wechsele die Phasen der Ampel nach aktuellem Farbenstand
                    switch (ampelListe[i].phase) {
                        case ROT:
                            // Phase Rot
                            if (ampelListe[i].vorgaengerPhase == GELB) {
                                // Wenn die Ampel vorher auf Gelb stand,
                                // also die Rotphase + Wartezeit rum ist,
                                // platz für die nächste Ampel geben
                                aktiveAmpel = -1;
                                ampelListe[i].warteZeit = 0;
                                ampelListe[i].prioritaet = 0;
                                ampelListe[i].vorgaengerPhase = ROT;
 
                                if (ampelListe[i].partnerAmpel != NULL) {
                                    ampelListe[i].partnerAmpel->warteZeit = 0;
                                    ampelListe[i].partnerAmpel->prioritaet = 0;
                                    ampelListe[i].partnerAmpel->vorgaengerPhase = ROT;
                                }
                               
                                continue;
                            }
 
                            // Die Ampel schaltet von Rot auf Gelb und wird somit in Betrieb genommmen
                            ampelListe[i].inBetriebnahmen++;
 
                            // Schutz vor Überlauf, nicht relevant
                            if (ampelListe[i].inBetriebnahmen > 10000) {
                                ampelListe[i].inBetriebnahmen = 0;
                            }
 
                            // Partnerampel auch auf Betrieben setzen
                            if (ampelListe[i].partnerAmpel != NULL) {
                                ampelListe[i].partnerAmpel->inBetriebnahmen = ampelListe[i].inBetriebnahmen;
                            }
 
                            // Gelb als aktuelle Phase setzen
                            ampelListe[i].phase = GELB;
                            ampelListe[i].warteZeitLimit = PHASE_GELB;
 
                            // Von Rot auf Gelb geschaltet, Vorgängerphase ist nun Rot
                            ampelListe[i].vorgaengerPhase = ROT;
 
                            // Das selbe für die Partnerampel machen
                            if (ampelListe[i].partnerAmpel != NULL) {
                                ampelListe[i].partnerAmpel->phase = GELB;
                                ampelListe[i].partnerAmpel->warteZeitLimit = PHASE_GELB;
                                ampelListe[i].partnerAmpel->vorgaengerPhase = ROT;
                            }
                            break;
                        case GELB:
                            // Phase ist Gelb
                            if (ampelListe[i].vorgaengerPhase == GRUEN) {
                                // Phase von vorher Grün auf Rot setzen,
                                // neues WarteZeitLimit setzen
                                ampelListe[i].phase = ROT;
                                ampelListe[i].warteZeitLimit = PHASE_ROT_ZUSATZ;
                                ampelListe[i].vorgaengerPhase = GELB;
 
                                // Für die Partnerampel auch wenn gesetzt
                                if (ampelListe[i].partnerAmpel != NULL) {
                                    ampelListe[i].partnerAmpel->phase = ROT;
                                    ampelListe[i].partnerAmpel->warteZeitLimit = PHASE_ROT_ZUSATZ;
 
                                    ampelListe[i].partnerAmpel->vorgaengerPhase = GELB;
                                }
                               
                            } else if (ampelListe[i].vorgaengerPhase == ROT) {
                                // Phase war Rot, nun auf Grün schalten,
                                // neue WarteZeitLimit setzen
                                ampelListe[i].phase = GRUEN;
                                ampelListe[i].warteZeitLimit = PHASE_GRUEN;
                               
                                ampelListe[i].vorgaengerPhase = GELB;
                               
                                if (ampelListe[i].partnerAmpel != NULL) {
                                    ampelListe[i].partnerAmpel->phase = GRUEN;
                                    ampelListe[i].partnerAmpel->warteZeitLimit = PHASE_GRUEN;
 
                                    ampelListe[i].partnerAmpel->vorgaengerPhase = GELB;
                                }
 
                                // Lasse 100 Fahrzeuge passieren
                                // Der Wert ist nur so hoch weil die Berechnungsschritte
                                // so gering sind und sich sonst Fahrzeuge anstauen
                                ampelListe[i].autosInWarteschlange -= 80;
                                if (ampelListe[i].autosInWarteschlange < 0) {
                                    ampelListe[i].autosInWarteschlange = 0;
                                }
 
                                ampelListe[i].partnerAmpel->autosInWarteschlange -= 80;
                                if (ampelListe[i].partnerAmpel->autosInWarteschlange < 0) {
                                    ampelListe[i].partnerAmpel->autosInWarteschlange = 0;
                                }
                               
                            }
                            break;
                        case GRUEN:
                            // Von Grün auf Gelb schalten
                            // Vorgängerphase setzen, neues WarteZeitLimit setzen
                            ampelListe[i].phase = GELB;
                            ampelListe[i].vorgaengerPhase = GRUEN;
                           
                            ampelListe[i].warteZeitLimit = PHASE_GELB;
 
                            if (ampelListe[i].partnerAmpel != NULL) {
                                ampelListe[i].partnerAmpel->phase = GELB;
                                ampelListe[i].partnerAmpel->warteZeitLimit = PHASE_GELB;
                               
                                ampelListe[i].partnerAmpel->vorgaengerPhase = GRUEN;
                            }
                            break;
                    }
                }
            }
 
            aktuelleRunde++;
           
        }
 
        // Ampelstatus ausgeben oder beenden
        printf("Ampelstatus ausgeben [j/n/0 = Abbruch]\n");
        char ampelStatusAusgeben;
        scanf("%s", &ampelStatusAusgeben);
 
        if (ampelStatusAusgeben == 'j') {
            for (unsigned int i = 0; i < ampelAnzahl; i++) {
                ampelStatus(i);
            }
        } else if (ampelStatusAusgeben == '0') {
            break;
        }
    }
   
    return (EXIT_SUCCESS);
}
 
// Eine Ampel erstellen
void erstelleAmpel(unsigned int richtung) {
    // Test ob die Himmelsrichtung in einem gültigen Bereich liegt
    // ist nicht wirklich relevant und könnte weggelassen werden,
    // da die Ampeln selbst verwaltend sind
    if (!(richtung >= 0 && richtung <= 3) ) {
        printf("Keine gültige Himmelsrichtung für Ampel %d eingegeben\n", ampelAnzahl);
        return;
    }
 
    // Speicher für neue Ampel erfragen
    ampelListe = (ampel*) realloc(ampelListe, sizeof(ampel) * (ampelAnzahl + 1));
 
    // Kein Speicher => Programfehler und beenden
    if (ampelListe == NULL) {
        printf("Konnte kein Speicher für die Ampelschaltungen belegen... Abbruch.");
        schliesseSchaltung();
    }
 
    // Die Ampel anlegen
    ampelListe[ampelAnzahl].index = ampelAnzahl;
    ampelListe[ampelAnzahl].phase = ROT;
    ampelListe[ampelAnzahl].vorgaengerPhase = ROT;
    ampelListe[ampelAnzahl].gruenPhase = 6000;
    ampelListe[ampelAnzahl].gelbPhase = 2500;
    ampelListe[ampelAnzahl].rotPhase = 7000;
    ampelListe[ampelAnzahl].warteZeit = PHASE_ROT;
    ampelListe[ampelAnzahl].warteZeitLimit = PHASE_ROT+2000;
    ampelListe[ampelAnzahl].prioritaet = 0;
    ampelListe[ampelAnzahl].richtung = richtung;
    ampelListe[ampelAnzahl].autosInWarteschlange = 0;
    ampelListe[ampelAnzahl].inBetriebnahmen = 0;
    ampelListe[ampelAnzahl].partnerAmpel = NULL;
 
    ampelAnzahl++;
    return;
}
 
// Zwei Ampeln verbinden, Partnerampel setzen wenn gegenüber
void verbindeAmpel(unsigned int ampelIndex1, unsigned int ampelIndex2) {
    if (ampelIndex1 > ampelAnzahl || ampelIndex2 > ampelAnzahl) {
        return;
    }
 
    ampelListe[ampelIndex1].partnerAmpel = &ampelListe[ampelIndex2];
    ampelListe[ampelIndex2].partnerAmpel = &ampelListe[ampelIndex1];
}
 
// Ampelstatus ausgeben
void ampelStatus(unsigned int ampelIndex) {
    ampel Ampel = ampelListe[ampelIndex];
    printf("Ampel-Status:\n");
 
    char *ampelStatusPhase;
    char *ampelStatusRichtung;
   
    switch (Ampel.phase) {
        case AUSGESCHALTET:
            ampelStatusPhase = "Ausgeschaltet";                    
            break;
        case GRUEN:
            ampelStatusPhase = "Grün";
            break;
        case GELB:
            ampelStatusPhase = "Gelb";
            break;
        case ROT:
            ampelStatusPhase = "Rot";
            break;
        case DEFEKT_GRUEN:
            ampelStatusPhase = "Defekt Grün";
            break;
        case DEFEKT_GELB:
            ampelStatusPhase = "Defekt Gelb";
            break;
        case DEFEKT_ROT:
            ampelStatusPhase = "Defekt Rot";
            break;
        case DEFEKT:
        default:
            ampelStatusPhase = "genereller defekt";
            break;
    }
 
    switch (Ampel.richtung) {
        case NORD:
            ampelStatusRichtung = "Nord";
            break;
        case OST:
            ampelStatusRichtung = "Ost";
            break;
        case SUED:
            ampelStatusRichtung = "Süd";
            break;
        case WEST:
            ampelStatusRichtung = "West";
            break;
    }
 
    printf("Ampelindex: %i\n", ampelIndex);
    printf("Richtung: %s\n", ampelStatusRichtung);
    printf("Phase: %s\n", ampelStatusPhase);
    printf("In Betriebnahmen: %d\n", Ampel.inBetriebnahmen);
    printf("Warezeit in Sekunden: %.2f\n", (float) Ampel.warteZeit / 1000);
    printf("Aktuelle Priorität: %d\n", Ampel.prioritaet);
    printf("Partner-Ampel Index: %d\n", Ampel.partnerAmpel != NULL ? Ampel.partnerAmpel->index : -1);
    printf("Autos in Schlange: %d\n\n", Ampel.autosInWarteschlange);
 
    return;
}
 
// Anwendung beenden
void schliesseSchaltung() {
    free(ampelListe);
    exit(EXIT_SUCCESS);
}
