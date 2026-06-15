# PilotSpa

## Contrôle de Spa Balboa avec ESP32

Balboa avec panneau VL801D DeluxeSerie et carte SF273

<img src="https://github.com/zarzak12/pilotspa/blob/main/extra/img/carte-electronique-sf273-spaform-aquamarine.jpg" width="400"> <img src="https://github.com/zarzak12/pilotspa/blob/main/extra/img/VL801D-balboa-panel-spa-studio-WEB_ex.jpg" width="400">

## Broches RJ45

<img src="https://github.com/zarzak12/pilotspa/blob/main/extra/img/RJ45.png" width="200">

| BROCHE       | Description             | 
| ------------ | ---------------------- |
| BROCHE 1     | Fonctionnalité incertaine  |
| BROCHE 2     | Fonctionnalité incertaine  |
| BROCHE 3     | Données boutons        |  
| BROCHE 4     | GND                    |
| BROCHE 5     | Données affichage      | 
| BROCHE 6     | Horloge                |
| BROCHE 7     | 5 VDC                  |  
| BROCHE 8     | Sortie flottante       |

### BROCHE 6 (Jaune - horloge), BROCHE 5 (Bleu - données affichage), BROCHE 3 (Rose - données boutons)

<img src="https://github.com/zarzak12/pilotspa/blob/main/extra/img/DATA-1.jpg" width="800"> 
<img src="https://github.com/zarzak12/pilotspa/blob/main/extra/img/DATA-2.jpg" width="800"> 

#### Conclusions

- L'horloge pulse 7 fois pour chaque bloc de données d'affichage, soit 71 bits, et 4 fois pour les données de boutons. Un cycle complet = 75 pulsations. <br />
- Total de 6 blocs de données d'affichage par cycle <br />
- Chaque cycle est répété toutes les 10 ms <br />
- Durée d'un cycle complet : 3.54 ms
- Durée du bit pour les données d'affichage : 23.75 microsecondes <br />
- Durée du bit pour les données de boutons : 71.5 microsecondes <br />

#### Données d'Affichage

L'affichage montre **29.5C** - Comment cela se traduit-il en flux binaire ? Les 4 premiers blocs sont codés en BCD vers LCD 7 segments (voir référence) mais seulement avec 7 bits puisque le LSB est toujours (0). <br />
2 = 1101101(0) = (Bloc 1) <br />
9 = 1110011(0) = (Bloc 2) <br />
5 = 1011011(0) = (Bloc 3) <br />
C = 1001110(0) = (Bloc 4) <br />

| Bloc 4 - bit(0-7) | Bloc 3 - bit(7-14) | Bloc 2 - bit(14-21) | Bloc 4 - bit(21-28) | Bloc 5 - bit(29-71) | 
| ------------------- | ------------------- | ------------------   | -------------------- | -------------------- |                     
| Segment LCD 1      |  Segment LCD 2     |   Segment LCD 3     |   Segment LCD 4      | 29 - Bouton haut/bas |   
|                    |                     |                      |                      |       ???            |  
|                    |                     |                      |                      | 39 - Bouton          | 
|                    |                     |                      |                      | 40 - Bouton          | 
|                    |                     |                      |                      | 42 - Ventilateur     | 
|                    |                     |                      |                      | 44 - Pompe3          |
|                    |                     |                      |                      | 47 - Lumière         |
|                    |                     |                      |                      | 48 - Pompe1          |
|                    |                     |                      |                      | 49 - Pompe2          |
|                    |                     |                      |                      | 59 - Mode Standard   | 
|                    |                     |                      |                      | 60 - Mode Eco        | 
|                    |                     |                      |                      |        ???           |  

<img src="https://github.com/MagnusPer/Balboa-GS510SZ/blob/main/extras/docs/Balboa_GS_Interface.jpg" width="1000">

Pour la connexion, j'ai choisi un ESP 32 et j'ai relié :

Broche horloge : 23</br>
Broche lecture  : 22</br>
Broche écriture : 13</br>

#### Données des boutons

|  Bouton   |    Décodage bit(0-3) | 
| --------- | -------------------- |
| Mode      |   1 0 0 0            |
| Temp +    |   1 1 1 0            |
| Temp -    |   1 1 1 1            |  
| Lumière   |   1 0 1 1            |
| Pompe 1   |   1 0 0 0            | 
| Pompe 2   |   1 0 1 0            |
| Ventilo   |   1 1 0 1            |

## Fabrication PCB

<img src="https://github.com/Shuraxxx/-Balboa-GS523SZ-with-panel-VL801D-DeluxeSerie-/blob/main/extras/PCB%20layout/PCB_3D_view.jpg" width="303"> <img src="https://github.com/Shuraxxx/-Balboa-GS523SZ-with-panel-VL801D-DeluxeSerie-/blob/main/extras/PCB%20layout/PCB_2D_view.jpg" width="400">

## Schéma

<img src="https://github.com/zarzak12/pilotspa/blob/main/extra/PCB%20Layout/PCB_Schematic_Balboa.jpg" width="800"> 

Les fichiers Gerber sont disponibles ici : [PCB Balboa-GS510SZ](https://github.com/Shuraxxx/-Balboa-GS523SZ-with-panel-VL801D-DeluxeSerie-/tree/main/extras/PCB%20layout )

## Vue d'ensemble

Ce projet est une application pour contrôler un système de spa Balboa avec un ESP32, offrant une interface utilisateur web et l'intégration MQTT. Voici un résumé des principales fonctionnalités :

- **Connexion Wi-Fi** : Le code gère la connexion Wi-Fi au démarrage. En cas d'échec de connexion Wi-Fi, le code bascule en mode SoftAP pour permettre la configuration. Un serveur web est configuré pour la gestion de la configuration Wi-Fi.

- **EEPROM** : Les informations de configuration telles que SSID, mot de passe, etc., sont stockées dans l'EEPROM.

- **Serveur Web** : Un serveur web est configuré avec des pages HTML pour afficher l'état du spa et permettre la configuration.

- **MQTT** : Le code se connecte à un courtier MQTT avec les informations de configuration. Les données d'état du spa sont publiées via MQTT.

- **OTA (Over-The-Air)** : Le code supporte les mises à jour Over-The-Air, permettant les mises à jour du firmware sans fil.

- **Interface Balboa** : Le code utilise une interface Balboa pour interagir avec le système de spa. Les commandes sont envoyées au système Balboa en fonction des actions de l'utilisateur.

- **Intégration Home Assistant** : Les données du spa sont exposées comme des capteurs, des interrupteurs, etc., pour l'intégration avec Home Assistant. Les boutons virtuels de l'interface utilisateur déclenchent des commandes sur le système Balboa.

- **Horodatage** : L'ESP32 est configuré pour utiliser NTP pour l'horodatage.

- **Débogage** : Les informations de débogage sont affichées sur le port série. Une page de configuration spécifique est protégée par un nom d'utilisateur et un mot de passe.

- **Mécanisme de synchronisation (Ticker)** : Un mécanisme de synchronisation (Ticker) est utilisé pour vérifier périodiquement la connexion Wi-Fi.

- **Fonction de reconnexion Wi-Fi** : En cas de perte de connexion Wi-Fi, le code tente de se reconnecter.

## Méthodes d'upload sur ESP32

### Option 1 : Via Arduino IDE

#### Étapes :
1. **Installer Arduino IDE** et ajouter les cartes ESP32 via le gestionnaire de cartes
2. **Copier le code source** depuis le dossier `src/` (Balboa_GS_Interface.cpp et .h)
3. **Télécharger les fichiers de l'IHM** depuis le dossier `examples/data/` vers la mémoire flash SPIFFS de l'ESP32 :
   - `config.html` - Page de configuration Wi-Fi
   - `index.html` - Interface principale de contrôle
   - `pilotage.html` - Interface de pilotage
   - `w3.css` - Feuille de style
4. **Compiler et téléverser** vers l'ESP32

#### Ressources :
- Dossier source : `src/`
- Fichiers de l'IHM : `examples/data/`
- Exemple complet : `examples/Balboa_GS_HomeAssistant.ino`

---

### Option 2 : Via ESPHome

#### Étapes :
1. **Configurer ESPHome** avec le fichier de configuration YAML
2. **Utiliser le composant personnalisé** situé dans `esphome/components/balboa_spa/`
3. **Charger la configuration** `esphome/balboa_spa.yaml` dans votre instance ESPHome
4. **Compiler et téléverser** via l'interface ESPHome

#### Avantages d'ESPHome :
- Intégration directe avec Home Assistant
- Gestion simplifiée des dépendances
- Pas besoin d'IHM Web externe (accessible via Home Assistant)
- Mises à jour OTA intégrées

#### Composant ESPHome :
- Fichier principal : `esphome/components/balboa_spa/__init__.py`
- Capteurs : `sensor.py`, `text_sensor.py`, `binary_sensor.py`
- Contrôle climatique : `climate.py`
- En-têtes C++ : `balboa_spa.h`

---

## Comparaison des deux approches

| Caractéristique | Arduino IDE | ESPHome |
|---|---|---|
| Complexité | Moyenne | Faible |
| IHM Web | Oui (incluse) | Non (via Home Assistant) |
| Intégration HA | MQTT | Nativ |
| Mise à jour OTA | Manuel (ElegantOTA) | Intégré |
| Configuration | Code C++ | YAML simple |

---

## Installation initiale

- Télécharger SPIFF dans l'ESP32
- Téléverser le code généré dans l'ESP32
- Connecter vos appareils Wi-Fi au Wi-Fi OpenSpa-config avec le mot de passe openspa2801
- Appeler l'adresse IP d'OpenSpa-config (ex : http://192.168.4.1)
- Définir toutes les informations sur la page des paramètres
- Appeler la nouvelle adresse IP connectée à votre réseau local (ex : http://192.168.1.33)

## Références utiles

- https://github.com/Shuraxxx/-Balboa-GS523SZ-with-panel-VL801D-DeluxeSerie--MQTT
- https://docs.google.com/document/u/0/d/1xuc5N6-Q7J7bQxBARYjUywhqkIFjOnh8vAHqEya5QAI/mobilebasic
- https://github.com/NickB1/OpenSpa/blob/master/documents/Balboa/Balboa_Display_Measurements.pdf
- https://www.olivierhill.ca/archives/72-The-Internet-of-Spas.html
- https://create.arduino.cc/projecthub/meljr/7-segment-led-displays-102-using-a-shift-register-6b6976

## Autres projets Balboa

- GL2000 Series https://github.com/netmindz/balboa_GL_ML_spa_control
