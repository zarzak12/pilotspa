# PilotSpa - Système de Contrôle pour Spa Balboa

[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)
[![ESP32](https://img.shields.io/badge/ESP32-Compatible-blue.svg)](https://www.espressif.com/)
[![ESPHome Compatible](https://img.shields.io/badge/ESPHome-Compatible-green.svg)](https://esphome.io/)
[![Home Assistant](https://img.shields.io/badge/Home%20Assistant-Compatible-blue.svg)](https://www.home-assistant.io/)

## Vue d'ensemble

PilotSpa est un système complet de contrôle basé sur ESP32 pour les spas Balboa équipés du panneau VL801D DeluxeSerie. Il propose deux approches d'implémentation :
- **Arduino IDE** avec interface web intégrée et MQTT
- **ESPHome** avec intégration native Home Assistant

<img src="https://github.com/zarzak12/pilotspa/blob/main/extra/img/carte-electronique-sf273-spaform-aquamarine.jpg" width="400" alt="Carte PCB"> <img src="https://github.com/zarzak12/pilotspa/blob/main/extra/img/VL801D-balboa-panel-spa-studio-WEB_ex.jpg" width="400" alt="Panneau Balboa">

---

## Table des matières

- [Spécifications Matérielles](#spécifications-matérielles)
  - [Broches RJ45](#broches-rj45)
  - [Configuration des Pins](#configuration-des-pins)
- [Analyse du Protocole](#analyse-du-protocole)
  - [Données d'Affichage](#données-daffichage)
  - [Données des Boutons](#données-des-boutons)
- [Méthodes d'Upload](#méthodes-dupload)
  - [Arduino IDE](#option-1--via-arduino-ide)
  - [ESPHome](#option-2--via-esphome)
- [Fonctionnalités](#fonctionnalités)
- [Installation](#installation--première-configuration)
- [Fabrication PCB](#fabrication-pcb)
- [Références](#références)

---

## Spécifications Matérielles

### Broches RJ45

<img src="https://github.com/zarzak12/pilotspa/blob/main/extra/img/RJ45.png" width="200" alt="Pinout RJ45">

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

### Configuration des Pins

**Lignes de Signal Critiques :**
- **BROCHE 6 (Jaune)** : Signal d'horloge
- **BROCHE 5 (Bleu)** : Données d'affichage
- **BROCHE 3 (Rose)** : Données des boutons

<img src="https://github.com/zarzak12/pilotspa/blob/main/extra/img/DATA-1.jpg" width="800" alt="Analyse Signal 1"> 
<img src="https://github.com/zarzak12/pilotspa/blob/main/extra/img/DATA-2.jpg" width="800" alt="Analyse Signal 2">

---

## Analyse du Protocole

### Caractéristiques du Signal

- **Pulsations d'Horloge** : 7 pulsations par bloc de données d'affichage (71 bits), 4 pour les données de boutons
- **Cycle Complet** : 75 pulsations totales
- **Blocs de Données** : 6 blocs de données d'affichage par cycle
- **Fréquence de Cycle** : Toutes les 10 ms (100 Hz)
- **Durée d'un Cycle Complet** : 3.54 ms
- **Durée Bit Données d'Affichage** : 23.75 μs
- **Durée Bit Données des Boutons** : 71.5 μs

### Données d'Affichage

**Exemple : Affichage de Température 29.5°C**

L'affichage encode les valeurs en utilisant le format BCD vers LCD 7 segments (7 bits, LSB toujours 0) :

```
2 = 1101101(0) → Chiffre 1
9 = 1110011(0) → Chiffre 2
5 = 1011011(0) → Chiffre 3
C = 1001110(0) → Chiffre 4
```

**Table de Mappage des Bits :**

| Bloc 1 | Bloc 2 | Bloc 3 | Bloc 4 | Bloc 5 (Bits d'État) | 
| --- | --- | --- | --- | --- |
| Seg LCD 1 | Seg LCD 2 | Seg LCD 3 | Seg LCD 4 | Bit 29: Temp ±  |   
| | | | | Bit 39: Bouton  | 
| | | | | Bit 40: Bouton  | 
| | | | | Bit 42: Ventilateur | 
| | | | | Bit 44: Pompe 3  |
| | | | | Bit 47: Lumière   |
| | | | | Bit 48: Pompe 1   |
| | | | | Bit 49: Pompe 2   |
| | | | | Bit 59: Standard  |
| | | | | Bit 60: Eco       |

### Données des Boutons

Les commandes de contrôle sont codées en séquences de 4 bits :

| Bouton | Code Binaire | 
| --- | --- |
| Mode | 1 0 0 0 |
| Température ↑ | 1 1 1 0 |
| Température ↓ | 1 1 1 1 |  
| Lumière | 1 0 1 1 |
| Pompe 1 | 1 0 0 0 | 
| Pompe 2 | 1 0 1 0 |
| Ventilateur | 1 1 0 1 |

---

## Cartographie de Connexion ESP32

- **Broche Horloge** : GPIO 23
- **Broche Lecture (Données d'Affichage)** : GPIO 22
- **Broche Écriture (Contrôle Boutons)** : GPIO 13

#### Conclusions

- L'horloge pulse 7 fois pour chaque bloc de données d'affichage, soit 71 bits, et 4 fois pour les données de boutons. Un cycle complet = 75 pulsations. <br />
- Total de 6 blocs de données d'affichage par cycle <br />
- Chaque cycle est répété toutes les 10 ms <br />
- Durée d'un cycle complet : 3.54 ms
- Durée du bit pour les données d'affichage : 23.75 microsecondes <br />
- Durée du bit pour les données de boutons : 71.5 microsecondes <br />

---

## Fabrication PCB & Schéma

### Conception PCB

<img src="https://github.com/Shuraxxx/-Balboa-GS523SZ-with-panel-VL801D-DeluxeSerie-/blob/main/extras/PCB%20layout/PCB_3D_view.jpg" width="303" alt="Vue 3D PCB"> <img src="https://github.com/Shuraxxx/-Balboa-GS523SZ-with-panel-VL801D-DeluxeSerie-/blob/main/extras/PCB%20layout/PCB_2D_view.jpg" width="400" alt="Vue 2D PCB">

### Schéma Électrique

<img src="https://github.com/zarzak12/pilotspa/blob/main/extra/PCB%20Layout/PCB_Schematic_Balboa.jpg" width="800" alt="Schéma PCB"> 

**Fichiers Gerber** : [PCB Balboa-GS510SZ](https://github.com/Shuraxxx/-Balboa-GS523SZ-with-panel-VL801D-DeluxeSerie-/tree/main/extras/PCB%20layout)

---

## Fonctionnalités

### Capacités Principales

- **Connectivité Wi-Fi**
  - Connexion automatique au démarrage
  - Mode SoftAP de secours pour la configuration
  - Stockage configurable du SSID et mot de passe en EEPROM

- **Interface Web Serveur**
  - Tableau de bord HTML configurable
  - Affichage temps réel de l'état du spa
  - Design réactif avec W3.CSS

- **Intégration MQTT**
  - Publication des données de télémétrie du spa
  - Abonnement aux commandes de contrôle
  - Point de terminaison du courtier configurable

- **Contrôle du Spa**
  - Ajustement de la température (↑/↓)
  - Contrôle des pompes (1, 2, 3)
  - Contrôle du ventilateur
  - Contrôle de la lumière
  - Commutation de mode (Standard/Eco)

- **Mises à Jour Over-The-Air**
  - Support ElegantOTA (Arduino IDE)
  - OTA intégré (ESPHome)
  - Mises à jour du firmware sans fil

- **Intégration Home Assistant**
  - Découverte MQTT
  - Intégration native ESPHome
  - Entités capteur pour tous les contrôles

---

## Méthodes d'Upload

### Option 1 : Via Arduino IDE

#### Prérequis
- Arduino IDE 1.8.19 ou supérieur
- Paquet boards ESP32 (v2.0.0+)
- Pilote USB CH340/CH341 (si nécessaire)

#### Étapes de Configuration

1. **Installer Arduino IDE** et configurer les boards ESP32
   ```
   Tools → Board Manager → Rechercher "esp32" → Installer
   ```

2. **Ajouter les Dépendances de Bibliothèque**
   ```
   Sketch → Inclure la Bibliothèque → Gérer les Bibliothèques
   Rechercher et installer : ArduinoJson, PubSubClient, WiFiManager
   ```

3. **Préparer la Structure du Projet**
   - Copier `src/Balboa_GS_Interface.cpp` et `.h` dans le dossier du sketch
   - Créer le sous-dossier `data/` avec les fichiers SPIFFS :
     - `config.html` - Interface de configuration
     - `index.html` - Tableau de bord principal
     - `pilotage.html` - Panneau de contrôle
     - `w3.css` - Feuille de style

4. **Télécharger les Fichiers SPIFFS**
   ```
   Tools → ESP32 Sketch Data Upload
   ```

5. **Compiler et Téléverser**
   ```
   Sélectionner Board : ESP32 Dev Module
   Sélectionner Port : COM# (votre port ESP32)
   Upload Speed : 921600
   Cliquer : Upload
   ```

#### Ressources

| Élément | Localisation |
|--------|-------------|
| Code Source | `src/` |
| Fichiers UI | `examples/data/` |
| Exemple Complet | `examples/Balboa_GS_HomeAssistant.ino` |

---

### Option 2 : Via ESPHome

#### Prérequis
- Instance Home Assistant avec module ESPHome
- Connaissance de la configuration YAML
- Connexion USB vers ESP32

#### Étapes de Configuration

1. **Créer un Appareil ESPHome**
   ```yaml
   esphome:
     name: balboa-spa
     platform: ESP32
     board: esp32devkitc
   ```

2. **Ajouter le Composant Personnalisé**
   - Copier `esphome/components/balboa_spa/` vers votre répertoire config ESPHome
   - Référencer dans votre YAML d'appareil :
   ```yaml
   external_components:
     - source: local
       components: [balboa_spa]
   ```

3. **Charger la Configuration**
   - Utiliser `esphome/balboa_spa.yaml` fourni comme modèle
   - Personnaliser les paramètres Wi-Fi et MQTT

4. **Build et Upload**
   ```
   ESPHome Dashboard → Sélectionner Appareil → Build & Upload
   ```

#### Fichiers du Composant ESPHome

| Fichier | Utilité |
|---------|---------|
| `__init__.py` | Initialisation du composant |
| `sensor.py` | Capteurs de température et d'état |
| `text_sensor.py` | Informations basées sur du texte |
| `binary_sensor.py` | États booléens |
| `climate.py` | Contrôle du thermostat |
| `balboa_spa.h` | Code d'interface C++ |

#### Avantages d'ESPHome

✅ Intégration native Home Assistant  
✅ Pas de serveur web externe nécessaire  
✅ Mises à jour OTA simplifiées  
✅ Configuration basée sur YAML  
✅ Support communautaire actif  

---

## Comparaison : Arduino IDE vs ESPHome

| Aspect | Arduino IDE | ESPHome |
|--------|-------------|---------|
| **Courbe d'Apprentissage** | Modérée | Faible |
| **Interface Web** | Incluse | Via Home Assistant |
| **Configuration** | Code C++ | YAML |
| **Home Assistant** | Pont MQTT | Intégration Native |
| **Mises à Jour OTA** | Configuration Manuelle (ElegantOTA) | Intégré |
| **Flexibilité** | Élevée (Code C++ complet) | Moyenne (basé sur YAML) |
| **Maintenance** | Auto-gérée | Maintenue par la communauté |

---

## Installation & Première Configuration

### Configuration Initiale

1. **Télécharger SPIFF vers ESP32**
   ```
   Arduino IDE : Tools → ESP32 Sketch Data Upload
   ```

2. **Téléverser le Firmware**
   ```
   Arduino IDE : Sketch → Upload
   ```

3. **Se Connecter au Portail de Configuration Wi-Fi**
   - L'appareil diffuse : `OpenSpa-config`
   - Mot de passe : `openspa2801`
   - Ouvrir le navigateur : http://192.168.4.1

4. **Configurer les Paramètres Wi-Fi**
   - Entrer les identifiants de votre réseau local
   - Définir les détails du courtier MQTT (si utilisé)
   - Enregistrer la configuration

5. **Se Connecter au Réseau Local**
   - Accéder à l'appareil sur l'IP locale (affichée lors de la configuration)
   - Exemple : http://192.168.1.33

---

## Dépannage

### Problèmes Courants

**La Connexion Wi-Fi Échoue**
- Vérifier le SSID/mot de passe dans la page de config
- Vérifier que le routeur diffuse le SSID
- Redémarrer l'appareil si nécessaire

**Pas d'Interface Web**
- Confirmer que les fichiers SPIFF ont été téléchargés
- Vérifier l'adresse IP dans le moniteur série
- Essayer la réinitialisation d'usine : Appuyer 3x rapidement sur le bouton RESET

**Problèmes de Connexion MQTT**
- Vérifier que le courtier fonctionne et est accessible
- Vérifier les identifiants et les paramètres de port
- Surveiller la sortie série pour les messages d'erreur

---

## Références

### Documentation Technique

- [Balboa GS510SZ avec VL801D Contrôle MQTT](https://github.com/Shuraxxx/-Balboa-GS523SZ-with-panel-VL801D-DeluxeSerie--MQTT)
- [Analyse du Protocole d'Affichage Balboa](https://docs.google.com/document/u/0/d/1xuc5N6-Q7J7bQxBARYjUywhqkIFjOnh8vAHqEya5QAI/mobilebasic)
- [Référence Mesure d'Affichage Balboa](https://github.com/NickB1/OpenSpa/blob/master/documents/Balboa/Balboa_Display_Measurements.pdf)
- [Internet des Spas - Vue d'ensemble Technique](https://www.olivierhill.ca/archives/72-The-Internet-of-Spas.html)
- [Affichage LED 7 Segments avec Décaleur](https://create.arduino.cc/projecthub/meljr/7-segment-led-displays-102-using-a-shift-register-6b6976)

### Projets Connexes

- [Contrôleur Série GL2000](https://github.com/netmindz/balboa_GL_ML_spa_control)
- [Analyse du Protocole Balboa Worldwide App](https://github.com/ccutrer/balboa_worldwide_app)
- [Implémentation Complète Balboa GS510SZ](https://github.com/MagnusPer/Balboa-GS510SZ/)

---

## Licence

Ce projet est licencié sous la Licence MIT - voir le fichier LICENSE pour plus de détails.

---

## Contribution

Les contributions sont bienvenues ! N'hésitez pas à soumettre une Pull Request pour toute amélioration, correction de bug ou amélioration de la documentation.

---

**Dernière mise à jour** : 2026-06-15  
**Version** : 1.0  
**Matériel Supporté** : ESP32, Panneau Balboa VL801D DeluxeSerie

## Fabrication PCB

<img src="https://github.com/Shuraxxx/-Balboa-GS523SZ-with-panel-VL801D-DeluxeSerie-/blob/main/extras/PCB%20layout/PCB_3D_view.jpg" width="303"> <img src="https://github.com/Shuraxxx/-Balboa-GS523SZ-with-panel-VL801D-DeluxeSerie-/blob/main/extras/PCB%20layout/PCB_2D_view.jpg" width="400">

## Schéma

<img src="https://github.com/zarzak12/pilotspa/blob/main/extra/PCB%20Layout/PCB_Schematic_Balboa.jpg" width="800">
