class LibNtgJni
{
    // public native void setNumberOfWeights(int N);
    public native String Compute(String command);
    static
    {
	System.loadLibrary("ntgjni");
    }
    public void initParams(int N)
    {
	//this.setNumberOfWeights(N);
    }
    public String calculateMove(String command)
    {
	return Compute(command);
    }
}