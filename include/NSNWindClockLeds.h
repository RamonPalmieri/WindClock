
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
void LightEen();
void LightTwee();
void LightDrie();
void LightVier();
void LightVijf();
void LightZes();
void LightZeven();
void LightAcht();
void LightNegen();
void LightTien();
void LightElf();
void LightTwaalf();
void LightDertien();
void LightVeertien();
void LightTwintig();
void LightDertig();
void LightVeertig();
void LightVijftig();
void LightKnopen(CRGB color);
void LightRichting(CRGB color);
void LightHoog();
void LightEn();
void LightStatus(CRGB color);


void LightOff()
{
  for (int i = 0; i <= 203; i++) {
    leds[i] =CRGB::Black;
  }
}

void LightEen()
{
  leds[176] = CRGB::Yellow;
  leds[177] = CRGB::Yellow;
  leds[178] = CRGB::Yellow;

  leds[163] = CRGB::Yellow;
  leds[162] = CRGB::Yellow;
  leds[161] = CRGB::Yellow;
}

void LightTwee()
{
  leds[129] = CRGB::Green;
  leds[130] = CRGB::Green;
  leds[131] = CRGB::Green;
  leds[132] = CRGB::Green;
  
}

void LightDrie()
{
  leds[195] = CRGB::Red;
  leds[196] = CRGB::Red;
  leds[197] = CRGB::Red;
}
void LightVier()
{
  leds[160] = CRGB::Red;
  leds[159] = CRGB::Red;
  leds[158] = CRGB::Red;

  leds[145] = CRGB::Red;
  leds[146] = CRGB::Red;
  leds[147] = CRGB::Red;

  leds[124] = CRGB::Red;
  leds[125] = CRGB::Red;
  leds[126] = CRGB::Red;
}
void LightVijf()
{
  leds[85] = CRGB::Blue;
  leds[86] = CRGB::Blue;

  leds[117] = CRGB::Blue;
  leds[118] = CRGB::Blue;

  leds[119] = CRGB::Blue;
  leds[120] = CRGB::Blue;
}
void LightZes()
{
  //leds[161] = CRGB::Blue;
  leds[138] = CRGB::Blue;
  leds[139] = CRGB::Blue;
  leds[140] = CRGB::Blue;
  leds[141] = CRGB::Blue;

  leds[167] = CRGB::Blue;
  leds[166] = CRGB::Blue;
  leds[165] = CRGB::Blue;
  leds[164] = CRGB::Blue;
}
void LightZeven()
{
  leds[194] = CRGB::Purple;
  leds[193] = CRGB::Purple;
  leds[192] = CRGB::Purple;
  leds[191] = CRGB::Purple; 

  leds[179] = CRGB::Purple;
  leds[180] = CRGB::Purple;
  leds[181] = CRGB::Purple;  
  leds[182] = CRGB::Purple;
}
void LightAcht()
{
  leds[203] = CRGB::Orange;
  leds[202] = CRGB::Orange;
  
  leds[170] = CRGB::Orange;
  leds[171] = CRGB::Orange;

  leds[169] = CRGB::Orange;
  leds[168] = CRGB::Orange;
  
  leds[136] = CRGB::Orange;
  leds[137] = CRGB::Orange;

  leds[135] = CRGB::Orange;
  leds[134] = CRGB::Orange;
  
}
void LightNegen()
{
  leds[183] = CRGB::Blue;
  leds[184] = CRGB::Blue;
  leds[185] = CRGB::Blue;
  leds[186] = CRGB::Blue;

  leds[187] = CRGB::Blue;
  leds[188] = CRGB::Blue;
  leds[189] = CRGB::Blue;
  leds[190] = CRGB::Blue;
  
}
void LightTien()
{
  leds[84] = CRGB::Purple;
  leds[83] = CRGB::Purple;
  leds[82] = CRGB::Purple;
  leds[81] = CRGB::Purple;

  leds[51] = CRGB::Purple;
  leds[52] = CRGB::Purple;
  leds[53] = CRGB::Purple;
  leds[54] = CRGB::Purple;
}
void LightElf()
{
  leds[201] = CRGB::Purple;
  leds[200] = CRGB::Purple;
  leds[199] = CRGB::Purple;
  
  leds[172] = CRGB::Purple;
  leds[173] = CRGB::Purple;
  leds[174] = CRGB::Purple;
}
void LightTwaalf()
{
  leds[116] = CRGB::Purple;
  leds[115] = CRGB::Purple;
  leds[114] = CRGB::Purple;
  leds[113] = CRGB::Purple;
  leds[112] = CRGB::Purple;
  leds[111] = CRGB::Purple;

  leds[87] = CRGB::Purple;
  leds[88] = CRGB::Purple;
  leds[89] = CRGB::Purple;
  leds[90] = CRGB::Purple;
  leds[91] = CRGB::Purple;
  leds[92] = CRGB::Purple;
}

void LightDertien()
{
  leds[153] = CRGB::Yellow;
  leds[154] = CRGB::Yellow;
  leds[155] = CRGB::Yellow;
  leds[156] = CRGB::Yellow;
  leds[157] = CRGB::Yellow;

  leds[152] = CRGB::Yellow;
  leds[151] = CRGB::Yellow;
  leds[150] = CRGB::Yellow;
  leds[149] = CRGB::Yellow;
  leds[148] = CRGB::Yellow;
}

void LightVeertien()
{
  leds[102] = CRGB::Red;
  leds[103] = CRGB::Red;
  leds[104] = CRGB::Red;
  leds[105] = CRGB::Red;
  leds[106] = CRGB::Red;
  leds[107] = CRGB::Red;
  leds[108] = CRGB::Red;
  
  leds[101] = CRGB::Red;
  leds[100] = CRGB::Red;
  leds[99] = CRGB::Red;
  leds[98] = CRGB::Red;
  leds[97] = CRGB::Red;
  leds[96] = CRGB::Red;
  leds[95] = CRGB::Red;
}
void LightTwintig()
{
  leds[0] = CRGB::LightGreen;
  leds[1] = CRGB::LightGreen;
  leds[2] = CRGB::LightGreen;
  leds[3] = CRGB::LightGreen;
  leds[4] = CRGB::LightGreen;
  leds[5] = CRGB::LightGreen;
  leds[6] = CRGB::LightGreen;

  leds[33] = CRGB::LightGreen;
  leds[32] = CRGB::LightGreen;
  leds[31] = CRGB::LightGreen;
  leds[30] = CRGB::LightGreen;
  leds[29] = CRGB::LightGreen;
  leds[28] = CRGB::LightGreen;
  leds[27] = CRGB::LightGreen;
  
}
void LightDertig()
{  
  leds[72] = CRGB::Blue;
  leds[73] = CRGB::Blue;
  leds[74] = CRGB::Blue;
  leds[75] = CRGB::Blue;
  leds[76] = CRGB::Blue;
  leds[77] = CRGB::Blue;
  leds[78] = CRGB::Blue;
  leds[79] = CRGB::Blue;
  leds[80] = CRGB::Blue;

  leds[63] = CRGB::Blue;
  leds[62] = CRGB::Blue;
  leds[61] = CRGB::Blue;
  leds[60] = CRGB::Blue;
  leds[59] = CRGB::Blue;
  leds[58] = CRGB::Blue;
  leds[57] = CRGB::Blue;
  leds[56] = CRGB::Blue;
  leds[55] = CRGB::Blue;
  
  leds[38] = CRGB::Blue;
  leds[39] = CRGB::Blue;
  leds[40] = CRGB::Blue;
}
void LightVeertig()
{
  leds[41] = CRGB::Purple;
  leds[42] = CRGB::Purple;
  leds[43] = CRGB::Purple;
  leds[44] = CRGB::Purple;
  leds[45] = CRGB::Purple;
  leds[46] = CRGB::Purple;
  leds[47] = CRGB::Purple;

  leds[26] = CRGB::Purple;
  leds[25] = CRGB::Purple;
  leds[24] = CRGB::Purple;
  leds[23] = CRGB::Purple;
  leds[22] = CRGB::Purple;
  leds[21] = CRGB::Purple;
  leds[20] = CRGB::Purple;
}
void LightVijftig()
{
  leds[68] = CRGB::Yellow;
  leds[69] = CRGB::Yellow;
  leds[70] = CRGB::Yellow;
  leds[71] = CRGB::Yellow;

  leds[67] = CRGB::Yellow;
  leds[66] = CRGB::Yellow;
  leds[65] = CRGB::Yellow;
  leds[64] = CRGB::Yellow;

  leds[34] = CRGB::Yellow;
  leds[35] = CRGB::Yellow;
  leds[36] = CRGB::Yellow;
  leds[37] = CRGB::Yellow;
}
void LightEn()
{
  leds[143] = CRGB::Orange;
  leds[144] = CRGB::Orange;

  leds[128] = CRGB::Orange;
  leds[127] = CRGB::Orange;

  leds[109] = CRGB::Orange;
  leds[110] = CRGB::Orange;

  leds[94] = CRGB::Orange;
  leds[93] = CRGB::Orange;
}

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