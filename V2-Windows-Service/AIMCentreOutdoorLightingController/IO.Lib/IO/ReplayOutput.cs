namespace IO.Lib.IO
{
    public class ReplayOutput : UsbPortHandler
    {
        private string _data;

        const string P1ON = "A00100A1";
        const string P1OFF = "A00101A2";
        public ReplayOutput(string portName)
            : base(portName, 9600, 8)
        {
            _data = "A00101A2";
        }

        public override bool TryOpen()
        {
            //_comPort.DtrEnable = false;
            return base.TryOpen();
        }

        public void Toggle()
        {
            if (_data == P1ON)
                _data = P1OFF;
            else
                _data = P1ON;

            Write(_data);
        }


    }
}
