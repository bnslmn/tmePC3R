/**
*	Producer-Consumer Model using Goroutines
*
*	Amine Benslimane
*	Master 1 STL,
*	Sorbonne-Université,
*	May 2021
*
*	The folder "stop_times.txt" must be at root ! get it on : https://frama.link/pc3r-sncf
*
 */

package main

import (
	"bufio"
	"fmt"
	"os"
	"strconv"
	"strings"
	"time"
)

type paquet struct {
	arrivee string
	depart  string
	arret   int
}

const WORKER int = 3

//parsing file
func reader(file chan string) {
	f, err := os.Open("./stop_times.txt")
	if err != nil {
		panic(err)
	}

	defer f.Close()

	scanner := bufio.NewScanner(f)
	for scanner.Scan() {
		file <- scanner.Text()

	}

}

func worker(client chan string, server chan chan paquet, reducer chan paquet) {
	for {
		line := <-client
		var lines []string = strings.Split(line, ",")
		linePacket := make(chan paquet)
		server <- linePacket
		var packet paquet
		packet.arret = 0
		packet.arrivee = lines[1]
		packet.depart = lines[2]

		linePacket <- packet
		result := <-linePacket
		reducer <- result
	}

}

func compute(cp chan paquet) {
	packet := <-cp
	arr, _ := time.Parse("15:04:05", packet.arrivee)
	deb, _ := time.Parse("15:04:05", packet.depart)
	packet.arret = int(deb.Sub(arr).Seconds())
	cp <- packet
}

func serverCompute(server chan chan paquet) {
	for {
		s := <-server
		go func() { compute(s) }()
	}
}

func reducer(transPack chan paquet, fin chan int) {
	var accumulator int
	var cp int
	for {
		select {
		case <-fin: //signal de fin du temps
			fin <- accumulator / cp //la moyenne actuelle
			return
		case p := <-transPack:
			accumulator = accumulator + p.arret //accumule les arrêts
			cp = cp + 1
		}
	}
}

func main() {

	if len(os.Args) < 2 {
		fmt.Println("Format : run go tme.go <duree>")
		return
	}
	inputTime, _ := (strconv.Atoi(os.Args[1]))

	//Initialisation des canaux
	cres := make(chan paquet)
	fin := make(chan int)
	client := make(chan string)
	server := make(chan chan paquet)

	//Lancement des goroutines
	for i := 0; i < WORKER; i++ {
		go func() { worker(client, server, cres) }()
	}
	go func() { reader(client) }()
	go func() { serverCompute(server) }()
	go func() { reducer(cres, fin) }()

	time.Sleep(time.Duration(inputTime) * (time.Millisecond))
	//signal de fin des temps ==> préemption du calcul !
	fin <- 0
	moy := <-fin
	fmt.Printf("Durée d'arrêt moyenne des trains : %vsec\n", moy)
}
