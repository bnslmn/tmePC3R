/**
 *      @author : Amine Benslimane
 * 		February 2021,
 * 		Sorbonne Université 
 * 	
 **/
package pc3r.tme1;

public class Producteur implements Runnable{
	
	private Tapis tapis;
	private String prod;
	private int target;
	
	public Producteur(String prod, int target, Tapis tapis) {
		this.prod = prod;
		this.target = target;
		this.tapis = tapis;
		
	}

	@Override
	public void run() {
		
		for(int i = 0; i < this.target; i++) {
			try {
				tapis.enfiler(new Paquet(prod + i));
			} catch (InterruptedException e) {
				e.printStackTrace();
			}
		}		
	}
	

}