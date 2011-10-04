class LibNtgJni
{
    public native String Compute(String command);
    static
    {
	System.loadLibrary("ntgjni");
    }
    public String calculateMove(String command)
    {
	return this.Compute(command);
    }
}
