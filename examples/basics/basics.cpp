/* 
 *  =============================================================================================================================================
 *  Titre    : basics.cpp
 *  Auteur   : Thomas Broussard
 *  Projet   : Domokit
 *  Création : Octobre 2018
 *  ---------------------------------------------------------------------------------------------------------------------------------------------
 *  Description :
 *  Template pour créer les programmes Domokit au niveau industriel
 *  
 * ---------------------------------------------------------------------------------------------------------------------------------------------
 * Règles de conception : 
 * - Les boucles d'attentes (delay) ne sont pas autorisées
 * - La manipulation des entrées/sorties numériques se fera avec la librairie GPIO_esp8266.h
 * - Limiter au maximum l'utilisation des Serial.print (uniquement en mode Debug si possible)
 * - Prévoir un mode Debug et un mode Release 
 * =============================================================================================================================================
 */

// Dépendance(s)
#include "Arduino.h"

// Librairie Domokit
// lien : https://github.com/Thomas-Broussard/Domokit
#include "DomoKit.h"

// Librairie Domokit_esp8266
// lien : https://github.com/Thomas-Broussard/Domokit_ESP8266
#include "registres_esp8266.h"
#include "GPIO_esp8266.h"
#include "UART_esp8266.h"
#include "TIMER_esp8266.h"
#include "Scheduler.h"

// ##########################################################################################################################
//                                      DEFINE
// ##########################################################################################################################
// Version du template
#define VERSION_TEMPLATE "1.0.0"

// Définition des Pins utilisés sur l'ESP8266
#define LED_R GPIO12
#define LED_G GPIO13
#define LED_B GPIO15

#define BUT_1 D2
#define BUT_2 D7

#define MY_UART UART0

// ##########################################################################################################################
//                                      VARIABLES GLOBALES
// ##########################################################################################################################
// variables globales (interruptions)
volatile uint8 flag_int_GPIO = 0;

bool Domokit_Start;

// Définition de l'objet Domokit
#ifdef __DOMOKIT_H__
    Domokit Domokit("MonPremierObjet"); // Nom à donner à votre appareil
#endif
// ##########################################################################################################################
//                                       APPELS DE FONCTIONS
// ##########################################################################################################################
// Fonctions d'interruption (Handler)
void Interruption_GPIO(uint8 *myflag);
void Interruption_UART(uint8 *myflag);

// Fonctions liées au Scheduler
void Task_10us();
void Task_1ms();
void Task_1s();

// Fonctions Domokit
void Alarme_Interrupt(String data);

// ##########################################################################################################################
//                                      INITIALISATION
// ##########################################################################################################################
void setup()
{
    // ---------------------------------------------------------
    // Initialisation des variables
    // ---------------------------------------------------------

    // ---------------------------------------------------------
    // Interface Domokit
    // ---------------------------------------------------------
#ifdef __DOMOKIT_H__

    // Activation des Serial print Domokit (mode Debug uniquement)
    INIT_DEBUG_PRINT(); 
    DEBUG_PRINT("\nVersion du Template : ");DEBUG_PRINTLN(VERSION_TEMPLATE);

    // Initialisation de l'objet Domokit
    Domokit.begin();
#endif
    
    // ---------------------------------------------------------
    // GPIO (Entrées/Sorties Numériques)
    // ---------------------------------------------------------
    // Initialisation des pins
    init_GPIO(LED_R,GPIO_OUTPUT);
    init_GPIO(LED_G,GPIO_OUTPUT);
    init_GPIO(LED_B,GPIO_OUTPUT);
    init_GPIO(BUT_1,GPIO_INPUT);
    init_GPIO(BUT_2,GPIO_INPUT);
    
    // Etats par défaut des sorties
    GPIO_Write(LED_R,ETAT_HAUT);
    GPIO_Write(LED_G,ETAT_HAUT);
    GPIO_Write(LED_B,ETAT_HAUT);

    // Initialisation des Interruptions externes
    ETS_GPIO_INTR_DISABLE();
    ETS_GPIO_INTR_ATTACH(Interruption_GPIO,&flag_int_GPIO);
    Set_GPIO_Interrupt(BUT_1,GPIO_Interrupt::FRONT_DESCENDANT);
    Set_GPIO_Interrupt(BUT_2,GPIO_Interrupt::FRONT_DESCENDANT);
    ETS_GPIO_INTR_ENABLE();
    
    // ---------------------------------------------------------
    // TIMER / SCHEDULER
    // ---------------------------------------------------------
    // initialisation des compteurs virtuels (pour le scheduler)
    init_Compteurs_Virtuels();

    // Initialisation du Scheduler
    init_TIMER1_Scheduler();
}

// ##########################################################################################################################
//                                      PROGRAMME PRINCIPAL
// ##########################################################################################################################

void loop()
{
    // Gestion des actions via le scheduler
    Scheduler(Task_10us,Task_1ms,Task_1s);

    if (Domokit_Start == true)
    {
        // Gestion des interruptions GPIO (exemple)
        if (flag_int_GPIO > 0)
        {
            switch(flag_int_GPIO)
            {
                case 1 :
                    GPIO_Write(LED_R,ETAT_HAUT);
                    GPIO_Write(LED_G,ETAT_BAS);
                    GPIO_Write(LED_B,ETAT_BAS);
                    DEBUG_PRINT("Interrupt Bouton 1 ! ");
                break;

                case 2 : 
                    GPIO_Write(LED_R,ETAT_BAS);
                    GPIO_Write(LED_G,ETAT_BAS);
                    GPIO_Write(LED_B,ETAT_HAUT);
                    DEBUG_PRINT("Interrupt Bouton 2 ! ");
                break;
            }
            flag_int_GPIO = 0;
        }
    }   
}

// Interruption externe
void Interruption_GPIO(uint8 * myflag)
{
    // on vérifie la gpio qui a demandé l'interruption
    if (READ_BIT(Registre_GPIO->STATUS,BUT_1) == 1)
    {
        SET_BIT(Registre_GPIO->STATUS_W1TC,BUT_1); // clear de l'interruption
        *myflag = 1;
    }
    if (READ_BIT(Registre_GPIO->STATUS,BUT_2) == 1)
    {
        SET_BIT(Registre_GPIO->STATUS_W1TC,BUT_2); // clear de l'interruption
        *myflag = 2;
    }
}

// ##########################################################################################################################
//                                       TACHES CADENCEES PAR LE SCHEDULER
// ##########################################################################################################################
void Task_10us()
{
    // ##############################################################
    //              Compteurs virtuels de type TIMER
    // ##############################################################
    // Décrémentation des compteurs virtuels


    // Action(s) instantanée(s)
    //GPIO_Toggle(LED_B);

    // Action(s) dès qu'un compteur virtuel tombe à zéro

    // ##############################################################
    //              Compteurs virtuels de type DELAY
    // ##############################################################
    // Exemple
    // Décrémentation des compteurs uniquement si le mode delay est activé
    /*
    if (Compteur_virtuel[].delay == true) {
        Compteur_virtuel[].valeur--;
        // code ici
    }
    */

    // Action à réaliser dès que le compteur_virtuel tombe à zéro
    /*
    if(Compteur_virtuel[].valeur == 0)
    {
        Compteur_virtuel[].delay = false;
        // code ici
    }
    */
}

void Task_1ms()
{   
    // ##############################################################
    //              Compteurs virtuels de type TIMER
    // ##############################################################
    // Décrémentation des compteurs virtuels
    Compteur_virtuel_ms[0].valeur--;

    // Action(s) instantanée(s)
    //GPIO_Toggle(LED_R);

    // Action(s) dès qu'un compteur virtuel tombe à zéro
    if (Compteur_virtuel_ms[0].valeur == 0)
    {
        Compteur_virtuel_ms[0].valeur = 200; // Nombre de ms avant le prochain déclenchement

        // Routine permettant la réception des messages MQTT sur les topics souscrits
        #ifdef __DOMOKIT_H__
        Domokit.verifierMQTT_Receive();
        #endif
    }


    // ##############################################################
    //              Compteurs virtuels de type DELAY
    // ##############################################################
    // Exemple
    // Décrémentation des compteurs uniquement si le mode delay est activé
    /*
    if (Compteur_virtuel[].delay == true) {
        Compteur_virtuel[].valeur--;
        // code ici
    }
    */

    // Action à réaliser dès que le compteur_virtuel tombe à zéro
    /*
    if(Compteur_virtuel[].valeur == 0)
    {
        Compteur_virtuel[].delay = false;
        // code ici
    }
    */
    
}

void Task_1s()
{
    // ##############################################################
    //              Compteurs virtuels de type DELAY
    // ##############################################################
    // Décrémentation des compteurs virtuels
    Compteur_virtuel_s[0].valeur--;

    // Action(s) instantanée(s)
    GPIO_Toggle(LED_R);
    GPIO_Toggle(LED_G);
    GPIO_Toggle(LED_B);

    // Action(s) dès qu'un compteur virtuel tombe à zéro
    if (Compteur_virtuel_s[0].valeur == 0)
    {
        Compteur_virtuel_s[0].valeur = 5; // Nombre de secondes avant le prochain déclenchement
        //UART_WriteString(MY_UART,"A");

        // Vérification de la connexion au réseau Domokit
        #ifdef __DOMOKIT_H__
        Domokit_Start = Domokit.checkConnexion(); 
        #endif

        // code ici
        // Envoi d'une valeur aléatoire sur la jauge de l'application mobile
        String mPayload = String(random(0,100));
        Domokit.SendtoTile("test_graph",mPayload);
    }
    /*
    // ##############################################################
    //              Compteurs virtuels de type DELAY
    // ##############################################################
    // Exemple
    // Décrémentation des compteurs uniquement si le mode delay est activé
    
    if (Compteur_virtuel[].delay == true) {
        Compteur_virtuel[].valeur--;
        // code ici
    }
    

    // Action à réaliser dès que le compteur_virtuel tombe à zéro
    if(Compteur_virtuel[].valeur == 0)
    {
        Compteur_virtuel[].delay = false;
        // code ici
    }*/
    
}

// ##########################################################################################################################
//                                       FONCTIONS CALLBACK DOMOKIT - APPLICATION MOBILE
// ##########################################################################################################################
#ifdef __DOMOKIT_H__
// --------------------------------------------------------
// Fonctions dont le contenu doit être modifié selon les besoins
// --------------------------------------------------------

// Permet d'envoyer une alerte au serveur
void Alarme_Interrupt(String data)
{
  Domokit.MQTT_Send(Domokit.topic_interruption,data);
}

// Défini les Tiles qui seront affichées sur le Dashboard pour cet objet
void init_Tile(){
    Domokit.setTileText("Exemple Texte","test_texte",false);
    Domokit.setTileSwitch("Exemple Switch","test_switch");
    Domokit.setTileGraph("Exemple Graph","test_graph",0,100);
    Domokit.setTileJauge("Exemple Jauge","test_jauge",true,40,60);
    Domokit.setTileRadioButton("Exemple Radio","test_radio","fa-eye","fa-circle");
    Domokit.setTileIcon("Exemple Icon","test_icone");
}

// Défini les Actions réalisées lorsqu'une Tile est modifiée par l'utilisateur
void callBack_Tile(String TileTopic, String payload)
{
    if(TileTopic == "test_switch")
    {
        if(payload == "ON")
        {
            DEBUG_PRINTLN("Test switch = ON");
            Domokit.SendIconToTile("test_icone","fa_eye","#00FF00");
        }
        else
        {
            DEBUG_PRINTLN("Test switch = OFF");
            Domokit.SendIconToTile("test_icone","fa_gears","#FF0000");
        }
    }
    if(TileTopic == "test_jauge")
    {
        DEBUG_PRINTLN("Niveau Jauge = " + payload);
    }
}
#endif
// ##########################################################################################################################
//                                       AUTRES FONCTIONS
// ##########################################################################################################################
