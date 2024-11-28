# figherfighting_robot
Ce projet est un robot pompier basé sur Arduino. Il utilise des capteurs de flamme pour détecter le feu, se déplace vers celui-ci et l'éteint en pulvérisant de l'eau. Le robot est équipé de moteurs pour le déplacement, d'une pompe à eau pour éteindre le feu et d'un servo-moteur pour orienter la pulvérisation d'eau.  

## Composants  

- Arduino Uno (ou carte compatible)  
- 3 capteurs de flamme  
- 2 moteurs DC avec un driver de moteur  
- Pompe à eau  
- Servo-moteur  
- Fils de connexion et alimentation électrique  

## Configuration des broches  

| Composant               | Broche Arduino |  
| ----------------------- | --------------- |  
| Capteur de flamme gauche | 9               |  
| Capteur de flamme droit  | 10              |  
| Capteur de flamme avant  | 8               |  
| Moteur gauche 1          | 2               |  
| Moteur gauche 2          | 3               |  
| Moteur droit 1           | 4               |  
| Moteur droit 2           | 5               |  
| Pompe à eau              | 6               |  
| Servo-moteur             | 7               |  

## Vue d'ensemble du code  

Le code est divisé en deux sections principales : `setup()` et `loop()`.  

### Setup  

- Initialisation des broches : Configure les broches des capteurs comme entrées, les broches du driver de moteur comme sorties, et la broche de la pompe à eau comme sortie.  
- Servo-moteur : Connecte le servo-moteur à la broche spécifiée et définit sa position initiale.  

### Loop  

- Détection du feu : Vérifie en continu les capteurs de flamme pour détecter un feu.  
- Déplacement :  
  - Si un feu est détecté par le capteur gauche, le robot tourne à gauche.  
  - Si détecté par le capteur droit, il tourne à droite.  
  - Si détecté par le capteur avant, il avance.  
  - Si aucun feu n'est détecté, le robot s'arrête.  
- Activation de la pompe à eau :  
  - Lorsque le robot est proche du feu, il active la pompe à eau et pulvérise de l'eau.  
  - Le servo-moteur est utilisé pour diriger la pulvérisation.  
  - Lorsque le feu est éteint ou non détecté, la pompe à eau est désactivée et le servo-moteur revient à sa position initiale.  

## Utilisation  

1. Configuration matérielle :  
   - Connectez les capteurs de flamme, les moteurs, la pompe à eau et le servo-moteur à l'Arduino selon le tableau de configuration des broches.  
   - Assurez-vous que tous les composants sont correctement alimentés.  

2. Téléchargement du code :  
   - Ouvrez l’IDE Arduino.  
   - Copiez le code fourni dans un nouveau sketch.  
   - Téléchargez le sketch sur votre carte Arduino.  

3. Lancer le robot :  
   - Placez le robot dans un environnement où il peut détecter et se déplacer vers un feu.  
   - Assurez-vous qu'une source de feu sécurisée et contrôlée est utilisée pour les tests.  
   - Le robot détectera automatiquement le feu, se déplacera vers celui-ci et tentera de l'éteindre.  
