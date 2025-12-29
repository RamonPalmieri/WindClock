
#include <FastLED.h>

///////////////////////////////////////////////////////////////////////////////////////////
//
// Move a white dot along the strip of leds.  This program simply shows how to configure the leds,
// and then how to turn a single pixel white and then off, moving down the line of pixels.
//

// How many leds are in the strip?
#define NUM_LEDS 204

// For led chips like WS2812, which have a data line, ground, and power, you just
// need to define DATA_PIN.  For led chipsets that are SPI based (four wires - data, clock,
// ground, and power), like the LPD8806 define both DATA_PIN and CLOCK_PIN
// Clock pin only needed for SPI based chipsets when not using hardware SPI

// This is an array of leds.  One item for each led in your strip.
CRGB leds[NUM_LEDS];

void LightOff();

// Number words. Overloads:
// - `LightX()` keeps the original fixed color.
// - `LightX(color)` allows callers to control the color (e.g. one random color for 22).
void LightEen(const CRGB &color);
void LightEen();
void LightTwee(const CRGB &color);
void LightTwee();
void LightDrie(const CRGB &color);
void LightDrie();
void LightVier(const CRGB &color);
void LightVier();
void LightVijf(const CRGB &color);
void LightVijf();
void LightZes(const CRGB &color);
void LightZes();
void LightZeven(const CRGB &color);
void LightZeven();
void LightAcht(const CRGB &color);
void LightAcht();
void LightNegen(const CRGB &color);
void LightNegen();
void LightTien(const CRGB &color);
void LightTien();
void LightElf(const CRGB &color);
void LightElf();
void LightTwaalf(const CRGB &color);
void LightTwaalf();
void LightDertien(const CRGB &color);
void LightDertien();
void LightVeertien(const CRGB &color);
void LightVeertien();
void LightTwintig(const CRGB &color);
void LightTwintig();
void LightDertig(const CRGB &color);
void LightDertig();
void LightVeertig(const CRGB &color);
void LightVeertig();
void LightVijftig(const CRGB &color);
void LightVijftig();
void LightEn(const CRGB &color);
void LightEn();

void LightKnopen(CRGB color);
void LightRichting(CRGB color);
void LightHoog(CRGB color);
void LightStatus(CRGB color);

void LightOff()
{
  for (int i = 0; i <= 203; i++) {
    leds[i] = CRGB::Black;
  }
}

void LightEen(const CRGB &color)
{
  leds[176] = color;
  leds[177] = color;
  leds[178] = color;

  leds[163] = color;
  leds[162] = color;
  leds[161] = color;
}

void LightEen() { LightEen(CRGB::Yellow); }

void LightTwee(const CRGB &color)
{
  leds[129] = color;
  leds[130] = color;
  leds[131] = color;
  leds[132] = color;
}

void LightTwee() { LightTwee(CRGB::Green); }

void LightDrie(const CRGB &color)
{
  leds[195] = color;
  leds[196] = color;
  leds[197] = color;
}

void LightDrie() { LightDrie(CRGB::Red); }

void LightVier(const CRGB &color)
{
  leds[160] = color;
  leds[159] = color;
  leds[158] = color;

  leds[145] = color;
  leds[146] = color;
  leds[147] = color;

  leds[124] = color;
  leds[125] = color;
  leds[126] = color;
}

void LightVier() { LightVier(CRGB::Red); }

void LightVijf(const CRGB &color)
{
  leds[85] = color;
  leds[86] = color;

  leds[117] = color;
  leds[118] = color;

  leds[119] = color;
  leds[120] = color;
}

void LightVijf() { LightVijf(CRGB::Blue); }

void LightZes(const CRGB &color)
{
  //leds[161] = color;
  leds[138] = color;
  leds[139] = color;
  leds[140] = color;
  leds[141] = color;

  leds[167] = color;
  leds[166] = color;
  leds[165] = color;
  leds[164] = color;
}

void LightZes() { LightZes(CRGB::Blue); }

void LightZeven(const CRGB &color)
{
  leds[194] = color;
  leds[193] = color;
  leds[192] = color;
  leds[191] = color;

  leds[179] = color;
  leds[180] = color;
  leds[181] = color;
  leds[182] = color;
}

void LightZeven() { LightZeven(CRGB::Purple); }

void LightAcht(const CRGB &color)
{
  leds[203] = color;
  leds[202] = color;

  leds[170] = color;
  leds[171] = color;

  leds[169] = color;
  leds[168] = color;

  leds[136] = color;
  leds[137] = color;

  leds[135] = color;
  leds[134] = color;
}

void LightAcht() { LightAcht(CRGB::Orange); }

void LightNegen(const CRGB &color)
{
  leds[183] = color;
  leds[184] = color;
  leds[185] = color;
  leds[186] = color;

  leds[187] = color;
  leds[188] = color;
  leds[189] = color;
  leds[190] = color;
}

void LightNegen() { LightNegen(CRGB::Blue); }

void LightTien(const CRGB &color)
{
  leds[84] = color;
  leds[83] = color;
  leds[82] = color;
  leds[81] = color;

  leds[51] = color;
  leds[52] = color;
  leds[53] = color;
  leds[54] = color;
}

void LightTien() { LightTien(CRGB::Purple); }

void LightElf(const CRGB &color)
{
  leds[201] = color;
  leds[200] = color;
  leds[199] = color;

  leds[172] = color;
  leds[173] = color;
  leds[174] = color;
}

void LightElf() { LightElf(CRGB::Purple); }

void LightTwaalf(const CRGB &color)
{
  leds[116] = color;
  leds[115] = color;
  leds[114] = color;
  leds[113] = color;
  leds[112] = color;
  leds[111] = color;

  leds[87] = color;
  leds[88] = color;
  leds[89] = color;
  leds[90] = color;
  leds[91] = color;
  leds[92] = color;
}

void LightTwaalf() { LightTwaalf(CRGB::Purple); }

void LightDertien(const CRGB &color)
{
  leds[153] = color;
  leds[154] = color;
  leds[155] = color;
  leds[156] = color;
  leds[157] = color;

  leds[152] = color;
  leds[151] = color;
  leds[150] = color;
  leds[149] = color;
  leds[148] = color;
}

void LightDertien() { LightDertien(CRGB::Yellow); }

void LightVeertien(const CRGB &color)
{
  leds[102] = color;
  leds[103] = color;
  leds[104] = color;
  leds[105] = color;
  leds[106] = color;
  leds[107] = color;
  leds[108] = color;

  leds[101] = color;
  leds[100] = color;
  leds[99] = color;
  leds[98] = color;
  leds[97] = color;
  leds[96] = color;
  leds[95] = color;
}

void LightVeertien() { LightVeertien(CRGB::Red); }

void LightTwintig(const CRGB &color)
{
  leds[0] = color;
  leds[1] = color;
  leds[2] = color;
  leds[3] = color;
  leds[4] = color;
  leds[5] = color;
  leds[6] = color;

  leds[33] = color;
  leds[32] = color;
  leds[31] = color;
  leds[30] = color;
  leds[29] = color;
  leds[28] = color;
  leds[27] = color;
}

void LightTwintig() { LightTwintig(CRGB::LightGreen); }

void LightDertig(const CRGB &color)
{
  leds[72] = color;
  leds[73] = color;
  leds[74] = color;
  leds[75] = color;
  leds[76] = color;
  leds[77] = color;
  leds[78] = color;
  leds[79] = color;
  leds[80] = color;

  leds[63] = color;
  leds[62] = color;
  leds[61] = color;
  leds[60] = color;
  leds[59] = color;
  leds[58] = color;
  leds[57] = color;
  leds[56] = color;
  leds[55] = color;

  leds[38] = color;
  leds[39] = color;
  leds[40] = color;
}

void LightDertig() { LightDertig(CRGB::Blue); }

void LightVeertig(const CRGB &color)
{
  leds[41] = color;
  leds[42] = color;
  leds[43] = color;
  leds[44] = color;
  leds[45] = color;
  leds[46] = color;
  leds[47] = color;

  leds[26] = color;
  leds[25] = color;
  leds[24] = color;
  leds[23] = color;
  leds[22] = color;
  leds[21] = color;
  leds[20] = color;
}

void LightVeertig() { LightVeertig(CRGB::Purple); }

void LightVijftig(const CRGB &color)
{
  leds[68] = color;
  leds[69] = color;
  leds[70] = color;
  leds[71] = color;

  leds[67] = color;
  leds[66] = color;
  leds[65] = color;
  leds[64] = color;

  leds[34] = color;
  leds[35] = color;
  leds[36] = color;
  leds[37] = color;
}

void LightVijftig() { LightVijftig(CRGB::Yellow); }

void LightEn(const CRGB &color)
{
  leds[143] = color;
  leds[144] = color;

  leds[128] = color;
  leds[127] = color;

  leds[109] = color;
  leds[110] = color;

  leds[94] = color;
  leds[93] = color;
}

void LightEn() { LightEn(CRGB::Orange); }

void LightHoog(CRGB color)
{
  leds[121] = color;
  leds[122] = color;
  leds[123] = color;
}

void LightKnopen(CRGB color)
{
  leds[16] = color;
  leds[15] = color;
  leds[14] = color;
  leds[13] = color;
  leds[12] = color;
  leds[11] = color;
  leds[10] = color;
  leds[9] = color;
  leds[8] = color;
  leds[7] = color;
}

void LightRichting(CRGB color)
{
  leds[48] = color;
  leds[49] = color;
  leds[50] = color;

  leds[19] = color;
  leds[18] = color;
  leds[17] = color;
}

void LightStatus(CRGB color)
{
  leds[198] = color;
  leds[175] = color;
}
