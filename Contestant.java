import java.io.*;
import java.util.*;

class Contestant extends NoTippingPlayer 
{
  private static Process ntgCppProcess = null;
  // STDIN for the contestant process.
  private static BufferedWriter ntgCppStdin = null;
  // STDOUT for the contestant process.
  private static BufferedReader ntgCppStdout = null;

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
      System.out.println("Process move by writing to contestant...");
      for(String s : lines)
      {
        System.out.println(s);
        ntgCppStdin.write(s + "\n");
      }
      // The parent framework removes STATE END.
      {
        String s = "STATE END";
        System.out.println(s);
        ntgCppStdin.write(s + "\n");
      }
      ntgCppStdin.flush();
      // Read response.
      System.out.println("Reading move from contestant...");
      move = ntgCppStdout.readLine().trim();
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
      assert(null == ntgCppStdin);
      assert(null == ntgCppStdout);
      try
      {
        System.out.println("Starting contestant process...");
        ntgCppProcess = Runtime.getRuntime().exec("./contestant");
        OutputStream stdin = ntgCppProcess.getOutputStream();
        InputStream stdout = ntgCppProcess.getInputStream();
        ntgCppStdin = new BufferedWriter(new OutputStreamWriter(stdin));
        ntgCppStdout = new BufferedReader(new InputStreamReader(stdout));
        return true;
      }
      catch (Exception e)
      {
        System.out.println("Exception starting contestant: " +
                           e.getMessage());
        return false;
      }
    }
    else
    {
      assert(null != ntgCppStdin);
      assert(null != ntgCppStdout);
      return true;
    }
  }

  public static void killNtgProcess()
  {
    ntgCppProcess.destroy();
    ntgCppStdin = null;
    ntgCppStdout = null;
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
        System.out.println("Error executing contestant.");
      }
    }
    else
    {
      System.out.println("Usage: Contestant.java <port-number>");
    }
  }
}

