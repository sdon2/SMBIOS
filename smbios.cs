using System;
using System.Runtime.InteropServices;
using System.Text;

class Program
{
    [DllImport("smbios.dll", CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Ansi)]
    static extern int GetBiosSerial(StringBuilder buffer, int bufferSize);

    static void Main()
    {
        StringBuilder serialBuilder = new StringBuilder(256);
        int success = GetBiosSerial(serialBuilder, serialBuilder.Capacity);

        if (success != 0)
        {
            Console.WriteLine(String.Format("BIOS Serial Number: {0}", serialBuilder));
        }
        else
        {
            Console.WriteLine("Failed to retrieve BIOS serial number.");
        }
    }
}
