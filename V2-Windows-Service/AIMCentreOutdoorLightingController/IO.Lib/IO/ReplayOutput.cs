namespace IO.Lib.IO
{
    public class ReplayOutput : UsbPortHandler
    {
        bool _isOn;

        // ON/OFF command pairs for relay pins. 
        string[] P1 = ["A00100A1", "A00101A2"];
        string[] P2 = ["A00100A1", "A00101A2"];
        string[] P3 = ["A00100A1", "A00101A2"];

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
            string[] pin = P1;

            if (_isOn) //If PIN is ON
                Write(pin[0]);
            else
                Write(pin[1]);

            _isOn = !_isOn;
        }


    }
}
