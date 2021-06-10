*** Exo 1 ***

let compteur = ref 0

let rec entree nb =
    if nb > 0 then

        begin
            compteur := compteur + 1
            print_int nb;
            print_endline "=> Entree"
            Thread.delay (Random.float 0.2)
            entree (nb -1)
        end
    else print_endline "Fin entree"

let rec sortie1 () =
    if !compteur == 0 then
        ()
    else
        begin
            compteur := !compteur -1;
            print_int !compteur;
            print_endline "=> Sortie";
            Thread.delay (Random.float O.4);
            sortie1 ()
        end

let rec sortie2 () =
    if !compteur == 0 then
        ()
    else
        begin
            compteur := !compteur -1;
            print_int !compteur;
            print_endline "=> Sortie";
            Thread.delay (Random.float O.4);
        end
    sortie2 ()

let main() =
    let nb_vis = int_of_string Sys.argv.(1) in
    let ts1 = Thread.create sortie () in
    let ts2 = Thread.create sortie () in
    let te1 = Thread.create entree () in
    let te2 = Thread.create entree () in
    Thread.join te1;
    Thread.join te2;
    while !compteur > 0 do () done;
    Thread.kill ts1;
    Thread.kill ts2;
    print_string "Fin"
    exit 0;;

let _ = main();;


*** Problème 1 ***

Course:
    Les quatres threads lisent et écrivent en même temps la variable compteur

Solution:
    Verrou => Un mutex pour protéger compteur
    Les manipulations de compteur auront lieu en section critique
    (Au plus un thread pourra accéder à la variable compteur)

*** Problème 2 ***
    Les threads sortie et le thread principal peuvent boucler sans rien produire si le compteur est à 0

Solution:
    Conditon (primitive attente et de réveil  - wait / signal)


*** Problème 3 ***
C'est bon (on ne fait pas de terminaison prématurée)



let compteur = ref 0
let compteur_lock = Mutex.create ()
let compteur_cond = Condition.create ()
let fin_entrees = ref false

let rec entree nb = 
    if nb > 0 then
        beign
            Mutex.lock(compteur_lock);
            compteur := !compteur + 1
            print_int nb;
            print_endline "=> Entree"
            Mutex.unlock(compteur)
            Thread.delay (Ranfom.float 0.2)
            entree (nb - 1)
        end
    else print_endline "Fin entree"

let sortie2 () =
    while not !fin_entree  do
        Mutex.lock compteur_lock;
        while !compteur == 0 do
            Conditon.wait compteur_cond compteur_lock
        done;
        compteur := !compteur -1;
        print_int !compteur;
        print_endline "=> Sortie";
        Mutex.unlock compteur_lock;
        Thread.delay (Random.float 0.4)
    done;
    print_endline "Fin des sorties"

let main ();;

*** Exo 2.1 ***

void run_p (void *phrase){
    while(1){
        printf("%s \n", (char *) phrase);
        ft_thread_cooperate();
    }
}

int main(void){
    ft_scheduler sched = ft_scheduler_create ();
    ft_thread_create(sched, run_p, NULL, (void * "Belle 
    Marquise");
    ft_thread_create(sched, run_p, NULL, (void * "vos beaux
     yeux");
    ft_thread_create(sched, run_p, NULL, (void * "me font mourir");
    ft_thread_create(sched, run_p, NULL, (void * "d'amour");

    ft_scheduler_start (sched);
}
    
