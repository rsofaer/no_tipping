import java.io.*;
import java.util.*;

class Contestant extends NoTippingPlayer 
{
  private static Process ntgCppProcess = null;
  // STDIN for the no_tipping_game process.
  private static BufferedReader ntgCppIn = null;
  // STDOUT for the no_tipping_game process.
  private static BufferedWriter ntgCppOut = null;

  public Contestant(int port) 
  {
    // Note that the ctor of NoTippingPlayer is a modal process. Execution does
    // not return to this point until the game is complete.
    super(port);
  }

  protected String process(String command) 
  {
    String move = null;
    try 
    {
      String[] lines = command.split("\n");
      
      // Write to process pipe.
      System.out.println("Pipe to no_tipping_game...");
      for(String s : lines)
      {
        System.out.println(s);
        ntgCppOut.write(s + "\n");
      }
      // The parent framework removes STATE END.
      {
        String s = "STATE END";
        System.out.println(s);
        ntgCppOut.write(s + "\n");
      }
      // Read response.
      System.out.println("Reading move...");
      move = ntgCppIn.readLine().trim();
      System.out.println(move);
      System.out.flush();
    } 
    catch (Exception e) 
    {
      System.out.println("Error performing PIPE operations: " + e.getMessage());
    }
    return move;
  }

  public static boolean openNtgProcess()
  {
    if (null == ntgCppProcess)
    {
      assert(null == ntgCppIn);
      assert(null == ntgCppOut);
      try
      {
        System.out.println("Starting no_tipping_game process...");
        ntgCppProcess = Runtime.getRuntime().exec("./no_tipping_game");
        ntgCppIn = new BufferedReader(new InputStreamReader(ntgCppProcess.getInputStream()));
        ntgCppOut = new BufferedWriter(new OutputStreamWriter(ntgCppProcess.getOutputStream()));
        return true;
      }
      catch (Exception e)
      {
        System.out.println("Exception starting no_tipping_game: " +
                           e.getMessage());
        return false;
      }
    }
    else
    {
      assert(null != ntgCppIn);
      assert(null != ntgCppOut);
      return true;
    }
  }

  public static void killNtgProcess()
  {
    ntgCppProcess.destroy();
    ntgCppIn = null;
    ntgCppOut = null;
    ntgCppProcess = null;
  }

  public static void main(String[] args) throws Exception 
  {
    if (1 == args.length)
    {
      if (openNtgProcess())
      {
        System.out.println("Starting game.");
        int port = Integer.parseInt(args[0]);
        new Contestant(port);
        System.out.println("Game completed.");
        killNtgProcess();
      }
      else
      {
        System.out.println("Error executing no_tipping_game.");
      }
    }
    else
    {
      System.out.println("Usage: Contestant.java <port-number>");
    }
  }
}

