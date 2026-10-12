# 🔥 Robot anti-incendie Arduino

Robot mobile autonome capable de **détecter une flamme**, **s'orienter vers elle**, **s'arrêter à bonne distance** puis **l'éteindre avec un jet d'eau** balayé de gauche à droite par un servomoteur.

## Fonctionnement en bref

1. Trois capteurs de flamme analogiques (gauche, centre, droite) mesurent l'intensité infrarouge.
2. Plus la valeur lue est **basse**, plus la flamme est **forte / proche**.
3. Le robot tourne vers le capteur le plus fort, puis avance par petits pas.
4. Quand une valeur passe sous le seuil d'arrêt, le robot s'immobilise et lance l'arrosage.
5. S'il n'y a plus de flamme, il reste à l'arrêt, pompe coupée.

## Matériel

| Composant | Rôle |
|---|---|
| Arduino Uno (ou compatible) | Cerveau du robot |
| Driver moteur L298N | Pilotage des moteurs (2 canaux : gauche / droite) |
| Moteurs DC + roues | Déplacement (type char : virage sur place) |
| 3 capteurs de flamme analogiques | Détection de la direction et de la distance |
| Servomoteur | Oriente la buse de la pompe |
| Module relais | Commande de la pompe à eau |
| Pompe à eau + tuyau | Extinction |
| Alimentation séparée pour moteurs/pompe | Évite les chutes de tension sur l'Arduino |

## Câblage

### Moteurs (L298N)

| Signal | Broche Arduino | Remarque |
|---|---|---|
| IN1 | 10 | Moteur droit, sens 1 |
| IN2 | 9 | Moteur droit, sens 2 |
| IN3 | 8 | Moteur gauche, sens 1 |
| IN4 | 7 | Moteur gauche, sens 2 |
| ENA | 11 (PWM) | Vitesse moteur droit |
| ENB | 6 (PWM) | Vitesse moteur gauche |

### Capteurs de flamme

| Capteur | Broche |
|---|---|
| Gauche | A0 |
| Centre | A1 |
| Droite | A2 |

### Extinction

| Élément | Broche |
|---|---|
| Servomoteur (signal) | 5 |
| Relais (pompe) | 4 |

> ⚠️ Relier **toutes les masses (GND)** ensemble : Arduino, L298N, alimentation, relais, capteurs.

## Logique de décision

```
Lecture des 3 capteurs
        │
        ├── tous > 850 ───────────────► AUCUN FEU : arrêt, pompe OFF
        │
        ├── au moins un ≤ 600 ────────► FEU PROCHE : arrêt + arrosage
        │
        └── sinon (entre 600 et 850) ─► FEU LOINTAIN : on s'oriente
                 ├── gauche la plus basse ─► pivot à gauche (200 ms)
                 ├── droite la plus basse ─► pivot à droite (200 ms)
                 └── centre le plus bas ───► avance (200 ms)
```

Le robot avance **par impulsions** (mouvement 200 ms, pause 150 ms) pour relire les capteurs régulièrement.

## Paramètres réglables

| Paramètre | Valeur | Effet |
|---|---|---|
| `FIRE_DETECT` | 850 | En dessous : une flamme est détectée |
| `FIRE_STOP` | 600 | En dessous : flamme assez proche, on arrose |
| `speedLeft` / `speedRight` | 140 (0–255) | Vitesse des moteurs |
| `moveTime` | 200 ms | Durée de chaque impulsion de mouvement |
| `servoLeft/Center/Right` | 50° / 90° / 130° | Positions du balayage |
| `RELAY_ON` / `RELAY_OFF` | HIGH / LOW | À inverser si le relais est actif à LOW |

### Calibration des capteurs

Valeurs mesurées avec ce montage (moniteur série, 9600 bauds) :

| Situation | Valeur approximative |
|---|---|
| Pas de flamme | ~915 |
| Flamme lointaine | ~800 |
| Flamme proche | ~300–400 |

Les valeurs dépendent de la lumière ambiante, du capteur et de la flamme : **recalibrez** `FIRE_DETECT` et `FIRE_STOP` dans votre environnement.

## Installation

1. Installer l'[Arduino IDE](https://www.arduino.cc/en/software).
2. La bibliothèque `Servo` est incluse par défaut.
3. Câbler selon les tableaux ci-dessus.
4. Ouvrir le fichier `.ino`, choisir la carte (Arduino Uno) et le port, puis téléverser.
5. Ouvrir le moniteur série (9600 bauds) pour voir les valeurs `Left / Center / Right` et les messages d'état.

## Séquence d'arrosage

1. Arrêt des moteurs.
2. Pompe ON.
3. 3 allers-retours du servo (gauche ↔ droite).
4. Retour au centre.
5. Pompe OFF.

## Limites connues et améliorations possibles

- **Égalité de valeurs** : si deux capteurs lisent exactement la même valeur la plus basse, aucune direction n'est choisie et le robot s'arrête une itération.
- **Bruit des capteurs** : une seule lecture par cycle ; faire une moyenne de plusieurs lectures stabiliserait le comportement.
- **Pas de recherche active** : sans flamme, le robot reste immobile (pas de balayage de la zone).
- **Messages série répétés** (`PUMP OFF` à chaque cycle sans feu) : à limiter si on veut un moniteur plus lisible.
- **Pas de limite de temps d'arrosage** si la flamme persiste (risque de vider le réservoir) : ajouter un compteur.
- **Pas d'évitement d'obstacles**.

## Sécurité

Projet pédagogique. Tester avec une petite flamme (bougie) dans un espace dégagé, loin de tout matériau inflammable, avec un extincteur à proximité. Ne jamais laisser le robot sans surveillance.

