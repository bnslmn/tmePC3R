package main

import (
	"bufio"
	"fmt"
	"log"
	"math/rand"
	"net"
	"os"
	"strconv"
	"strings"

	st "../client/structures"
	tr "./travaux"
)

var ADRESSE = "localhost"

var pers_vide = st.Personne{Nom: "", Prenom: "", Age: 0, Sexe: "M"}
var mapPersonne = make(map[int]*personne_serv)

type Request struct {
	personeID    int
	method       string
	responseChan chan string
}

// type d'un paquet de personne stocke sur le serveur, n'implemente pas forcement personne_int (qui n'existe pas ici)
type personne_serv struct {
	id       int
	personne st.Personne
	afaire   []func(st.Personne) st.Personne
	statut   string
}

// cree une nouvelle personne_serv, est appelé depuis le client, par le proxy, au moment ou un producteur distant
// produit une personne_dist
func creer(id int) *personne_serv {
	p := personne_serv{id: id, personne: pers_vide, afaire: nil, statut: "V"}
	return &p
}

// Méthodes sur les personne_serv, on peut recopier des méthodes des personne_emp du client
// l'initialisation peut être fait de maniere plus simple que sur le client
// (par exemple en initialisant toujours à la meme personne plutôt qu'en lisant un fichier)
func (p *personne_serv) initialise() {
	//oui, j'ai vérifié l'age de la saga James Bond...
	p.personne = st.Personne{Nom: "Bond", Prenom: "James", Age: 68, Sexe: "M"}
	p.statut = "R"
	len := rand.Intn(4)
	for i := 0; i <= len; i++ {
		p.afaire = append(p.afaire, tr.UnTravail())
	}

}

func (p *personne_serv) travaille() {
	if len(p.afaire) == 0 {
		p.statut = "C"

	} else {
		fun := p.afaire[0]
		println(fun)
		p.afaire = p.afaire[1:]
		p.personne = fun(p.personne)
	}
}

func (p *personne_serv) vers_string() string {
	age := strconv.Itoa(p.personne.Age)
	return p.personne.Nom + " " + p.personne.Prenom + " " + age + " " + p.personne.Sexe
}

func (p *personne_serv) donne_statut() string {
	return p.statut
}

// Goroutine qui maintient une table d'association entre identifiant et personne_serv
// il est contacté par les goroutine de gestion avec un nom de methode et un identifiant
// et il appelle la méthode correspondante de la personne_serv correspondante
func mainteneur(requestChan chan Request) {
	for {
		req := <-requestChan
		switch req.method {

		case "initialise":
			persone, ok := mapPersonne[req.personeID]
			if ok {
				persone.initialise()
				req.responseChan <- "Initialisation avec succès\n"
			}
		case "creer":
			p := creer(req.personeID)
			mapPersonne[req.personeID] = p
			req.responseChan <- "Création avec succès\n"
		case "donne_statut":
			persone, ok := mapPersonne[req.personeID]
			if ok {
				req.responseChan <- persone.donne_statut() + "\n"
			}
		case "travaille":
			persone, ok := mapPersonne[req.personeID]
			if ok {
				persone.travaille()
				req.responseChan <- "La personne travaille\n"
			}
		case "vers_string":
			persone, ok := mapPersonne[req.personeID]
			if ok {
				req.responseChan <- persone.vers_string() + "\n"
			}
		}

	}
}

// Goroutine de gestion des connections
// elle attend sur la socketi un message content un nom de methode et un identifiant et appelle le mainteneur avec ces arguments
// elle recupere le resultat du mainteneur et l'envoie sur la socket, puis ferme la socket
func gere_connection(conn net.Conn, requestChan chan Request) {

	reponsechan := make(chan string)
	command, err := bufio.NewReader(conn).ReadString('\n')
	if err != nil {
		log.Fatal(err)
	}
	splitCommand := strings.Split(command, ",")
	id, _ := strconv.Atoi(splitCommand[0])
	request := Request{personeID: id, method: splitCommand[1][:len(splitCommand[1])-1], responseChan: reponsechan}
	requestChan <- request
	response := <-request.responseChan
	fmt.Fprintf(conn, response)
}

func main() {

	if len(os.Args) < 2 {
		fmt.Println("Format: client <port>")
		return
	}
	port, _ := strconv.Atoi(os.Args[1]) // doit être le meme port que le client
	addr := ADRESSE + ":" + fmt.Sprint(port)
	// A FAIRE: creer les canaux necessaires, lancer un mainteneur
	reqChan := make(chan Request)
	go mainteneur(reqChan)
	ln, _ := net.Listen("tcp", addr) // ecoute sur l'internet electronique
	fmt.Println("Ecoute sur", addr)
	for {
		conn, _ := ln.Accept() // recoit une connection, cree une socket
		fmt.Println("Accepte une connection.")
		go gere_connection(conn, reqChan) // passe la connection a une routine de gestion des connections
	}
}
