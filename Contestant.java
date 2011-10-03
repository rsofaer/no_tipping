import java.io.*;
import LibNtgJni;

class Contestant extends NoTippingPlayer 
{
    LibNtgJni libNtgJni = null;
    Contestant(int port) 
    {
	super(port);
	ntJniWrap = new NoTippingJniWrapper();
    }
    protected String process(String command) 
    {
	try 
	{
	    String move = libNtgJni.calculateMove(command);
    	} 
	catch (Exception ev) 
	{
	    System.out.println(ev.getMessage());
	}
	return move;			
    }

    public static void main(String[] args) throws Exception 
    {
	new Contestant(Integer.parseInt(args[0]));
    }
}
