using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Runtime.Serialization;
using System.Net;

namespace IO.Lib.Exceptions
{
    public class LightingControllerException : Exception
    {
        public HttpStatusCode HttpStatusCode { get; set; }

        public LightingControllerException() : base() { }
        public LightingControllerException(string message) : base(message) { }
        public LightingControllerException(string message, Exception innerException) : base(message, innerException) { }
    }
}
