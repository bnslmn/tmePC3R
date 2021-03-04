/**
 *      @author : Amine Benslimane
 * 		February 2021,
 * 		Sorbonne Université 
 * 	
 **/

package pc3r.tme1;


public class Consommateur implements Runnable{
	
	private static int compteur = 0;
	private int id;
	private Tapis tapis;
	private Compteur cp;
	
	public Consommateur(Tapis tapis, Compteur cpt) {
		//variable de classe
		compteur++;
		
		this.id = compteur;
		this.tapis = tapis;
		this.cp = cpt;
	}

	@Override
	public void run() {
		
			while(cp.getCompteur() > 0) {
				try {
					Paquet p = tapis.defiler();
					System.out.println("C"+id+ " mange " + p.getName());
					cp.decrementer();
					} catch (InterruptedException e) {
						e.printStackTrace();
														}
										}
						
		
					  }
	
}