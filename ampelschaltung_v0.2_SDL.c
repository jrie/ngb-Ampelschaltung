/*
 *
 *  Ampelschaltung v0.2b by theSplit @ 03.05.15
 *  for the new gulli board coding competition
 *
 */
 
// C-Header inkludieren
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
 
// SDL Header inkludieren
#include <SDL2/SDL.h>
#include <SDL2/SDL_timer.h>
#include <SDL2/SDL_events.h>
 
// Funktions Deklaration
void schliesseSchaltung();
void ampelStatus(unsigned int ampelIndex);
 
void erstelleAmpel(unsigned int richtung);
void verbindeAmpel(unsigned int ampelIndex1, unsigned int ampelIndex2);
void lasseAutoPassieren(unsigned int i);
 
// Definition der Ampelzustände
enum ampelZustand {
    AUSGESCHALTET = 0,
    GRUEN = 1,
    GELB = 2,
    GELB_ROT = 3,
    ROT = 4,
    DEFEKT_GRUEN = -1,
    DEFEKT_GELB = -2,
    DEFEKT_ROT = -3,
    DEFEKT = -4
};
 
// Ampel Phasen-Dauer Definition, sollten durch 500 teilbar sein, Angabe in Sekunden
// aufgrund der Berechnunsschrittgröße
enum ampelZeiten {
    PHASE_GRUEN = 6000,         // Dauer der Grün-Phase
    PHASE_GELB = 2500,          // Dauer der Gelb-Phase
    PHASE_ROT = 7000,           // Dauer der Rot-Phase
    PHASE_GELB_ROT = 1500,      // Dauer der Gelb-Rot Phase (Anfahrtssignal)  
    PHASE_ROT_ZUSATZ = 1500     // Extra Dauer nach aktiver Grün und Gelb Phase, in Rot-Phase, bevor die Kreuzung freigegeben wird
};
 
// Definition der Himmelsrichtungen für Erstellung, nur Orientierungsangabe
enum ampelRichtungen {
    NORD = 0,
    OST = 1,
    SUED = 2,
    WEST = 3
};
 
enum fahrtRichtungen {
    GERADEAUS = 0,
    LINKS = 1,
    RECHTS = 2
};
 
// Auto Definition
typedef struct fahrzeugFarbe {
    unsigned char r = 0;
    unsigned char g = 0;
    unsigned char b = 0;
};
 
typedef struct pkw {
    bool kannFahren = false;
    unsigned int fahrtRichtung = 0;
    unsigned int warteZeit = 0;
    int geschwindigkeit = 0;
    int ps = 0;
    int x = 0;
    int y = 0;
    fahrzeugFarbe farbe;
   
};
 
 
 
// Ampel Definition
typedef struct ampel {
    int phase = ROT;
    int vorgaengerPhase = ROT;
    unsigned int index = 0;
    unsigned int warteZeit = 0;
    unsigned int warteZeitLimit = 0;
    unsigned int prioritaet = 0;
    unsigned int richtung = 0;
    unsigned int inBetriebnahmen = 0;
    unsigned int autosPassiert = 0;
    int autosInWarteschlange = 0;
    pkw fahrzeugListe[100];
    ampel* partnerAmpel;
};
 
// Variablen Deklaration
static unsigned int ampelAnzahl = 0;
static ampel *ampelListe;
static int aktiveAmpel = -1;
static int aktuelleRunde = 0;
static int berechneRunden;
 
static unsigned int hoechsterIndex = 0;
static unsigned int hoechstePrioritaet = 0;
static unsigned int i, j = 0;
static unsigned int zufallsAmpel = 0;
 
// Zeichenvariablen
static int lineYStart = 0;
static int lineXStart = 0;
 
// SDL Deklaration
static SDL_Window *appWindow = NULL;
static SDL_Renderer *appRender = NULL;
 
static const int windowWidth = 640;
static const int windowHeight = 640;
 
 
int main() {
    // Beenden Funktion definieren die den Speicher freigibt
    atexit(schliesseSchaltung);
 
    // Verkehrsampeln erstellen
    printf("Verkehrsampeln in Erstellung: %d\n", 4);
    erstelleAmpel(NORD);
    erstelleAmpel(OST);
    erstelleAmpel(SUED);
    erstelleAmpel(WEST);
 
    // Die Partner-Ampel verbinden die gegenüber liegen
    verbindeAmpel(0, 2);
    verbindeAmpel(1, 3);
 
    // SDL Video initialisieren
    SDL_Init(SDL_INIT_VIDEO);
 
    appWindow = SDL_CreateWindow("Ampelschaltung by theSplit for ngb", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, windowWidth, windowHeight, 0);
    appRender = SDL_CreateRenderer(appWindow, -1, SDL_RENDERER_ACCELERATED);
    SDL_Event eventHandle;
 
    // Hauptschleife
    while (true) {
       
        if (SDL_PollEvent(&eventHandle)) {
            if (eventHandle.type == SDL_QUIT) {
                return(EXIT_SUCCESS);
            }
        }
 
        // Wartezeit um 500ms bei jeder Ampel erhöhen
        for (i = 0; i < ampelAnzahl; i++) {
            ampelListe[i].warteZeit += 15;
        }
 
        // Fahrzeuge zufällig an Ampel erhöhen
        if (rand() % 70 == 69) {
            zufallsAmpel = rand() % ampelAnzahl;
 
            ampelListe[zufallsAmpel].autosInWarteschlange++;
 
            // Auskommentiert um nicht ständig zu printen
            // printf("Autos in Schlange von Ampel-Index %d: %d\n", zufallsAmpel, ampelListe[zufallsAmpel].autosInWarteschlange);
 
            // Überlauf verhindern, nicht weiter relevant
            if (ampelListe[zufallsAmpel].autosInWarteschlange > 100) {
                ampelListe[zufallsAmpel].autosInWarteschlange = 100;
            } else {
                // Neues Auto für Ampel erstellen
                pkw fahrzeug;
                fahrzeug.fahrtRichtung = rand() % 3;
                fahrzeug.warteZeit = 0;
                fahrzeug.geschwindigkeit = 0;
                fahrzeug.ps = 140;
                fahrzeug.farbe.r = (unsigned char) rand() % 151 + 70;
                fahrzeug.farbe.g = (unsigned char) rand() % 151 + 70;
                fahrzeug.farbe.b = (unsigned char) rand() % 151 + 70;
                fahrzeug.kannFahren = false;
 
                // An Eckkoordinaten, außerhalb des Fensters, erstellen
                int baseX = 0;
                int baseY = 0;
                switch (ampelListe[zufallsAmpel].richtung) {
                    case OST:
                        baseX = windowWidth;
                        baseY = 270;
                        break;
                    case WEST:
                        baseX = -55;
                        baseY = 350;
                        break;
                    case NORD:
                        baseX = 280;
                        baseY = -55;
                        break;
                    case SUED:
                        baseX = 340;
                        baseY = windowHeight;
                        break;
                }
                fahrzeug.x = baseX;
                fahrzeug.y = baseY;
               
 
                // Fahrzeug in die Liste der Fahrzeuge kopieren (auf 100 begrenzt)
                memcpy(&ampelListe[zufallsAmpel].fahrzeugListe[ampelListe[zufallsAmpel].autosInWarteschlange-1], &fahrzeug, sizeof(pkw));
            }
        }
 
        // Welche Ampel hat die höchste Priorität und muss aktiviert werden?
        hoechsterIndex = 0;
        hoechstePrioritaet = ampelAnzahl > 0 ? ampelListe[0].prioritaet : 0;
        for (i = 0; i < ampelAnzahl; i++) {
            if (ampelListe[i].prioritaet > hoechstePrioritaet) {
                hoechsterIndex = i;
                hoechstePrioritaet = ampelListe[i].prioritaet;
            }
        }
 
        // Ampelschaltung Logik
        for (i = 0; i < ampelAnzahl; i++) {
            if (aktiveAmpel == -1 || ampelListe[i].warteZeit >= ampelListe[i].warteZeitLimit) {
 
                // Wenn wir eine aktive Ampel haben, ist dies die aktive bzw. deren Partnerampel?
                // Falls nicht die Ampel Prioriät erhöhen
                if (aktiveAmpel != -1 && (aktiveAmpel != i && (ampelListe[i].partnerAmpel != NULL && ampelListe[i].partnerAmpel->index != i))) {
                    ampelListe[i].prioritaet++;
                    continue;
                }
 
                // Keine aktive Ampel, ist dies eine der Ampeln mit der höchsten Priorität bzw. deren Partnerampel falls vorhanden?
                // Falls nicht die Ampel Prioriät erhöhen
                if (aktiveAmpel == -1 && (hoechsterIndex != i && (ampelListe[i].partnerAmpel != NULL && ampelListe[i].partnerAmpel->index != hoechsterIndex))) {
                    ampelListe[i].prioritaet++;
                    continue;
                }
 
                // Aktive Ampel setzen und eventuelle Partnerampel
                // einspannen falls gesetzt
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
                            ampelListe[i].warteZeitLimit = PHASE_ROT;
                           
 
                            if (ampelListe[i].partnerAmpel != NULL) {
                                ampelListe[i].partnerAmpel->warteZeit = 0;
                                ampelListe[i].partnerAmpel->prioritaet = 0;
                                ampelListe[i].partnerAmpel->vorgaengerPhase = ROT;
                                ampelListe[i].partnerAmpel->warteZeitLimit = PHASE_ROT;
                            }
                           
                            continue;
                        }
 
                        // Die Ampel schaltet von Rot auf Gelb und wird somit in Betrieb genommmen
                        ampelListe[i].inBetriebnahmen++;
 
                        // Schutz vor Überlauf, nicht relevant
                        if (ampelListe[i].inBetriebnahmen == 1000000) {
                            ampelListe[i].inBetriebnahmen = 0;
                        }
 
                        // Partnerampel auch auf Betrieben setzen
                        if (ampelListe[i].partnerAmpel != NULL) {
                            ampelListe[i].partnerAmpel->inBetriebnahmen = ampelListe[i].inBetriebnahmen;
                        }
 
                        // Gelb-Rot als aktuelle Phase setzen
                        ampelListe[i].phase = GELB_ROT;
                        ampelListe[i].warteZeitLimit = PHASE_GELB_ROT;
 
                        // Von Rot auf Gelb geschaltet, Vorgängerphase ist nun Rot
                        ampelListe[i].vorgaengerPhase = ROT;
 
                        // Das selbe für die Partnerampel machen
                        if (ampelListe[i].partnerAmpel != NULL) {
                            ampelListe[i].partnerAmpel->phase = GELB_ROT;
                            ampelListe[i].partnerAmpel->warteZeitLimit = PHASE_GELB_ROT;
                            ampelListe[i].partnerAmpel->vorgaengerPhase = ROT;
                        }
                        break;
                    case GELB_ROT:
                        // Phase ist Gelb-Rot auf Grün setzen
                        ampelListe[i].phase = GRUEN;
                        ampelListe[i].warteZeitLimit = PHASE_GRUEN;
                        ampelListe[i].vorgaengerPhase = GELB_ROT;
 
                        // Für die Partnerampel auch wenn gesetzt
                        if (ampelListe[i].partnerAmpel != NULL) {
                            ampelListe[i].partnerAmpel->phase = GRUEN;
                            ampelListe[i].partnerAmpel->warteZeitLimit = PHASE_GRUEN;
 
                            ampelListe[i].partnerAmpel->vorgaengerPhase = GELB_ROT;
                        }
 
                        // Lasse Autos über die Ampel fahren (8)
                        // Nicht mehr nötig in SDL Version
                        //lasseAutoPassieren(i);
 
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
 
                            // Lasse Autos über die Ampel fahren (8)
                            // Nicht mehr nötig in SDL Version
                            //lasseAutoPassieren(i);
                           
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
 
        // SDL Zeichenroutinen
        SDL_Rect windowRect = {0, 0, windowWidth, windowHeight };
        SDL_SetRenderDrawColor(appRender, 110, 110, 110, 255);
        SDL_RenderClear(appRender);
 
        // Straßen zeichnen
        SDL_SetRenderDrawColor(appRender, 30, 30, 30, 255);
        SDL_Rect strasseNordSued = {239, 0, 161, windowHeight};
        SDL_Rect strasseWestOst = {0, 239, windowWidth, 161};
        SDL_RenderFillRect(appRender, &strasseNordSued);
        SDL_RenderFillRect(appRender, &strasseWestOst);
 
        // Straßenlinien zeichnen
        SDL_SetRenderDrawColor(appRender, 255, 255, 255, 255);
 
        lineYStart = 10;
        for (i = 0; i < 6; i++) {
            SDL_RenderDrawLine(appRender, 320, lineYStart, 320, lineYStart+15);
            lineYStart += 40;
        }
 
        lineYStart = 410;
        for (i = 0; i < 6; i++) {
            SDL_RenderDrawLine(appRender, 320, lineYStart, 320, lineYStart+15);
            lineYStart += 40;
        }
 
        lineXStart = 10;
        for (i = 0; i < 6; i++) {
            SDL_RenderDrawLine(appRender, lineXStart, 320, lineXStart+15, 320);
            lineXStart += 40;
        }
 
        lineXStart = 410;
        for (i = 0; i < 6; i++) {
            SDL_RenderDrawLine(appRender, lineXStart, 320, lineXStart+15, 320);
            lineXStart += 40;
        }
 
        // Ampeln zeichnen
        SDL_Rect zeichenAmpel = {180, 100, 40, 100};
        SDL_Rect ampelLicht = {8, 10, 20, 20};
 
        for (i = 0; i < ampelAnzahl; i++) {
            SDL_SetRenderDrawColor(appRender, 80, 80, 80, 255);
            SDL_Rect ampelLicht1 = {0, 0, 25, 25};
            SDL_Rect ampelLicht2 = {0, 0, 25, 25};
            SDL_Rect ampelLicht3 = {0, 0, 25, 25};
           
            switch(ampelListe[i].richtung) {
                case NORD:
                    zeichenAmpel = {180, 100, 40, 100};
                    break;
                case OST:
                    zeichenAmpel = {420, 100, 40, 100};
                    break;
                case SUED:
                    zeichenAmpel = {420, 420, 40, 100};
                    break;
                case WEST:
                    zeichenAmpel = {180, 420, 40, 100};
                    break;
            }
 
            SDL_RenderFillRect(appRender, &zeichenAmpel);
            ampelLicht1.x = zeichenAmpel.x + ampelLicht.x;
            ampelLicht1.y = zeichenAmpel.y + ampelLicht.y;
            ampelLicht2.x = zeichenAmpel.x + ampelLicht.x;
            ampelLicht2.y = zeichenAmpel.y + ampelLicht.y + 30;
            ampelLicht3.x = zeichenAmpel.x + ampelLicht.x;
            ampelLicht3.y = zeichenAmpel.y + ampelLicht.y + 60;
 
            // Rot zeichnen
            if (ampelListe[i].phase == ROT || ampelListe[i].phase == GELB_ROT) {
                SDL_SetRenderDrawColor(appRender, 255, 0, 0, 255);
            } else {
                SDL_SetRenderDrawColor(appRender, 0, 0, 0, 255);
            }
            SDL_RenderFillRect(appRender, &ampelLicht1);
           
            // Gelb zeichnen
            if (ampelListe[i].phase == GELB || ampelListe[i].phase == GELB_ROT) {
                SDL_SetRenderDrawColor(appRender, 255, 255, 0, 255);
            } else {
                SDL_SetRenderDrawColor(appRender, 0, 0, 0, 255);
            }
            SDL_RenderFillRect(appRender, &ampelLicht2);
 
            // Grün zeichnen
            if (ampelListe[i].phase == GRUEN) {
                SDL_SetRenderDrawColor(appRender, 0, 255, 0, 255);
            } else {
                SDL_SetRenderDrawColor(appRender, 0, 0, 0, 255);
            }
            SDL_RenderFillRect(appRender, &ampelLicht3);
        }
 
        // Fahrzeuge bewegen und aussortieren wenn außerhalb des Fensterbereichs
        for (i = 0; i < ampelAnzahl; i++) {
            int offsetX = 0;
            int offsetY = 0;
            unsigned int passedIndex = 0;
            switch (ampelListe[i].richtung) {
                case NORD:
                    // Berechnung für Norden
                    for (j = 0; j < ampelListe[i].autosInWarteschlange; j++) {
                        // Animiere die Autos hinein deren y kleiner als 170 ist (Kreuzungslinie)
                        if (ampelListe[i].fahrzeugListe[j].y < 170) {
                            // Haben wir einen Vorgänger, setze den Grenzwert y - mit y letztes Fahrzeug (Stoßstange an Stoßstange)
                            if (j > 0 && ampelListe[i].fahrzeugListe[j].y + 55 < ampelListe[i].fahrzeugListe[j-1].y) {
                                ampelListe[i].fahrzeugListe[j].y += 2;
                            } else if (j == 0) {
                                ampelListe[i].fahrzeugListe[j].y += 2;
                            }
                        } else if (ampelListe[i].fahrzeugListe[j].y > 190) {
                            // Ist das Fahrzeug schon über der Kreuzung, animiere weiter
                            ampelListe[i].fahrzeugListe[j].y += 2;
                        } else if (ampelListe[i].phase == GRUEN || ampelListe[i].phase == GELB_ROT || (ampelListe[i].phase == GELB && ampelListe[i].fahrzeugListe[j].y > 180)) {
                            // Ist das Auto noch nicht über die Kreuzung, achte auf die Ampelphasen oder überfahre wenn noch im gültigen Bereich
                            ampelListe[i].fahrzeugListe[j].y += 2;
                        }
 
                        // Ist das Auto außerhalb des Fensters animiert?
                        // Nimm den Index zum löschen auf
                        if (ampelListe[i].fahrzeugListe[j].y > windowHeight) {
                            passedIndex++;
                        }
                    }
 
                    break;
                case OST:
                    // Gleiche Berechnung für Osten
                    for (j = 0; j < ampelListe[i].autosInWarteschlange; j++) {
                        if (ampelListe[i].fahrzeugListe[j].x > 430) {
                            if (j > 0 && ampelListe[i].fahrzeugListe[j].x - 55 > ampelListe[i].fahrzeugListe[j-1].x) {
                                ampelListe[i].fahrzeugListe[j].x -= 2;
                            } else if (j == 0) {
                                ampelListe[i].fahrzeugListe[j].x -= 2;
                            }
                        } else if (ampelListe[i].fahrzeugListe[j].x < 320) {
                            ampelListe[i].fahrzeugListe[j].x -= 2;
                        } else if (ampelListe[i].phase == GRUEN || ampelListe[i].phase == GELB_ROT || (ampelListe[i].phase == GELB && ampelListe[i].fahrzeugListe[j].x < 390)) {
                            ampelListe[i].fahrzeugListe[j].x -= 2;
                        }
 
                        if (ampelListe[i].fahrzeugListe[j].x < -55) {
                            passedIndex++;
                        }
                    }
                    break;
                case SUED:
                    // Gleiche Berechnung für Süden
                    for (j = 0; j < ampelListe[i].autosInWarteschlange; j++) {
                        if (ampelListe[i].fahrzeugListe[j].y > 430) {
                            if (j > 0 && ampelListe[i].fahrzeugListe[j].y - 55 > ampelListe[i].fahrzeugListe[j-1].y) {
                                ampelListe[i].fahrzeugListe[j].y -= 2;
                            } else if (j == 0) {
                                ampelListe[i].fahrzeugListe[j].y -= 2;
                            }
                        } else if (ampelListe[i].fahrzeugListe[j].y < 320) {
                            ampelListe[i].fahrzeugListe[j].y -= 2;
                        } else if (ampelListe[i].phase == GRUEN || ampelListe[i].phase == GELB_ROT || (ampelListe[i].phase == GELB && ampelListe[i].fahrzeugListe[j].y < 390)) {
                            ampelListe[i].fahrzeugListe[j].y -= 2;
                        }
 
                        if (ampelListe[i].fahrzeugListe[j].y < -55) {
                            passedIndex++;
                        }
                    }
                    break;
                case WEST:
                    // Gleiche Berechnung für Westen
                    for (j = 0; j < ampelListe[i].autosInWarteschlange; j++) {
                        if (ampelListe[i].fahrzeugListe[j].x < 170) {
                            if (j > 0 && ampelListe[i].fahrzeugListe[j].x + 55 < ampelListe[i].fahrzeugListe[j-1].x) {
                                ampelListe[i].fahrzeugListe[j].x += 2;
                            } else if (j == 0) {
                                ampelListe[i].fahrzeugListe[j].x += 2;
                            }
                        } else if (ampelListe[i].fahrzeugListe[j].x > 190) {
                            ampelListe[i].fahrzeugListe[j].x += 2;
                        } else if (ampelListe[i].phase == GRUEN || ampelListe[i].phase == GELB_ROT || (ampelListe[i].phase == GELB && ampelListe[i].fahrzeugListe[j].x > 200)) {
                            ampelListe[i].fahrzeugListe[j].x += 2;
                        }
 
                        if (ampelListe[i].fahrzeugListe[j].x > windowWidth) {
                            passedIndex++;
                        }
                    }
                    break;
            }
 
            // Anzahl der Fahrzeuge die das Fenster verlassen haben von Schlange entfernen
            if (passedIndex > 0) {
                memmove(&ampelListe[i].fahrzeugListe, &ampelListe[i].fahrzeugListe[passedIndex], sizeof(pkw) * (100 - passedIndex));
                ampelListe[i].autosInWarteschlange -= passedIndex;
                ampelListe[i].autosPassiert += passedIndex;
 
                // Überlauf Schutz
                if (ampelListe[i].autosPassiert > 250000) {
                    ampelListe[i].autosPassiert = 0;
                }
            }
 
           
        }
 
        // Fahrzeuge rendern
        for (i = 0; i < ampelAnzahl; i++) {
            ampel aktiveAmpel = ampelListe[i];
            int offsetX = 0;
            int offsetY = 0;
            SDL_Rect zeichenFahrzeug = {0, 0, 0, 0};
 
            switch (aktiveAmpel.richtung) {
                case NORD:
                case SUED:
                    offsetY = 20;
                    break;
                case OST:
                case WEST:
                    offsetX = 20;
                    break;
            }
 
            for (j = 0; j < aktiveAmpel.autosInWarteschlange; j++) {
                SDL_SetRenderDrawColor(appRender, aktiveAmpel.fahrzeugListe[j].farbe.r, aktiveAmpel.fahrzeugListe[j].farbe.g, aktiveAmpel.fahrzeugListe[j].farbe.b, 255);
                zeichenFahrzeug.x = aktiveAmpel.fahrzeugListe[j].x;
                zeichenFahrzeug.y = aktiveAmpel.fahrzeugListe[j].y;
                zeichenFahrzeug.w = 25 + offsetX;
                zeichenFahrzeug.h = 25 + offsetY;
                SDL_RenderFillRect(appRender, &zeichenFahrzeug);
            }
        }
 
        // Fenster aktualisieren
        SDL_RenderPresent(appRender);
 
        // Verzögerung von 10 Millisekunden
        SDL_Delay(10);
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
 
// Funktion um Autos über die Ampeln fahren zu lassen
void lasseAutoPassieren(unsigned int i) {
    // Lasse 8 Fahrzeuge passieren
    ampelListe[i].autosInWarteschlange -= 8;
 
   
 
    if (ampelListe[i].autosInWarteschlange < 0) {
        ampelListe[i].autosPassiert += (8 + ampelListe[i].autosInWarteschlange);
        ampelListe[i].autosInWarteschlange = 0;
    } else {
        ampelListe[i].autosPassiert += 8;
    }
 
    // Schutz vor Überlauf
    if (ampelListe[i].autosPassiert > 150000) {
        ampelListe[i].autosPassiert = 0;
    }
 
    // Das gleiche für die Partnerampel
    if (ampelListe[i].partnerAmpel != NULL) {
        ampelListe[i].partnerAmpel->autosInWarteschlange -= 8;
        if (ampelListe[i].partnerAmpel->autosInWarteschlange < 0) {
            ampelListe[i].partnerAmpel->autosPassiert += (8 + ampelListe[i].partnerAmpel->autosInWarteschlange);
            ampelListe[i].partnerAmpel->autosInWarteschlange = 0;
        } else {
            ampelListe[i].partnerAmpel->autosPassiert += 8;
        }
 
        // Schutz vor Überlauf
        if (ampelListe[i].partnerAmpel->autosPassiert > 150000) {
            ampelListe[i].partnerAmpel->autosPassiert = 0;
        }
    }
 
    return;
}
 
// Ampelstatus ausgeben, nicht mehr in SDL Version verwendet
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
        case GELB_ROT:
            ampelStatusPhase = "Gelb-Rot";
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
    printf("Aktuelle Warezeit in Phase (Sekunden): %.2f\n", (float) Ampel.warteZeit / 1000);
    printf("Normale Wartezeit in Phase (Sekunden): %.2f\n", (float) Ampel.warteZeitLimit / 1000);
    printf("Aktuelle Priorität: %d\n", Ampel.prioritaet);
    printf("Partner-Ampel Index: %d\n", Ampel.partnerAmpel != NULL ? Ampel.partnerAmpel->index : -1);
    printf("Autos in Schlange: %d\n", Ampel.autosInWarteschlange);
    printf("Passierte Autos: %d\n\n", Ampel.autosPassiert);
 
    return;
}
 
// Anwendung beenden
void schliesseSchaltung() {
    // Speicher der Ampeln freigeben
    printf("Ampelspeicher freigeben...\n");
    free(ampelListe);
 
    // SDL aufräumen
    printf("SDL-Ressourcen freigeben...\n");
    SDL_DestroyRenderer(appRender);
    SDL_DestroyWindow(appWindow);
    SDL_Quit();
    printf("Ampelschaltung beendet.\n\n");
    exit(EXIT_SUCCESS);
}
