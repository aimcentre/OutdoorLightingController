using Microsoft.AspNetCore.Http;
using System.IO.Ports;

namespace IO.Lib.IO
{
    /// <summary>
    /// This class represents the USB Relay module.
    /// </summary>
    public class Relay : SerialPortDevice
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

        public static readonly int CHECK_PIN = 0;
        public static readonly int Lamp1 = 1;
        public static readonly int Lamp2 = 3;
        public static readonly int Lamp3 = 5;
        public static readonly int Lamp4 = 7;

        public Relay(string portName)
            : base(portName, 9600, 8, 25)
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

        public async Task Set(int pin, bool on)
        {
            Console.WriteLine(on ? "ON" : "OFF" + $" pin {pin}");
            await Write(on ? RelayConfigs[pin].OnCommand : RelayConfigs[pin].OffCommand);
        }

        public async Task SetCheckPin(bool on) => await Set(CHECK_PIN, on);
        public async Task SetL1(bool on) => await Set(Lamp1, on);
        public async Task SetL2(bool on) => await Set(Lamp2, on);
        public async Task SetL3(bool on) => await Set(Lamp3, on);
        public async Task SetL4(bool on) => await Set(Lamp4, on);

    }
}
