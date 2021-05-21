/**
 *      @author : Amine Benslimane
 * 		February 2021,
 * 		Sorbonne Université 
 * 	
 **/
package pc3r.tme1;


import java.util.ArrayList;
import java.util.List;

public class Tapis{
	
	private int cap_max;
	private List<Paquet> tapis;
	
	public Tapis(int cap_max) {
		this.cap_max  = cap_max;
		tapis = new ArrayList<Paquet>();
	}
	
	public synchronized void enfiler(Paquet p) throws InterruptedException{
		//si tapis plein, attendre
		while(tapis.size() +1 > cap_max) {
			this.wait();
		}
		//le producteur en attente active se libère et ajoute au tapis, puis prévient les autres
		tapis.add(p);
		
		this.notifyAll();
	}
	
	public synchronized Paquet defiler() throws InterruptedException {
		// si tapis vide, attendre
		while(tapis.isEmpty()) {
			this.wait();
			}
		// retirer le premier élément du tapis (structure de file à ajout en queue et à suppression en tête)
		Paquet tmp = tapis.get(0);
		tapis.remove(0);
		
		this.notifyAll();
		
		return tmp;
	}
}