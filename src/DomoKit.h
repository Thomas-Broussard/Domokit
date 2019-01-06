/* 
 *  =============================================================================================================================================
 *  Titre : DomoKit.h
 *  Auteur : Thomas Broussard
 *  ---------------------------------------------------------------------------------------------------------------------------------------------
 *  Description :
 *  Librairie permettant de construire facilement un objet connecté Domokit
 * =============================================================================================================================================
 */
 
#ifndef __DOMOKIT_H__
#define __DOMOKIT_H__ 

// ################################################################################
// 									DEBUG
// ################################################################################
// définition du mode Debug (commenter pour désactiver les options)
#define DEBUG_DOMOKIT // active les Serial.Print pour le débug

#ifdef DEBUG_DOMOKIT
  #define DEBUG_MQTT_RECEIVE // affiche les trames MQTT reçues
  #define DEBUG_MQTT_SEND    // affiche les trames MQTT envoyées
  //#define DEBUG_WIFI_DATA    // affiche les infos wifi de l'objet (/!\ informations sensibles ! /!\)
#endif
// ################################################################################
// 									Librairies
// ################################################################################
#ifdef DEBUG_DOMOKIT
  #include "Arduino.h"
#endif
  #include <ESP8266WiFi.h>
  #include <PubSubClient.h>
  #include <EEPROM.h>

// ################################################################################
// 				VERSION DE LA LIBRAIRIE
 // ################################################################################
#define VERSION_DOMOKIT "1.0.0"


// ################################################################################
// 				Jeu d'instruction interprétable par l'objet domokit
// ################################################################################
  #define START       "START"
  #define STOP        "STOP"
  #define ON          "ON"
  #define OFF         "OFF"
  #define DATA        "DATA"
  #define CONNECT     "CONNECT"
  #define COMMANDE    "CMD"
  #define WIFI_DATA   "WIFI_DATA"

// ################################################################################
// 				Définition des "Tile" existantes pour les Dashboards
// ################################################################################
  #define TILE_TEXT_DISPLAY   0
  #define TILE_JAUGE_DISPLAY  1
  #define TILE_GRAPH          2
  #define TILE_SWITCH         3
  #define TILE_COMMANDLIST    4
  #define TILE_TEXT_DRIVE     5
  #define TILE_JAUGE_DRIVE    6
  #define TILE_RADIOBUTTON    7
  #define TILE_ICON           8
  
// ################################################################################
// 				Defines , définition et variables globales
// ################################################################################
  #define _PARSE  ";" // Caractère pour parser les trames MQTT
  
  #define SSID_WIFI_APPAIRAGE "<Domokit_Appairage>"
  #define PASSWORD_WIFI_APPAIRAGE "domokit_appairage"
  #define NB_TENTATIVE_CONNEXION 30
  #define WIFI_MODE_NORMAL 0
  #define WIFI_MODE_APPAIRAGE 1

  // Mapping mémoire EEPROM
  #define EEPROM_ADDR_WIFI 0x0000

  // Pins pour la led RGB
  #define LED_R_PIN 0x0C
  #define LED_G_PIN 0x0D
  #define LED_B_PIN 0x0F

  // Informations de connexion au serveur MQTT
  #define MQTT_SERVER   "192.168.100.1" 
  #define MQTT_PORT     1883            
  
  // Login MQTT
  #define MQTT_USER     "domokit"
  #define MQTT_PSWD     "domokit"

  // Topic principal MQTT
  #define MAIN_TOPIC    "domokit"


  // Fonctions associées au mode Debug
  #ifdef DEBUG_DOMOKIT
   #define DEBUG_PRINTLN(x)  Serial.println(x)
   #define DEBUG_PRINT(x)    Serial.print(x)
  #else
   #define DEBUG_PRINTLN(x)
   #define DEBUG_PRINT(x)  
  #endif

  #ifdef DEBUG_DOMOKIT
   #define INIT_DEBUG_PRINT()  Serial.begin(9600)
  #else
   #define INIT_DEBUG_PRINT()
  #endif


// Liste des états de la connexion wifi
typedef enum{NON_CONNECTE,APPAIRAGE,CONNECTE,ETEINT} Statut_Wifi;

// ################################################################################
// 								Fonctions de callback
// ################################################################################
  extern void init_Tile();
  extern void callBack_Tile(String TileTopic, String payload);
  
// ################################################################################
// 									Classes
// ################################################################################
	class Domokit
	{

// ================================================================================
// 									PUBLIC
// ================================================================================
		public:
      // -------------------------
			// Constructeur
      // -------------------------
      Domokit(String Nom_Appareil);
			
			// -------------------------
      // Setters
      // -------------------------
			void enableLedWifi(void);
      void disableLedWifi(void);
			void setName(String Name);
			void startProgram();
			void stopProgram();
			void Debug_MQTT_Print(String message);
      void Create_Topics();
			// -------------------------
      // Getters
      // -------------------------
			boolean getStateProgram();
			String  getAddrMac();
      
			// -------------------------
      // Fonctions DomoKit
      // -------------------------
			void begin();
      void Wifi_Data_EEPROM(); 
			boolean checkConnexion();
      void verifierMQTT_Receive();
      void MQTT_Send(String topic, String Payload);
      
      void allumerLedWifi(Statut_Wifi Mode);
      void clignoterLedWifi(Statut_Wifi Mode,int Nb_clignotement, int Duree_clignotement_ms);

      // Gestion des Tiles
      void resetTile();
      void SendtoTile(String topic, String payload);
      void SendIconToTile(String topic, String fa_icon, String color);
      
      void setTileText(String Titre, String Topic, bool enablePub);
      void setTileSwitch(String Titre, String Topic);
      void setTileGraph(String Titre, String Topic, int levelMin,int levelMax);
      void setTileJauge(String Titre, String Topic, bool enablePub,int levelMin,int levelMax);
      void setTileRadioButton(String Titre, String Topic, String onIcon, String offIcon);
      void setTileIcon(String Titre, String Topic);


      // Variables
      String topic_instruction;
      String topic_donnees;
      String topic_interruption;
      String topic_connexion;
      String topic_connect;
      String topic_debug;
      String topic_tile;
      String topic_set_tile;
      

// ================================================================================
// 									PRIVATE
// ================================================================================
		private:
			// -------------------------
      // Paramètres Wifi
      // -------------------------
      // Wifi en cours d'utilisation
      String _Wifi_SSID;
      String _Wifi_Password;
      // Wifi normal de la box Domokit
			String 	_Wifi_Normal_SSID;
			String 	_Wifi_Normal_Password;

      // Wifi d'appairage de la box Domokit
      String 	_Wifi_Appairage_SSID;
			String 	_Wifi_Appairage_Password;
			
			// -------------------------
      // Paramètres MQTT
      // -------------------------
			String 	_MQTT_Serveur;
			int 	  _MQTT_Port;
			String 	_MQTT_User;
			String 	_MQTT_Password;
			String  _MQTT_Main_Topic;

      int _TileID;
      // ------------------- 
      // Caractéristiques 
      // ------------------- 

      // Configuration de la connexion wifi
			String 	_NOM_APPAREIL;
			String  _ADDR_MAC;
      String _CLIENT_NAME;

      // Led
      boolean 	_LED_WIFI; // indique si on utilise ou non le témoin lumineux RGB
      int   _LED_R; // D6
      int   _LED_G; // D7
      int   _LED_B; // D8

			
			
			// --------------------
      // Indique que le programme peut commencer
      // --------------------
			boolean _Program_Start;

      // -------------------------
      // Fonctions privées
      // -------------------------
      void MQTT_Subscribe(String topic);
      void setup_wifi() ;
      void setup_mqtt();
      bool reconnect_mqtt();
      boolean Check_Connexion_Wifi();
      String macToStr(const uint8_t* mac);
      void setWifiMode(int Mode);
      void setWifi(String Wifi_SSID, String Wifi_Password);
      boolean ConnexionWifi(int nb_tentative, int mode);
      void MQTT_Receive(char* topic, byte* payload, unsigned int length);
      void Decode_Instruction(String Instruction,String Topic);
      void composeSetTilePayload(String attribut, String valeur);
      void setTile(String Titre, int Type, String Topic , int levelMin, int levelMax,String onIcon, String offIcon);
	};
  
  
// ################################################################################
// 						Fonctions externes à la classe Domokit
// ################################################################################ 

// Parse d'une String
String ParseString(String data, char separator, int index);

// Manipulation mémoire EEPROM
String Read_STR_EEPROM(int taille, int addr_debut);
void Write_STR_EEPROM(char str[], int taille, int addr_debut);

// Cryptage des données
String Cryptage(char* src, String key);
String Decryptage(char* src, String key);
#endif
