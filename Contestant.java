class Contestant extends NoTippingPlayer 
{
    private LibNtgJni libNtgJni = null;
    private int N=0;
    public Contestant(int port) 
    {
	super(port);
	libNtgJni = new LibNtgJni();
    }
    protected String process(String command) 
    {
	String move = null;
	try 
	{
	   move = libNtgJni.calculateMove(command);
    	} 
	catch (Exception ev) 
	{
	    System.out.println(ev.getMessage());
	}
	return move;			
    }

    public static void main(String[] args) throws Exception 
    {
	if(args.length==1)
	{
	    int port = Integer.parseInt(args[0]);
	    new Contestant(port);
	}
	else
	{
	    System.out.println("Usage: Contestant.java <port-number> <N>");
	}
    }
}