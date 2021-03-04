/**
 *      @author : Amine Benslimane
 * 		February 2021,
 * 		Sorbonne Université 
 * 	
 **/
package pc3r.tme1;

public class Compteur{
	private int cp;
	
	public Compteur(int cp) {
		this.cp  = cp;
	}
	
	// manipualtion directe du compteur --> section critique 
	public synchronized void decrementer() {
		this.cp--;
	}
	
	public int getCompteur() {
		return this.cp;
	}
	

	
}