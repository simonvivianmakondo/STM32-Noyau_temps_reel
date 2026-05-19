# MI11 — Noyau temps réel bare-metal (STM32H7)

> Implémentation d'un noyau temps réel minimal sur microcontrôleur **STM32H743** (Cortex-M7),
> réalisée dans le cadre de l'UE MI11 à l'**UTC Compiègne**.  
> Ordonnancement coopératif avec primitives de synchronisation, sans HAL ni RTOS commercial.

**Auteur :** Simon Vivian Makondo  
**Formation :** Cycle Ingénieur — UTC Compiègne  
**Année :** 2025–2026

---

## Contexte et enjeux

La plupart des systèmes embarqués temps réel s'appuient sur des RTOS commerciaux (FreeRTOS, Zephyr, ThreadX…) qui offrent des abstractions complètes mais masquent les mécanismes fondamentaux. Ce TP adopte l'approche inverse : **construire un noyau temps réel from scratch**, en programmant directement les registres du Cortex-M7 et en implémentant manuellement l'ordonnanceur et les primitives de synchronisation.

Cette démarche n'est pas académique. Dans des domaines critiques — avionique, spatial, médical — la compréhension du comportement exact du scheduler et des mécanismes de commutation de contexte est indispensable pour garantir le déterminisme, prouver les temps de réponse pire cas, et certifier le système selon les normes DO-178C ou IEC 61508.

L'objectif pédagogique est triple : maîtriser les mécanismes bas niveau de l'ordonnancement temps réel, comprendre les problématiques de synchronisation et de sections critiques, et développer la rigueur nécessaire au développement de systèmes critiques.

---

## Environnement de développement

| Composant | Détail |
|---|---|
| Cible | STM32H743ZI (Nucleo-H743ZI2) |
| Architecture | ARM Cortex-M7, 480 MHz |
| Compilateur | `arm-none-eabi-gcc` |
| Build system | Eclipse CDT avec configurations multiples |
| Débogage | GDB + OpenOCD via ST-Link V3 |
| Émulation | QEMU ARM + Renode — tests sans cible physique |

---

## Architecture du noyau

### Ordonnanceur de tâches (FIFO)

Le scheduler implémente une politique d'ordonnancement **round-robin** avec file d'attente FIFO. Toutes les tâches ont initialement la même priorité. La gestion de leur activité repose sur une structure de file circulaire mémorisant les numéros de tâches dans un tableau de dimension `MAX_TACHE`.

**Primitives principales :**
- `creer_tache()` — Création et initialisation du contexte d'une tâche
- `active()` — Activation d'une tâche dormante et insertion dans la file d'ordonnancement
- `dort()` — Mise en sommeil volontaire de la tâche courante
- `fin_tache()` — Terminaison propre d'une tâche avec retrait de la file

### Commutation de contexte

La commutation entre tâches s'effectue via la sauvegarde et restauration explicite du contexte processeur (registres R4-R11, pointeur de pile). Le basculement est déclenché par un appel au scheduler qui détermine la prochaine tâche éligible selon la politique FIFO.

### Synchronisation

**Sémaphores binaires et compteurs** — Implémentation des primitives `P()` et `V()` avec gestion de la file d'attente des tâches bloquées. Les sections critiques sont protégées par désactivation temporaire des interruptions via les registres `PRIMASK` du Cortex-M7.

**Gestion des sections critiques** — Les macros `_lock_()` et `_unlock_()` préservent l'état des interruptions pour permettre l'imbrication de sections critiques sans corruption de l'état système.

---

## Structure du projet

```
hwsupport/       # Drivers bas niveau (GPIO, UART, RCC, startup code)
kernel/          # Cœur du noyau (ordonnanceur, sémaphores, delays, chronogrammes)
io/              # Couche I/O série et terminal
noyau_test*.c    # Suites de tests unitaires et validation incrémentale
```

### Configurations de build

- **Debug_Hardware** — Déploiement et debug sur STM32H743 physique
- **Debug_Emu** — Émulation sous QEMU/Renode avec support GDB
- **Release_Hardware** / **Release_Emu** — Builds optimisés pour benchmark

---

## Tests et validation

Le développement a suivi une approche **test-driven incrémentale** avec plusieurs versions du programme de test :

- `noyau_test_V1.c` — Validation de base : création, activation, fin de tâche
- `noyau_test_V2.c` — Ordonnancement FIFO avec plusieurs tâches
- `noyau_test_V3.c` — Primitives de synchronisation (sémaphores)
- `noyau_test_V4.c` — Tests de charge et scénarios de blocage
- `noyau_test_prio.c` — Extension avec gestion de priorités

Chaque version constitue un point de validation avant d'ajouter la couche de complexité suivante. Les tests unitaires arrêtent l'exécution dès qu'une assertion échoue, permettant une localisation rapide des régressions.

---

## Émulation et portabilité

L'émulation sous **QEMU** et **Renode** a permis de valider la logique du noyau indépendamment du matériel. Les scripts de configuration (`renode_debug.resc`, `run_qemu.bat`) instancient une machine virtuelle ARM Cortex-M avec périphériques UART et GPIO simulés.

Cette approche *émulation → validation matérielle* a considérablement réduit les cycles de debug et permis de distinguer clairement les bugs logiques des problèmes de configuration hardware (horloges, GPIO, NVIC).

---

## Ce que ce TP nous a appris

**Sur le plan technique**, ce TP a imposé de maîtriser les mécanismes de commutation de contexte au niveau assembleur, de comprendre précisément le rôle du pointeur de pile et de la sauvegarde des registres, et de gérer manuellement les états de tâches (NCREE, PRET, EXEC, BLOQUE). L'implémentation des sémaphores a rendu concrets les concepts de synchronisation vus en cours théorique, avec les problématiques réelles d'interblocage et de famine.

**Sur le plan méthodologique**, le développement incrémental avec validation à chaque étape (V1 → V2 → V3 → V4) a montré l'importance d'une stratégie de test rigoureuse pour des systèmes bas niveau. La nécessité de documenter chaque choix d'implémentation (pourquoi FIFO, pourquoi `_lock_()` plutôt que `_irq_disable_()`) installe une discipline essentielle pour les systèmes critiques.

**Sur le plan industriel**, ce TP donne une compréhension intime de ce que font réellement FreeRTOS ou Zephyr sous le capot, et des compromis de conception inhérents à tout RTOS (latence vs équité, simplicité vs fonctionnalités). Cette connaissance est directement transférable aux contextes aéronautiques et spatiaux où la preuve formelle et la maîtrise du déterminisme sont des exigences réglementaires.

---

## Références

- [STM32H743 Reference Manual (RM0433)](https://www.st.com/resource/en/reference_manual/rm0433-stm32h742-stm32h743753-and-stm32h750-value-line-advanced-armbased-32bit-mcus-stmicroelectronics.pdf)
- [ARMv7-M Architecture Reference Manual](https://developer.arm.com/documentation/ddi0403/latest/)
- [Cortex-M7 Technical Reference Manual](https://developer.arm.com/documentation/ddi0489/latest/)
- [QEMU ARM System Emulator](https://www.qemu.org/docs/master/system/target-arm.html)
- [Renode documentation](https://renode.readthedocs.io/)
