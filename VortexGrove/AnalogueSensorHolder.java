package VortexGrove;

/**
* VortexGrove/AnalogueSensorHolder.java .
* Generated by the IDL-to-Java compiler (portable), version "3.2"
* from VortexGrove.idl
* Friday, 20 January 2017 17:26:48 o'clock GMT
*/

public final class AnalogueSensorHolder implements org.omg.CORBA.portable.Streamable
{
  public VortexGrove.AnalogueSensor value = null;

  public AnalogueSensorHolder ()
  {
  }

  public AnalogueSensorHolder (VortexGrove.AnalogueSensor initialValue)
  {
    value = initialValue;
  }

  public void _read (org.omg.CORBA.portable.InputStream i)
  {
    value = VortexGrove.AnalogueSensorHelper.read (i);
  }

  public void _write (org.omg.CORBA.portable.OutputStream o)
  {
    VortexGrove.AnalogueSensorHelper.write (o, value);
  }

  public org.omg.CORBA.TypeCode _type ()
  {
    return VortexGrove.AnalogueSensorHelper.type ();
  }

}
