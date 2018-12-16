/* 
 *  =============================================================================================================================================
 *  Titre : DomoKit.cpp
 *  Auteur : Thomas Broussard
 *  ---------------------------------------------------------------------------------------------------------------------------------------------
 *  Description :
 *  Librairie permettant de construire facilement un objet connecté Domokit
 * =====================================================================================================================================
 */

#include "DomoKit.h"

// ################################################################################
// 									Objets / Dépendances
// ################################################################################
WiFiClient espClient;
PubSubClient client(espClient);

// ################################################################################
// 									Variables globales
// ################################################################################
// Clef de chiffrement du serveur
String KEY_SERVEUR;
boolean key_serveur_dispo;


// ################################################################################
// 									Constructeur
// ################################################################################
Domokit::Domokit(String Nom_Appareil)
{
	// Affectations des paramètres
	_Wifi_Appairage_SSID 		  = SSID_WIFI_APPAIRAGE;
	_Wifi_Appairage_Password 	= PASSWORD_WIFI_APPAIRAGE;
	
	_MQTT_Serveur 	= MQTT_SERVER;
	_MQTT_Port 		  = MQTT_PORT;
	
	_MQTT_User 		  = MQTT_USER;
	_MQTT_Password 	= MQTT_PSWD;
	
	_MQTT_Main_Topic= MAIN_TOPIC;
	
	// Initialisation des autres variables
	_TileID = 0;

  // Témoin lumineux (désactivé par défaut)
	_LED_WIFI = false;
  #ifdef LED_R_PIN
  _LED_R = LED_R_PIN;
  #endif
  #ifdef LED_G_PIN
  _LED_G = LED_G_PIN;
  #endif
  #ifdef LED_B_PIN
  _LED_B = LED_B_PIN;
  #endif
	
	_NOM_APPAREIL 	= Nom_Appareil;
	_Program_Start	= false;

  // Obtention de l'adresse MAC du client
  uint8_t mac[6];
  WiFi.macAddress(mac);
  _ADDR_MAC = macToStr(mac);

  // Génération du nom client à partir de l'adresse MAC de l'objet et de son nom
  _CLIENT_NAME = _NOM_APPAREIL + '_' + _ADDR_MAC;
}


// ################################################################################
// 									Setters
// ################################################################################
// Active le témoin d'activité wifi (généralement une LED)
void Domokit::enableLedWifi	(void)
{
	_LED_WIFI = true;
  pinMode(_LED_R,OUTPUT);
  pinMode(_LED_G,OUTPUT);
  pinMode(_LED_B,OUTPUT);
}

void Domokit::disableLedWifi(void)
{
  if (_LED_WIFI == true)
  {
    digitalWrite(_LED_R,LOW);
    digitalWrite(_LED_G,LOW);
    digitalWrite(_LED_B,LOW);
  	_LED_WIFI = false;
  }
}

// Défini le nom de l'appareil
void Domokit::setName(String Name)
{
	_NOM_APPAREIL = Name;
}

// Défini le SSID/Password de la box Domokit (en mode de fonctionnement normal)
void Domokit::setWifi(String Wifi_SSID , String Wifi_Password)
{
  _Wifi_Normal_SSID = Wifi_SSID;
  _Wifi_Normal_Password = Wifi_Password;
}

// Configuration du wifi (SSID/Password) selon le mode choisi
// 0 = mode normal
// 1 = mode appairage
void Domokit::setWifiMode(int Mode)
{
  switch(Mode)
  {
    // Mode normal
    case 0 : 
      _Wifi_SSID      = _Wifi_Normal_SSID;
      _Wifi_Password  = _Wifi_Normal_Password;
    break;

    // Mode appairage
    case 1 : 
      _Wifi_SSID      = _Wifi_Appairage_SSID;
      _Wifi_Password  = _Wifi_Appairage_Password;
    break;

    // Par défaut : aucune modification
    default : 
    break;
  }
}
// Active le programme
void Domokit::startProgram()
{
	_Program_Start = true;
}

// Arrête le programme
void Domokit::stopProgram()
{
	_Program_Start = false;
}

// ################################################################################
// 									Getters
// ################################################################################
// Renvoie l'état du programme (true = activé / false : arrêté)
boolean Domokit::getStateProgram()
{
	return _Program_Start;
}

// Renvoie l'adresse MAC de l'objet
String Domokit::getAddrMac()
{
  return _ADDR_MAC;
}



// ################################################################################
// 									Témoins lumineux
// ################################################################################

/*===============================================================================
  Nom 			: allumerLedWifi
  
  Description	: Allume le témoin lumineux wifi. La couleur du témoin dépend du
  statut actuel du wifi (connecté, non connecté, en attente d'appairage, etc..)
  
  Paramètre(s) 	: Statut wifi
  
  Retour		: aucun
===============================================================================*/
void Domokit::allumerLedWifi(Statut_Wifi Mode)
{
  if(_LED_WIFI == true)
  {
    switch(Mode)
    {
      case NON_CONNECTE : 
        digitalWrite(_LED_R,HIGH);
        digitalWrite(_LED_G,LOW);
        digitalWrite(_LED_B,HIGH);
      break;

      case APPAIRAGE :
        digitalWrite(_LED_R,LOW);
        digitalWrite(_LED_G,LOW);
        digitalWrite(_LED_B,HIGH);
      break;

      case CONNECTE :
        digitalWrite(_LED_R,LOW);
        digitalWrite(_LED_G,HIGH);
        digitalWrite(_LED_B,LOW);
      break;

      case ETEINT :
        digitalWrite(_LED_R,LOW);
        digitalWrite(_LED_G,LOW);
        digitalWrite(_LED_B,LOW);
      break;
    }
  }
}

/*===============================================================================
  Nom 			: clignoterLedWifi
  
  Description	: Fait clignoter le témoin lumineux selon le statut wifi choisi
  
  Paramètre(s) 	: Statut wifi
  
  Retour		: aucun
===============================================================================*/
void Domokit::clignoterLedWifi(Statut_Wifi Mode,int Nb_clignotement, int Duree_clignotement_ms)
{
if(_LED_WIFI == true)
  {
    // la led Wifi clignote tant que l'objet n'est pas authentifié auprès du serveur
    for (int i = 0; i < Nb_clignotement; i ++)
    {
      allumerLedWifi(Mode);
      delay(Duree_clignotement_ms/2);
      allumerLedWifi(ETEINT);
      delay(Duree_clignotement_ms/2);
    }
  }
  else
  {
    delay(3000);
  }
}

// ################################################################################
// 									Fonctions Domokit
// ################################################################################

/*===============================================================================
  Nom 			: 	begin
  
  Description	: 	routine d'initialisation de l'objet Domokit
  
  Paramètre(s) 	: 	aucun
  
  Retour		: 	aucun
===============================================================================*/
void Domokit::begin(){
	DEBUG_PRINT("Librairie Domokit Version "); DEBUG_PRINTLN(VERSION_DOMOKIT);
  
  
  // Témoin d'activité wifi
  allumerLedWifi(APPAIRAGE);

  // Récupération du SSID et du Password en mémoire de l'objet
  this->Wifi_Data_EEPROM();


  // Si des paramètres de connexion ont été trouvé, alors on tente de se connecter au wifi domokit
  if (_Wifi_Normal_Password != "")
  {
    DEBUG_PRINTLN("Connexion au réseau wifi Domokit...");
    this->ConnexionWifi(NB_TENTATIVE_CONNEXION,WIFI_MODE_NORMAL);
  }
  // Sinon on tente de se connecter au wifi appairage
	else
	{
    DEBUG_PRINTLN("Aucune configuration Wifi trouvée. Connexion au réseau wifi Appairage...");
    this->ConnexionWifi(NB_TENTATIVE_CONNEXION,WIFI_MODE_APPAIRAGE);
    //this->setWifiMode(1);
    //this->setup_wifi();
	}
  

  // Connexion au serveur mqtt
  this->setup_mqtt();

  // Création des topics mqtt 
  this->Create_Topics();
}


/*===============================================================================
  Nom 			: 	Wifi_Data_EEPROM
  
  Description	:  récupère les infos wifi dans l'EEPROM de l'objet
  
  Paramètre(s) 	: 	aucun
  
  Retour		: 	aucun (complète des variables globales)
===============================================================================*/
void Domokit::Wifi_Data_EEPROM(){
  String Buffer_EEPROM;
  Buffer_EEPROM = Read_STR_EEPROM(510,EEPROM_ADDR_WIFI);

  DEBUG_PRINT("eeprom : ");DEBUG_PRINTLN(Buffer_EEPROM);

  _Wifi_Normal_SSID = ParseString(Buffer_EEPROM,';',0);
  _Wifi_Normal_Password = ParseString(Buffer_EEPROM,';',1);
  KEY_SERVEUR = ParseString(Buffer_EEPROM,';',2);

    
  // affichage au terminal
  #ifdef DEBUG_WIFI_DATA
    DEBUG_PRINT("SSID eeprom : ");
    DEBUG_PRINTLN(_Wifi_Normal_SSID);
    DEBUG_PRINT("Password eeprom : ");
    DEBUG_PRINTLN(_Wifi_Normal_Password);
    DEBUG_PRINT("Clef de chiffrement (serveur) eeprom :");
    DEBUG_PRINTLN(KEY_SERVEUR);
  #endif

  // Vérification de la clef de chiffrement serveur (pour le cryptage des données)
  key_serveur_dispo = (KEY_SERVEUR != "")? true:false;
}

// ################################################################################
// 						                        WIFI
// ################################################################################ 

/*===============================================================================
  Nom 			: 	Debug_MQTT_Print
  
  Description	:  Envoie un message sur le topic de debug
  
  Paramètre(s) 	: message à envoyer
  
  Retour		: 	aucun
===============================================================================*/
void Domokit::Debug_MQTT_Print(String message)
{
  #ifdef DEBUG_DOMOKIT
    this->MQTT_Send(topic_debug,message);
  #endif
}

/*===============================================================================
  Nom 			: 	setup_wifi
  
  Description	: 	Initialise la connexion au point d'accès wifi
  
  Paramètre(s) 	: 	aucun
  
  Retour		: 	aucun
===============================================================================*/
void Domokit::setup_wifi() 
{
  //delay(10);
  
  // On masque le point d'accès wifi généré par l'ESP et on passe en mode station
  WiFi.mode(WIFI_STA);
  WiFi.hostname((char*)_CLIENT_NAME.c_str());
  
  //delay(10);

  // Connexion au point d'accès wifi
  WiFi.begin((char*)_Wifi_SSID.c_str(), (char*)_Wifi_Password.c_str());
  allumerLedWifi(NON_CONNECTE);

  // affichage au terminal
  #ifdef DEBUG_WIFI_DATA
    DEBUG_PRINTLN();
    DEBUG_PRINT("Connexion au hostspot "); DEBUG_PRINTLN(WiFi.SSID());
    DEBUG_PRINT("Password : ");           DEBUG_PRINTLN(_Wifi_Password);
    DEBUG_PRINT("En tant que ");          DEBUG_PRINTLN(WiFi.hostname());
    DEBUG_PRINTLN();
  #endif
  
  while (WiFi.status() != WL_CONNECTED) { 
    delay(500);
    DEBUG_PRINT(".");
  }
  DEBUG_PRINTLN();
  DEBUG_PRINTLN("Connexion au wifi réussie");
  DEBUG_PRINT("Adresse IP : ["); DEBUG_PRINT(WiFi.localIP()); DEBUG_PRINTLN("]");
  DEBUG_PRINTLN();
  delay(200);
}

/*===============================================================================
  Nom 			: 	ConnexionWifi
  
  Description	: 
  l'objet domokit va tenter de se connecter au wifi, selon les 
  données qu'il a en mémoire (SSID / Password). Si au bout de N tentatives 
  l'objet Domokit n'a pas réussi à se connecter, alors il va utiliser les
  informations standards pour se connecter au wifi d'appairage.
  
  Paramètre(s) 	: nb_tentative : Nombre de tentatives de connexion au wifi
                  mode : mode de connexion (0 = normal / 1 = appairage)
  
  Retour		: 	
  true : on est connecté au wifi normal
  false : on est connecté au wifi d'appairage
===============================================================================*/
boolean Domokit::ConnexionWifi(int nb_tentative,int mode)
{
  int i = 0;

  // Mise à jour des informations wifi si on se connecte en mode normal
  if(mode == WIFI_MODE_NORMAL){
    this->Wifi_Data_EEPROM();
  }

  // On défini le mode du wifi
  this->setWifiMode(mode);
  
  // On se déconnecte du wifi actuel
  WiFi.disconnect();
  
  // On prépare les nouveaux paramètres de connexion
  WiFi.mode(WIFI_STA);
  WiFi.hostname((char*)_CLIENT_NAME.c_str());

  // Affichage au terminal
  #ifdef DEBUG_WIFI_DATA
    DEBUG_PRINTLN();
    DEBUG_PRINT("Connexion au hostspot "); DEBUG_PRINTLN(_Wifi_SSID);
    DEBUG_PRINT("Password : ");           DEBUG_PRINTLN(_Wifi_Password);
    DEBUG_PRINT("En tant que ");          DEBUG_PRINTLN(WiFi.hostname());
    DEBUG_PRINTLN();
  #endif

  WiFi.begin((char*)_Wifi_SSID.c_str(), (char*)_Wifi_Password.c_str());

  //delay(10);
  // On essaye de se connecter au wifi Domokit
  for (i = 0 ; i < nb_tentative; i++)
  {
      // Si la connexion fonctionne, on peut sortir de la fonction
    if (WiFi.status() == WL_CONNECTED)
    {
      DEBUG_PRINTLN();
      DEBUG_PRINTLN("connexion au wifi Domokit OK");
      DEBUG_PRINT("Adresse IP : ["); DEBUG_PRINT(WiFi.localIP()); DEBUG_PRINTLN("]");
      DEBUG_PRINTLN();
      return true;
    }

    delay(500);
    DEBUG_PRINT(".");
  }
    
    // Si aucune tentative n'a marché, alors on bascule sur l'autre réseau'
    if(mode == WIFI_MODE_APPAIRAGE)
    {
      DEBUG_PRINTLN("Impossible de se connecter au wifi Appairage. Passage en mode Domokit");
      ConnexionWifi(NB_TENTATIVE_CONNEXION, WIFI_MODE_NORMAL);
    }
    else
    {
      DEBUG_PRINTLN("Impossible de se connecter au wifi Domokit. Passage en mode Appairage");
      ConnexionWifi(NB_TENTATIVE_CONNEXION, WIFI_MODE_APPAIRAGE);
    }
    return true;
}

/*===============================================================================
  Nom 			: 	checkConnexion
  
  Description	: 	Vérifie que l'objet est bien connecté au serveur mqtt, 
					et qu'il est authentifié auprès du serveur
  
  Paramètre(s) 	: 	aucun
  
  Retour		: 	aucun
===============================================================================*/
boolean Domokit::checkConnexion()
{  
  Check_Connexion_Wifi();

  if (_Program_Start == false)
  {
      // Trame d'initialisation : le client envoie ses informations principales au serveur
      MQTT_Send(topic_connexion,(_ADDR_MAC + ";" + _CLIENT_NAME));
      
      DEBUG_PRINTLN("Authentification en cours..."); 
      
      // Clignotement de la led wifi tant qu'on est pas authentifié
      //clignoterLedWifi(APPAIRAGE,3,1000);
      
      return false;
  }
  
  // Sinon on allume la led Wifi dès que l'objet est authentifié
  else
  {
    allumerLedWifi(CONNECTE);
    return true;
  }
}


/*===============================================================================
  Nom 			: Check_Connexion_Wifi
  
  Description	: Vérifie que l'objet est toujours connecté au serveur MQTT wifi
  
  Paramètre(s) 	: aucun
  
  Retour		: 
  * true si l'objet est connecté
  * false sinon		
===============================================================================*/
boolean Domokit::Check_Connexion_Wifi()
{
  // Vérification de la connexion wifi
  if(WiFi.status() != WL_CONNECTED)
  {
    ConnexionWifi(NB_TENTATIVE_CONNEXION,WIFI_MODE_NORMAL);
    return false;
  }

  // Vérification de la connexion au serveur MQTT
  if (!client.connected()) {
    if (!reconnect_mqtt());
      return false;
  }
  
  return client.loop();
}


void Domokit::verifierMQTT_Receive()
{ 
  client.loop();
}
/*===============================================================================
  Nom 			: 	macToStr
  
  Description	: 	permet de convertir une adresse mac contenue dans un tableau d'octets
					en une chaîne de caractère de type String.
  
  Paramètre(s) 	: 	aucun
  
  Retour		: 	String contenant l'adresse mac
===============================================================================*/
String Domokit::macToStr(const uint8_t* mac)
{
  String result;
  for (int i = 0; i < 6; ++i) {
    result += String(mac[i], 16);
    if (i < 5)
      result += ':';
  }
  return result;
}

// ################################################################################
// 						                        MQTT
// ################################################################################ 

/*===============================================================================
  Nom 			: 	setup_mqtt
  
  Description	: 	Initialise la connexion au broker MQTT
  
  Paramètre(s) 	: 	aucun
  
  Retour		: 	aucun
===============================================================================*/
void Domokit::setup_mqtt()
{
  client.setServer((char*)_MQTT_Serveur.c_str(), _MQTT_Port);
  client.setCallback([this] (char* topic, byte* payload, unsigned int length) { this->MQTT_Receive(topic, payload, length); });

  // Affichage de Debug
  DEBUG_PRINTLN("Connexion au serveur MQTT réussie");
  DEBUG_PRINTLN("Connecté en tant que " + _CLIENT_NAME);
  DEBUG_PRINTLN();
}


/*===============================================================================
  Nom 			: 	reconnect_mqtt
  
  Description	: 	Fonction appelée lorsque la connexion au broker MQTT est rompue.
					Elle reconnecte automatiquement l'objet au broker.
  
  Paramètre(s) 	: 	aucun
  
  Retour		: 	aucun
===============================================================================*/
bool Domokit::reconnect_mqtt() {
  // Loop until we're reconnected
  if (!client.connected()) 
  {
    DEBUG_PRINT("Connexion au broker MQTT ");   DEBUG_PRINTLN(_MQTT_Serveur);
    DEBUG_PRINT("En tant que ");                DEBUG_PRINTLN(_CLIENT_NAME);
    
    // Connexion
    if (client.connect((char*) _CLIENT_NAME.c_str() , (char*)_MQTT_User.c_str(), (char*)_MQTT_Password.c_str()))
    {
      DEBUG_PRINTLN("Connexion au broker MQTT réussie");
      
      // inscription au topic d'instructionx<
      String mTopic = _MQTT_Main_Topic + "/instruction/#";
      client.subscribe((char*)mTopic.c_str());
      return true;
    } 
    else 
    {
      DEBUG_PRINT("Echec de connexion au broker MQTT ! rc="); DEBUG_PRINTLN(client.state());
      DEBUG_PRINTLN("Nouvelle tentative dans 5 secondes...");
      //delay(5000);
      return false;
    }
  }
  else
  {
    return false;
  }
}

/*===============================================================================
  Nom 			: Create_Topics
  
  Description	: Génère la liste des topics MQTT utiles pour l'objet 
  
  Paramètre(s) 	: aucun
  
  Retour		: aucun
===============================================================================*/
void Domokit::Create_Topics()
{
  // Création des topics MQTT
  topic_instruction   = _MQTT_Main_Topic + "/instruction/"  + _ADDR_MAC;
  topic_donnees       = _MQTT_Main_Topic + "/donnees/"      + _ADDR_MAC;
  topic_interruption  = _MQTT_Main_Topic + "/interruption/" + _ADDR_MAC;
  topic_connexion     = _MQTT_Main_Topic + "/connexion";
  topic_connect       = _MQTT_Main_Topic + "/connect/"  	+ _ADDR_MAC;

  topic_tile          = _MQTT_Main_Topic + "/instruction/tile/" + _ADDR_MAC;
  topic_set_tile      = _MQTT_Main_Topic + "/set_tile/"  	+ _ADDR_MAC;

  #ifdef DEBUG_DOMOKIT
  topic_debug         = _MQTT_Main_Topic + "/debug/"  	+ _ADDR_MAC;
  #endif
  
  // Affichage de Debug
  DEBUG_PRINTLN("Liste des topics :");
  DEBUG_PRINTLN(topic_connexion);
  DEBUG_PRINTLN(topic_instruction);
  DEBUG_PRINTLN(topic_donnees);
  DEBUG_PRINTLN(topic_interruption);
  #ifdef DEBUG_DOMOKIT
  DEBUG_PRINTLN(topic_debug);
  #endif
  DEBUG_PRINTLN();
}

/*===============================================================================
  Nom 			: MQTT_Subscribe
  
  Description	: Permet d'abonner l'objet à un topic MQTT
  
  Paramètre(s) 	: 
  * topic		: topic sur lequel on abonne l'objet
  
  Retour		: aucun
===============================================================================*/
void Domokit::MQTT_Subscribe(String topic)
{
  client.subscribe((char*)topic.c_str());
}

/*===============================================================================
  Nom 			: MQTT_Send
  
  Description	: Publie un message MQTT
  
  Paramètre(s) 	: 
  * topic 		: topic du message MQTT
  * payload		: payload du message MQTT
  
  Retour		: aucun
===============================================================================*/
void Domokit::MQTT_Send(String topic, String Payload)
{
  // Cryptage des données
  String Crypt_Payload = Cryptage((char*)Payload.c_str(),KEY_SERVEUR);

  // Envoi des données cryptées
  client.publish((char*)topic.c_str(),(char*)Crypt_Payload.c_str());
  
  // Affichage au terminal
  #ifdef DEBUG_MQTT_SEND
    DEBUG_PRINT("Send MQTT : ");
    DEBUG_PRINT("\tTopic = [");DEBUG_PRINT(topic); DEBUG_PRINT("]");
    DEBUG_PRINT("\tPayload = [");DEBUG_PRINT(Payload); DEBUG_PRINTLN("]");
  #endif
}





/*===============================================================================
  Nom 			: 	MQTT_Receive
  
  Description	: 	Fonction appelée dès qu'un message MQTT est reçu par l'objet
					Cette fonction appelle directement Decode_Instruction
  
  Paramètre(s) 	: 
  * topic 		: topic du message MQTT	
  * payload 	: payload du message MQTT
  * length		: taille du message MQTT
  
  Retour		: aucun
===============================================================================*/
void Domokit::MQTT_Receive(char* topic, byte* payload, unsigned int length) 
{
  char str_payload[512];
  String mTopic = String(topic);
  String decrypt_payload;
  memset(str_payload, 0, sizeof(str_payload));

  // Réception des données
  for (unsigned int i = 0; i < length; i++)
    str_payload[i] = payload[i];

  // Décryptage des données
  decrypt_payload = Decryptage((char*)str_payload,KEY_SERVEUR);

  // Affichage au terminal
  #ifdef DEBUG_MQTT_RECEIVE
    DEBUG_PRINT("Receive MQTT : ");
    DEBUG_PRINT("\tTopic = [");DEBUG_PRINT(topic); DEBUG_PRINT("]");
    DEBUG_PRINT("\tPayload (crypté) = [");DEBUG_PRINT(str_payload); DEBUG_PRINTLN("]");
    DEBUG_PRINT("\tPayload (décrypté) = [");DEBUG_PRINT(decrypt_payload); DEBUG_PRINTLN("]");
  #endif

  // Décodage de l'instruction
  this->Decode_Instruction(decrypt_payload,topic);
}

/*===============================================================================
  Nom 			: 	Decode_Instruction
  
  Description	: 	Permet d'appeler une fonction de callback, selon l'instruction
					reçue via MQTT
					
  Paramètre(s) 	: 	Instruction = instruction à décoder
  
  Retour		: 	aucun
===============================================================================*/
void Domokit::Decode_Instruction(String Instruction,String Topic)
{
      String Data;
      String wifi_ssid;
      String wifi_password;
      String key_serveur;
      DEBUG_PRINTLN("Topic  : " + Topic);
      DEBUG_PRINTLN("Instruction reçue : " + Instruction);

    /* =========================================
        Commandes envoyées par le serveur
    * ========================================= */  
    if (Topic.equals(topic_instruction))
    {
      /* =========================================
      * START
      * Autorise le programme à démarrer
      * ========================================= */
      if(Instruction == START)
      {
            this->startProgram();
            init_Tile();
            DEBUG_PRINTLN("Authentification réussie. Début du programme.");
      }

      /* =========================================
      * STOP
      * Interdit au programme de s'exécuter
      * ========================================= */
      else if(Instruction == STOP)
      {
            this->stopProgram();
            DEBUG_PRINTLN("L'objet n'est plus authentifié. Fin du programme."); 
      }
      /* =========================================
      * CONNECT
      * Le serveur veut savoir si l'objet est connecté
      * ========================================= */
      else if(Instruction == CONNECT)
      {
        String Adresse_MAC = this->getAddrMac();
        // on envoie les deux premières lettres de l'@mac pour indiquer la bonne présence de l'objet
        this->MQTT_Send(this->topic_connect,Adresse_MAC.substring(0,2));
      } 

      /* =========================================
      * WIFI_DATA;ssid;password;cle_cryptage
      * L'objet connecté reçoit les infos de connexion wifi (SSID/PASSWORD)
      * et doit les enregistrer dans sa mémoire EEPROM
      * ========================================= */
      else if(Instruction.startsWith(WIFI_DATA))
      {
          wifi_ssid     = ParseString(Instruction, ';' , 1);
          wifi_password = ParseString(Instruction, ';' , 2);
          key_serveur   = ParseString(Instruction, ';' , 3);

          #ifdef DEBUG_WIFI_DATA
            DEBUG_PRINT("SSID :");
            DEBUG_PRINTLN(wifi_ssid);
            DEBUG_PRINT("PASSWORD :");
            DEBUG_PRINTLN(wifi_password);
            DEBUG_PRINT("CLEF DE CHIFFREMENT : ");
            DEBUG_PRINTLN(key_serveur);
          #endif
          Data = wifi_ssid + ";";
          Data += wifi_password;

          if(key_serveur.length()>0){
              Data += ";" + key_serveur;
          }
          Write_STR_EEPROM((char*)Data.c_str(),100,EEPROM_ADDR_WIFI);
      }
    }

    /* =========================================
        Commandes envoyées par l'utilisateur
    * ========================================= */ 
    else if(Topic.startsWith(topic_tile))
    {
      String TileTopic = Topic.substring(topic_tile.length());
      callBack_Tile(TileTopic,Instruction);
    }
}


// ################################################################################
// 						                    TILE DASHBOARD
// ################################################################################ 

  /*===============================================================================
    Nom 			: SendtoTile
    
    Description	: Envoie une trame de données à une tile
    
    Paramètre(s) 	: 
    * topic 		: topic de la tile
    * payload		: trame de données à envoyer
    
    Retour		: aucun
  ===============================================================================*/
  void Domokit::SendtoTile(String topic, String Payload)
  {
    String mTopic = topic_tile + "/" + topic;
    this->MQTT_Send(mTopic,Payload);
  }

    /*===============================================================================
    Nom 			: 	setTile
    
    Description	: Permet de définir une Tile qui sera affichée sur l'application mobile
    - L'objet envoie une trame au serveur contenant les données de la Tile
    - Le serveur vérifie si la tile existe ou non (repéré par l'ID donné)

    Paramètre(s) 	: 
    - Titre : titre de la Tile
    - Type : type de Tile voulue (voir #define TILE dans Domokit.h)
    - Topic : topic de réception/envoi de la Tile 
    - levelMax/levelMin : définition du niveau pour certaines tiles ( jauges, graphiques )

    Retour		: rien
  ===============================================================================*/
  void Domokit::setTile(String Titre, int Type, String Topic, int levelMin, int levelMax, String onIcon, String offIcon)
  {
      String mTopic = topic_tile + "/" + Topic; 
      composeSetTilePayload("delete",""); delay(1000);
      composeSetTilePayload("Titre",Titre);
      composeSetTilePayload("Type",String(Type));
      composeSetTilePayload("Topic",mTopic);
      
      composeSetTilePayload("LevelMin",String(levelMin));
      composeSetTilePayload("LevelMax",String(levelMax));
      

      if(onIcon != "")
        composeSetTilePayload("OnIcon",onIcon);
      if(offIcon != "")
        composeSetTilePayload("OffIcon",offIcon);

      _TileID++;
      delay(10);
  }

/*===============================================================================
  Nom 			: composeSetTilePayload
  
  Description	: Permet d'envoyer les trames de configuration des Tiles au serveur
  
  Paramètre(s) 	:
  - attribut : nom du paramètre à modifier (ex : 'Titre', 'Topic', etc...)
  - valeur : valeur du paramètre à appliquer
  
  Retour		: aucun
===============================================================================*/
  void Domokit::composeSetTilePayload(String attribut, String valeur)
  {
      String payload = String(_TileID) + ";" + attribut + ";" + valeur;
      this->MQTT_Send(topic_set_tile,payload);
  }

/*===============================================================================
    Nom 			: 	setTileText
    
    Description	: Définit une Tile de type Texte (pilotable ou affichage uniquement )
  ===============================================================================*/
  void Domokit::setTileText(String Titre, String Topic, bool enablePub)
  {
    if (enablePub)
    {
      this->setTile(Titre,TILE_TEXT_DRIVE,Topic,0,0,"","");
    }
    else
    {
      this->setTile(Titre,TILE_TEXT_DISPLAY,Topic,0,0,"","");
    }
  }

/*===============================================================================
    Nom 			: 	setTileSwitch
    
    Description	: Définit une Tile de type Switch
  ===============================================================================*/
  void Domokit::setTileSwitch(String Titre, String Topic)
  {
      this->setTile(Titre,TILE_SWITCH,Topic,0,0,"","");
  }

  /*===============================================================================
    Nom 			: 	setTileGraph
    
    Description	: Définit une Tile de type Switch
  ===============================================================================*/
  void Domokit::setTileGraph(String Titre, String Topic,int levelMin,int levelMax)
  {
      this->setTile(Titre,TILE_GRAPH,Topic,levelMin,levelMax,"","");
  }

  /*===============================================================================
    Nom 			: 	setTileJauge
    
    Description	: Définit une Tile de type Jauge
  ===============================================================================*/
  void Domokit::setTileJauge(String Titre, String Topic, bool enablePub,int levelMin,int levelMax)
  {
    if (enablePub)
    {
      this->setTile(Titre,TILE_JAUGE_DRIVE,Topic,levelMin,levelMax,"","");
    }
    else
    {
      this->setTile(Titre,TILE_JAUGE_DISPLAY,Topic,levelMin,levelMax,"","");
    }
      
  }

  /*===============================================================================
    Nom 			: 	setTileRadioButton
    
    Description	: Définit une Tile de type RadioButton

    Remarque : les icônes pour onIcon et offIcon sont disponibles ici 
    https://fontawesome.com/v4.7.0/cheatsheet/
    ex : fa-android , fa-ban , fa-circle-o , etc...
  ===============================================================================*/
  void Domokit::setTileRadioButton(String Titre, String Topic, String onIcon, String offIcon)
  {
    this->setTile(Titre,TILE_RADIOBUTTON,Topic,0,0,onIcon,offIcon);
  }

    /*===============================================================================
    Nom 			: 	setTileIcon
    
    Description	: Définit une Tile de type Icon

    Remarque : les icônes pour l'icône sont disponibles ici 
    https://fontawesome.com/v4.7.0/cheatsheet/
    ex : fa-android , fa-ban , fa-circle-o , etc...
  ===============================================================================*/
  void Domokit::setTileIcon(String Titre, String Topic)
  {
    this->setTile(Titre,TILE_ICON,Topic,0,0,"","");
  }

    /*===============================================================================
    Nom 			: 	sendIcon
    
    Description	: Envoie un icône à une tile de type Icon

    Remarque : les icônes pour l'icône sont disponibles ici 
    https://fontawesome.com/v4.7.0/cheatsheet/
    ex : fa-android , fa-ban , fa-circle-o , etc...

    la couleur doit être au format :
    #RRGGBB #AARRGGBB 'red', 'blue', 'green', 'black', 'white', 'gray', 'cyan', 'magenta', 'yellow', 'lightgray', 'darkgray' 
  ===============================================================================*/
  void Domokit::SendIconToTile(String topic, String fa_icon, String color)
  {
      String payload = fa_icon + ";" + color;
      SendtoTile(topic,payload);
  }

// ################################################################################
// 						Fonctions externes à la classe Domokit
// ################################################################################

/*===============================================================================
  Nom 			: 	ParseString
  
  Description	: 	Permet de récupérer une partie d'une chaîne de caractère String
                  selon un caractère de parse fourni
					
  Paramètre(s) 	: data : chaine à découper
                  separator : caractère de parse
                  index : n° de la chaîne découpée à récupérer. ex : part0;part1;part2;...
  
  Retour		: 	Chaîne découpée souhaitée
===============================================================================*/
String ParseString(String data, char separator, int index)
{
  int found = 0;
  int strIndex[] = {0, -1};
  int maxIndex = data.length()-1;

  for(int i=0; i<=maxIndex && found<=index; i++){
    if(data.charAt(i)==separator || i==maxIndex){
        found++;
        strIndex[0] = strIndex[1]+1;
        strIndex[1] = (i == maxIndex) ? i+1 : i;
    }
  }

  return found>index ? data.substring(strIndex[0], strIndex[1]) : "";
}


/*===============================================================================
  Nom 			: 	  Read_STR_EEPROM
  
  Description	: 	Permet de lire une partie de la mémoire EEPROM sous forme de String
					
  Paramètre(s) 	: taille : nombre d'octets à lire
                  addr_debut : adresse mémoire où on va lire
  
  Retour		: 	  Contenu de la mémoire sous forme de String
===============================================================================*/
String Read_STR_EEPROM(int taille, int addr_debut)
{
  DEBUG_PRINTLN("Read EEPROM ! ");
 char str[taille];
	EEPROM.begin(taille);

 for (int j=0; j<taille; j++) 
 {
   str[j] =  char(EEPROM.read(j + addr_debut));
 }
 str[taille] = '\0';

 return String(str);
}

/*===============================================================================
  Nom 			: 	  Write_STR_EEPROM
  
  Description	: 	Permet d'écrire une chaîne de caractère en mémoire EEPROM
					
  Paramètre(s) 	: str : chaîne de caractère à écrire
                  taille : nombre d'octets sur lesquels on va écrire
                  addr_debut : adresse mémoire où on va écrire
  
  Retour		: 	  aucun
===============================================================================*/
void Write_STR_EEPROM(char str[], int taille, int addr_debut)
{
  DEBUG_PRINTLN("Write EEPROM ! ");
  EEPROM.begin(taille);

  for (int i=0; i<taille; i++) {
     EEPROM.write(i + addr_debut,str[i]);
  }
  EEPROM.commit();
}


/*===============================================================================
  Nom 			: 	  Cryptage
  
  Description	: 	Permet de crypter une chaîne de caractère (String), selon une clé de chiffrement
					
  Paramètre(s) 	: src : chaîne à crypter
                  key : clé de chiffrement
  
  Retour		: 	  Chaîne cryptée
===============================================================================*/
String Cryptage(char* src, String key)
{
  char Dest[512];
  if (key_serveur_dispo)
  {
    for (unsigned int i=0 ; i < strlen(src) ; i++)
    {
      // Mettre algo de cryptage ici
      Dest[i] = src[i];
    }
    Dest[strlen(src)] = '\0';
    return String(Dest);
  }
  else
  {
    return String(src);
  }
}

/*===============================================================================
  Nom 			: 	  Decryptage
  
  Description	: 	Permet de décrypter une chaîne de caractère (String), selon une clé de chiffrement
					
  Paramètre(s) 	: src : chaîne à décrypter
                  key : clé de chiffrement
  
  Retour		: 	  Chaîne décryptée
===============================================================================*/
String Decryptage(char* src, String key)
{
  char Dest [512];
  if (key_serveur_dispo)
  {
    for (unsigned int i=0 ; i < strlen(src) ; i++)
    {
      // Mettre algo de décryptage ici
      Dest[i] = src[i];
    }
    Dest[strlen(src)] = '\0';
    return String(Dest);
  }
  else
  {
    return String(src);
  }
}
