# PARTIE 1
## Ordinnanceur de tâches

- Toutes les tâches ont la même 
priorité.
- La gestion de leur activité se fait en FIFO, c'est-à-dire que la plus ancienne dans la file 
sera la première activée.
- La solution choisie dans ce noyau minimum consiste à mémoriser les numéros des tâches dans 
un tableau _file de dimension **MAX_TACHE**.
-  L’indice d’un élément du tableau correspond à un 
numéro de tâche et l’élément du tableau à la tâche suivante. 
- La variable **_queue** contient l'indice 
de la tâche active, de sorte qu'on puisse connaître la prochaine tâche à activer. 
- Dès qu’une tâche devient active, elle est automatiquement remise en fin de file, puisqu’elle sera 
la dernière à être réactivée. 

### Q 1.a
- Les tests constituent l'ensemble des opérations menées pour s'assurer que les fonctions implementées ont le comportement voulu. La spécificité des test unitaires, est que, le programme test s'arrête, si un test unitaire est non validé

### Q 1.b
Parmi les tests unitaires, on peut imaginer :

- Essayer de retirer une tâche quand la file est vide
- Essayer de retirer une tâche qui n'existe pas
- Essayer de retirer quand il y a seulement une tâche dans la file
- Essayer de retirer la tête de la file
- Essayer de retirer la queue de la file
- Essayer de retirer une tâche au milieu de la file
- Essayer de retirer deux fois la même tâche
- Essayer de retirer une tâche avec un indice invalide
- Essayer d'ajouter une tâche quand la file est vide
- Essayer d'ajouter une tâche quand la file est pleine
- Essayer d'ajouter une tâche quand il reste une seule place
- Essayer d'ajouter une tâche qui existe déjà
- Essayer d'ajouter une tâche avec un indice invalide
- Essayer d'ajouter une tâche après l'avoir retirée
- Essayer de trouver le suivant dans une file vide
- Essayer de trouver le suivant, s'il y a une seule tâche
- Essayer de trouver le suivant avec deux tâches
- Essayer de trouver le suivant après un cycle complet
- Essayer de vider complètement la file puis de la remplir à nouveau

### Q 1.c
Décrivons en langage naturel les fonctions suivantes :

- *ajouter(n)* :
    
    La fonction ajouter, va baser son principe, sur la position actuelle de la queue. Vu que la queue contient l'indice de la dernière tâche ajoutée, alors, pour ajouter une tâche, dans la file, on cherchera juste la correspondance de la queue et, affecter à cette correspondance, la valeur de la nouvelle tâche, bien sûr en ayant vérifié au préalable que la tâche n'existe pas, que la file n'est pas pleine et on affectera ensuite à la queue, la nouvelle valeur ajoutée. Si la file est vide, on initialise juste la queue

- *suivant()* :

    Pour trouver le suivant d'une tâche, il s'agit tout s'implement, de s'assurer, que la la correspondance en file de la valeur de _queue est acceptable, et si c'est le cas, on retourne cette valeur et on affecte à queue, la valeur retournée

- *retire(n)* :

    La fonction retire, si la file est non vide, et la valeur de la tâche est présente dans la file, si, la tâche est seule, la queue passe à une valeur non acceptable, s'il y en a deux, la queue se resume à la seule tâche restante, sinon, on fera un branchement, entre le precedent de la tache courante et son suivant


### Q 1.d

### Q 1.e
_queue doit être initialisé à MAX_TACHES

# PARTIE 2
`_irq_disable_()` et `_irq_enable_()` activent ou désactivent directement les interruptions sans tenir compte de l’état précédent. C’est simple mais dangereux dans un code imbriqué, car un `enable` peut réactiver les IRQ alors qu’elles avaient déjà été désactivées avant.
À l’inverse, `_lock_()` et `_unlock_()` sauvegardent l’état de `PRIMASK` avant de modifier les interruptions puis le restaurent ensuite. Cela permet une gestion sûre des sections critiques, surtout dans un OS, un scheduler ou des mécanismes de synchronisation comme les mutex.

## Q2.1
Donc le choix se porte vers la macro `_irq_disable_()` pour desactiver les interruptions

## Q2.2
Quand on sort du noyau, on doit faire `_irq_enable_()` pour activer les interruptions.

Et, lancer une boucle infinie, pour eviter un comportement non deterministe

Les fonctions irq_enable() et irq_disable() fonctionnent de manière directe : elles activent ou désactivent les interruptions sans se soucier de l’état précédent du système. Elles travaillent donc en mode “force ON/OFF”. À l’inverse, lock() et unlock() sauvegardent d’abord l’état actuel des interruptions avant de les modifier, puis restaurent exactement cet état à la fin.

Utiliser irq, modifie les interruptions de manière absolue, alors, on peut envisager utiliser lock.

## Q2.3
`_lock_()`

## Q2.4
Le statut de la tâche qu'on tue est EXEC

Si on retire une tâche ordonnançable, on la retire de l'état PRET

Quand fin de tâche retourne au code de la fonction qui l'a appelé :
tache_A()
{
    fin_tache();
}

Déjà, fin tache est appelé dans le contexte d'exection d'une tâche, donc une bonne pratique serait de faire qu'apres fin tâche, qu'il n'y ai plus executiond e la fonction appelante; soit boucle infinie

## Q2.5
Le statut doit etre initialisé à NCREE lors du démarrage du noyau

## Q2.6
Le role de la variable _tache_c est pour connaitre la tache en cours d'execution, ou bien la tache courante. On va initialiser cette valeur à -1

## Q2.7
L'élément qui contient le haut de la pile est sp

## Q2.8
Car, il faut pas des que des interuptions se pointent lors de l'activation et creation des taches

## Q2.13
Une variable static est une variable qui persiste dans toutes les instances recursives ou appelantes d'une fonction