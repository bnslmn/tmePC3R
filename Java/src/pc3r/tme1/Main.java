/**
 *      @author : Amine Benslimane
 * 		February 2021,
 * 		Sorbonne Université 
 * 	
 **/


package pc3r.tme1;

public class Main{
	
	public static void main(String[] args) throws InterruptedException {
		
		//déclaration d'un tapis de taille 5
		Tapis tapis = new Tapis(5);
		//déclaration d'une cible de production de 20 items
		Compteur cp = new Compteur(20);
		
		System.out.println("***********************");
		
		//création des threads producteurs
		Thread thp1 = new Thread(
								new Producteur("Pomme", 5, tapis));
		Thread thp2 = new Thread(
								new Producteur("Abricot", 8, tapis));
		Thread thp3 = new Thread(
								new Producteur("Dattes", 7, tapis));
		
		//création des threads consommateurs
		Thread thc1 = new Thread(
								new Consommateur(tapis, cp));
		Thread thc2 = new Thread(
								new Consommateur(tapis, cp));
		Thread thc3 = new Thread(
								new Consommateur(tapis, cp));
		
		// ******** starting producers...  ********
		thp1.start();
		thp2.start();
		thp3.start();

		// ******** starting consumers...  ********
		thc1.start();
		thc2.start();
		thc3.start();
		
		// TQ la cible de production n'est pas atteinte, attendre ;
		
		synchronized(cp) {
			while(cp.getCompteur()>0) {
				try {
					cp.wait();
				} catch (InterruptedException e) {
					e.printStackTrace();
				}
			}
			
		}
		System.out.println("***********************\nExit_With_Success");
		System.exit(0);

	}
}