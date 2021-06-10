package main

import (
	"bufio"
	"fmt"
	"log"
	"math/rand"
	"net"
	"os"
	"regexp"
	"strconv"
	"strings"
	"time"

	st "./structures" // contient la structure Personne
	tr "./travaux"    // contient les fonctions de travail sur les Personnes
)

var ADRESSE string = "localhost"                           // adresse de base pour la Partie 2
var FICHIER_SOURCE string = "./conseillers-municipaux.txt" // fichier dans lequel piocher des personnes
var TAILLE_SOURCE int = 450000                             // inferieure au nombre de lignes du fichier, pour prendre une ligne au hasard
const TAILLE_G = 5                                         // taille du tampon des gestionnaires
var NB_G int = 2                                           // nombre de gestionnaires
var NB_P int = 2                                           // nombre de producteurs
var NB_O int = 4                                           // nombre d'ouvriers
var NB_PD int = 2                                          // nombre de producteurs distants pour la Partie 2
var PORT int
var PROXY_CHANNEL = make(chan Request)
var pers_vide = st.Personne{Nom: "", Prenom: "", Age: 0, Sexe: "M"} // une personne vide
var COUNTER = 1

// paquet de personne, sur lequel on peut travailler, implemente l'interface personne_int
type personne_emp struct {
	personne st.Personne
	ligne    int
	afaire   []func(st.Personne) st.Personne
	statut   string
}

// paquet de personne distante, pour la Partie 2, implemente l'interface personne_int
type personne_dist struct {
	id int
}

// interface des personnes manipulees par les ouvriers, les
type personne_int interface {
	initialise()          // appelle sur une personne vide de statut V, remplit les champs de la personne et passe son statut à R
	travaille()           // appelle sur une personne de statut R, travaille une fois sur la personne et passe son statut à C s'il n'y a plus de travail a faire
	vers_string() string  // convertit la personne en string
	donne_statut() string // renvoie V, R ou C
}

type Request struct {
	command      string
	canalReponse chan string
}

// fabrique une personne à partir d'une ligne du fichier des conseillers municipaux
// à changer si un autre fichier est utilisé
func personne_de_ligne(l string) st.Personne {
	separateur := regexp.MustCompile("\u0009") // oui, les donnees sont separees par des tabulations ... merci la Republique Francaise
	separation := separateur.Split(l, -1)
	naiss, _ := time.Parse("2/1/2006", separation[7])
	a1, _, _ := time.Now().Date()
	a2, _, _ := naiss.Date()
	agec := a1 - a2
	return st.Personne{Nom: separation[4], Prenom: separation[5], Sexe: separation[6], Age: agec}
}

// *** METHODES DE L'INTERFACE personne_int POUR LES PAQUETS DE PERSONNES ***

func lectrice(line int, returnChannel chan string) {
	f, err := os.Open(FICHIER_SOURCE)
	if err != nil {
		log.Fatal(err)
	}
	defer f.Close()

	sc := bufio.NewScanner(f)
	sc.Text() //éviter l'entête
	sc.Text()
	cp := 2
	for sc.Scan() {
		if cp == line {
			returnChannel <- sc.Text()
		}
		cp++
	}
	if err := sc.Err(); err != nil {
		log.Fatal(err)
	}
}

func (p *personne_emp) initialise() {
	reponseChan := make(chan string)
	go lectrice(p.ligne, reponseChan)

	personLine := <-reponseChan
	p.personne = personne_de_ligne(personLine)
	funcs := rand.Intn(4) + 1
	for i := 0; i < funcs; i++ {
		p.afaire = append(p.afaire, tr.UnTravail())
	}
	p.statut = "R"
}

func (p *personne_emp) travaille() {

	if len(p.afaire) == 0 {
		p.statut = "C"

	} else {
		fun := p.afaire[0]
		p.afaire = p.afaire[1:]
		fun(p.personne)
	}
}

func (p *personne_emp) vers_string() string {
	age := strconv.Itoa(p.personne.Age) //conversion int -> string
	return p.personne.Nom + " " + p.personne.Prenom + " " + age + " " + p.personne.Sexe

}

func (p *personne_emp) donne_statut() string {
	return p.statut
}

// *** METHODES DE L'INTERFACE personne_int POUR LES PAQUETS DE PERSONNES DISTANTES (PARTIE 2) ***
// ces méthodes doivent appeler le proxy (aucun calcul direct)

func (p personne_dist) initialise() {
	idPersonne := strconv.Itoa(p.id)
	commande := idPersonne + ",initialise\n"
	result := make(chan string)
	request := Request{command: commande, canalReponse: result}
	PROXY_CHANNEL <- request
}

func (p personne_dist) travaille() {
	id := strconv.Itoa(p.id)
	commande := id + ",travaille\n"
	result := make(chan string)
	request := Request{command: commande, canalReponse: result}
	PROXY_CHANNEL <- request
}

func (p personne_dist) vers_string() string {
	id := strconv.Itoa(p.id)
	commande := id + ",vers_string\n"
	result := make(chan string)
	request := Request{command: commande, canalReponse: result}
	PROXY_CHANNEL <- request
	response := <-request.canalReponse
	return response

}

func (p personne_dist) donne_statut() string {
	id := strconv.Itoa(p.id)
	commande := id + ",donne_statut\n"
	result := make(chan string)
	request := Request{command: commande, canalReponse: result}
	PROXY_CHANNEL <- request
	response := <-request.canalReponse
	return response
}

// *** CODE DES GOROUTINES DU SYSTEME ***

// Partie 2: contacté par les méthodes de personne_dist, le proxy appelle la méthode à travers le réseau et récupère le résultat
// il doit utiliser une connection TCP sur le port donné en ligne de commande
func proxy(reqChan chan Request, port int) {
	serverAddress := ADRESSE + ":" + strconv.Itoa(port)
	for {
		request := <-reqChan
		conn, err := net.Dial("tcp", serverAddress)
		if err != nil {
			log.Fatal(err)
		}
		commande := request.command
		fmt.Fprintf(conn, commande)
		reponse, _ := bufio.NewReader(conn).ReadString('\n')
		conn.Close()
		request.canalReponse <- reponse
	}
}

// Partie 1 : contacté par la méthode initialise() de personne_emp, récupère une ligne donnée dans le fichier source
func lecteur() {
	// A FAIRE
}

// Partie 1: récupèrent des personne_int depuis les gestionnaires, font une opération dépendant de donne_statut()
// Si le statut est V, ils initialise le paquet de personne puis le repasse aux gestionnaires
// Si le statut est R, ils travaille une fois sur le paquet puis le repasse aux gestionnaires
// Si le statut est C, ils passent le paquet au collecteur
func ouvrier(prodChan chan personne_int, consChan chan personne_int, collectChan chan personne_int) {
	// A FAIRE
	println("WORKER")

	for {
		p := <-consChan
		statut := p.donne_statut()
		if strings.Contains(statut, "\n") {
			statut = strings.Replace(statut, "\n", "", 1)
		}
		if statut == "V" {
			p.initialise()
			prodChan <- p

		} else if statut == "R" {
			p.travaille()
			prodChan <- p
		} else {
			collectChan <- p
		}

	}

}

// Partie 1: les producteurs cree des personne_int implementees par des personne_emp initialement vides,
// de statut V mais contenant un numéro de ligne (pour etre initialisee depuis le fichier texte)
// la personne est passée aux gestionnaires
func producteur(productionChannel chan personne_int) {

	for {
		p := personne_emp{personne: pers_vide, ligne: rand.Intn(2000) + 2, statut: "V"}
		productionChannel <- &p
		time.Sleep(2 * time.Second)
	}

}

// Partie 2: les producteurs distants cree des personne_int implementees par des personne_dist qui contiennent un identifiant unique
// utilisé pour retrouver l'object sur le serveur
// la creation sur le client d'une personne_dist doit declencher la creation sur le serveur d'une "vraie" personne, initialement vide, de statut V
func producteur_distant(producteurChan chan personne_int, proxyChannel chan Request) {

	for {
		p := personne_dist{id: COUNTER}
		id := strconv.Itoa(COUNTER)
		COUNTER++
		commande := id + ",creer\n"
		result := make(chan string)
		request := Request{command: commande, canalReponse: result}
		proxyChannel <- request
		producteurChan <- p
		time.Sleep(1 * time.Second)
	}

}

// Partie 1: les gestionnaires recoivent des personne_int des producteurs et des ouvriers et maintiennent chacun une file de personne_int
// ils les passent aux ouvriers quand ils sont disponibles
// ATTENTION: la famine des ouvriers doit être évitée: si les producteurs inondent les gestionnaires de paquets, les ouvrier ne pourront
// plus rendre les paquets surlesquels ils travaillent pour en prendre des autres
func gestionnaire(producteurChan chan personne_int, consumerChannel chan personne_int) {
	var file []personne_int
	taille := 0
	for {

		select {
		case p := <-producteurChan:

			//limite personne atteinte
			if taille >= TAILLE_G {
				for taille > 0 {
					taille--
					headPersonne := file[0]
					file = file[1:]
					consumerChannel <- headPersonne
				}

			}
			//ajout possible
			file = append(file, p)
			taille++

		default:
			//envoi de la personne au travailleur
			if taille > 0 {
				p := file[0]
				file = file[1:]
				consumerChannel <- p
				taille--
			}

		}

	}
}

// Partie 1: le collecteur recoit des personne_int dont le statut est c, il les collecte dans un journal
// quand il recoit un signal de fin du temps, il imprime son journal.
func collecteur(fin chan int, collecteurChan chan personne_int) {
	log := ""
	for {

		select {
		case p := <-collecteurChan:
			println("*** Collecteur arrivé ***")
			log = log + p.vers_string() + "\n"

		case <-fin:
			println("*** Arrêt du collecteur ***")
			println(log)
			fin <- 0
			return
		}
	}

}

func main() {
	rand.Seed(time.Now().UTC().UnixNano()) // graine pour l'aleatoire
	if len(os.Args) < 3 {
		fmt.Println("Format: client <port> <millisecondes d'attente>")
		return
	}
	PORT, _ := strconv.Atoi(os.Args[1])   // utile pour la partie 2
	millis, _ := strconv.Atoi(os.Args[2]) // duree du timeout
	fintemps := make(chan int)
	producersChan := make(chan personne_int)
	consumersChan := make(chan personne_int)
	collecteurChan := make(chan personne_int)
	go gestionnaire(producersChan, consumersChan)
	go proxy(PROXY_CHANNEL, PORT)
	go producteur_distant(producersChan, PROXY_CHANNEL)
	go ouvrier(producersChan, consumersChan, collecteurChan)
	go collecteur(fintemps, collecteurChan)

	time.Sleep(time.Duration(millis) * time.Second)
	fintemps <- 0
	<-fintemps
}
