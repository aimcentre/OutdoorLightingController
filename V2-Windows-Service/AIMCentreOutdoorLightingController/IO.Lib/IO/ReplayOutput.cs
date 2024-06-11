namespace IO.Lib.IO
{
    public class ReplayOutput : UsbPortHandler
    {
        bool _isOn;

        // ON/OFF command pairs for relay pins. 
        static string[] P1  = ["A00100A1", "A00101A2"];
        static string[] P2  = ["A00200A1", "A00201A2"];
        static string[] P3  = ["A00300A1", "A00301A2"];
        static string[] P9  = ["A00900A1", "A00901A2"];
        static string[] P10 = ["A00A00A1", "A00A01A2"];
        static string[] P11 = ["A00B00A1", "A00B01A2"];
        static string[] P12 = ["A00C00A1", "A00C01A2"];
        static string[] P13 = ["A00D00A1", "A00D01A2"];
        static string[] P14 = ["A00E00A1", "A00E01A2"];
        static string[] P15 = ["A00F00A1", "A00F01A2"];
        static string[] P16 = ["A01000A1", "A01001A2"];


        public ReplayOutput(string portName)
            : base(portName, 9600, 8)
        {
            _isOn = false;
        }

        public override bool TryOpen()
        {
            //_comPort.DtrEnable = false;
            return base.TryOpen();
        }

        public void Toggle()
        {
            string[] pin = P16;

            if (_isOn) //If PIN is ON
                Write(pin[0]);
            else
                Write(pin[1]);

            _isOn = !_isOn;
        }


    }
}
