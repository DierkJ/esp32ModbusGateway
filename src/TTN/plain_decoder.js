// Decoder for device payload encoder "PLAIN"
// copy&paste to TTN Console -> Applications -> PayloadFormat -> Decoder

function Decoder(bytes, port) 
{
  var decoded = {};

  // Based on https://stackoverflow.com/a/37471538 by Ilya Bursov
  function bytesToFloat(bytes) 
  {
    // JavaScript bitwise operators yield a 32 bits integer, not a float.
    // Assume LSB (least significant byte first).
    var bits = bytes[3]<<24 | bytes[2]<<16 | bytes[1]<<8 | bytes[0];
    var sign = (bits>>>31 === 0) ? 1.0 : -1.0;
    var e = bits>>>23 & 0xff;
    var m = (e === 0) ? (bits & 0x7fffff)<<1 : (bits & 0x7fffff) | 0x800000;
    var f = sign * m * Math.pow(2, e - 150);
    return f;
  }  

  if (port === 1) 
  {
    var i = 0;

    if (bytes.length >= 4) {
      decoded.energy_in = bytesToFloat(bytes.slice(0, 4));
    }     
    if (bytes.length > 4) {
      decoded.energy_out = bytesToFloat(bytes.slice(4, 8));
    }
    if (bytes.length > 8) {
      decoded.power = bytesToFloat(bytes.slice(8, 12));
    }
  }

  return decoded;
}
