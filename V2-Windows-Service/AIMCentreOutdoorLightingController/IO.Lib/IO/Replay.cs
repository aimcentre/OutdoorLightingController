using System.IO.Ports;

namespace IO.Lib.IO
{
    public class Replay : UsbPortHandler
    {
        public static RelayConfig[] RelayConfigs =
            [
                new RelayConfig("A00100A1", "A00101A2"),
                new RelayConfig("A00200A1", "A00201A2"),
                new RelayConfig("A00300A1", "A00301A2"),
                new RelayConfig("A00400A1", "A00401A2"),
                new RelayConfig("A00500A1", "A00501A2"),
                new RelayConfig("A00600A1", "A00601A2"),
                new RelayConfig("A00700A1", "A00701A2"),
                new RelayConfig("A00800A1", "A00801A2"),
                new RelayConfig("A00900A1", "A00901A2"),
                new RelayConfig("A00A00A1", "A00A01A2"),
                new RelayConfig("A00B00A1", "A00B01A2"),
                new RelayConfig("A00C00A1", "A00C01A2"),
                new RelayConfig("A00D00A1", "A00D01A2"),
                new RelayConfig("A00E00A1", "A00E01A2"),
                new RelayConfig("A00F00A1", "A00F01A2"),
                new RelayConfig("A01000A1", "A01001A2")
            ];

        public Replay(string portName)
            : base(portName, 9600, 8, 100)
        {
        }

        public override bool TryOpen()
        {
            return base.TryOpen();
        }

        public async Task Toggle(int relayNumber)
        {
            RelayConfig relay = RelayConfigs[relayNumber];

            if (relay.IsOn)
                await Write(relay.OffCommand);
            else
                await Write(relay.OnCommand);

            relay.IsOn = !relay.IsOn;
        }

        public async Task TurnAll(bool on)
        {
            Console.WriteLine(on ? "ON" : "OFF");

            foreach (var relay in RelayConfigs)
                await Write(on ? relay.OnCommand : relay.OffCommand);
        }

    }
}
