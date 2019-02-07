// raycasterGBsp
// Copyright(C) 2019 John D. Corrado
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.

#include <Gamebuino-Meta.h>
#define FIXED_POINTS_NO_RANDOM
#include <FixedPoints.h>
#include <FixedPointsCommon.h>

struct Door
{
  int16_t mapIndex;
  uint8_t state;
  SQ15x16 offset;
  uint8_t tics;
};

const SQ15x16 sinTable[64] =
{
  0.0122715, 0.0368072, 0.0613207, 0.0857973, 0.110222, 0.134581, 0.158858, 0.18304, 0.207111, 0.231058, 0.254866, 0.27852, 0.302006, 0.32531, 0.348419, 0.371317,
  0.393992, 0.41643, 0.438616, 0.460539, 0.482184, 0.503538, 0.52459, 0.545325, 0.565732, 0.585798, 0.605511, 0.624859, 0.643832, 0.662416, 0.680601, 0.698376,
  0.715731, 0.732654, 0.749136, 0.765167, 0.780737, 0.795837, 0.810457, 0.824589, 0.838225, 0.851355, 0.863973, 0.87607, 0.88764, 0.898674, 0.909168, 0.919114,
  0.928506, 0.937339, 0.945607, 0.953306, 0.960431, 0.966976, 0.97294, 0.978317, 0.983105, 0.987301, 0.990903, 0.993907, 0.996313, 0.998118, 0.999322, 0.999925
};

const SQ15x16 tanTable[64] =
{
  0.0122725, 0.0368322, 0.0614364, 0.0861149, 0.110898, 0.135816, 0.160901, 0.186185, 0.211702, 0.237484, 0.26357, 0.289995, 0.316799, 0.344023, 0.37171, 0.399908,
  0.428665, 0.458034, 0.48807, 0.518835, 0.550394, 0.582817, 0.616182, 0.650571, 0.686077, 0.722799, 0.760848, 0.800345, 0.841426, 0.884239, 0.928952, 0.975753,
  1.02485, 1.07648, 1.13092, 1.18846, 1.24946, 1.31432, 1.38351, 1.45756, 1.53711, 1.6229, 1.7158, 1.81688, 1.92739, 2.04889, 2.18325, 2.33282,
  2.50057, 2.69027, 2.90679, 3.15658, 3.44834, 3.79406, 4.2108, 4.72363, 5.37099, 6.21499, 7.36289, 9.0173, 11.6124, 16.277, 27.1502, 81.4832
};

const SQ15x16 heightToDistanceTable[256] =
{
  0.0625, 0.0627451, 0.0629921, 0.0632411, 0.0634921, 0.063745, 0.064, 0.064257, 0.0645161, 0.0647773, 0.0650407, 0.0653061, 0.0655738, 0.0658436, 0.0661157, 0.06639,
  0.0666667, 0.0669456, 0.0672269, 0.0675105, 0.0677966, 0.0680851, 0.0683761, 0.0686695, 0.0689655, 0.0692641, 0.0695652, 0.069869, 0.0701754, 0.0704846, 0.0707965, 0.0711111,
  0.0714286, 0.0717489, 0.0720721, 0.0723982, 0.0727273, 0.0730594, 0.0733945, 0.0737327, 0.0740741, 0.0744186, 0.0747664, 0.0751174, 0.0754717, 0.0758294, 0.0761905, 0.076555,
  0.0769231, 0.0772947, 0.0776699, 0.0780488, 0.0784314, 0.0788177, 0.0792079, 0.079602, 0.08, 0.080402, 0.0808081, 0.0812183, 0.0816327, 0.0820513, 0.0824742, 0.0829016,
  0.0833333, 0.0837696, 0.0842105, 0.0846561, 0.0851064, 0.0855615, 0.0860215, 0.0864865, 0.0869565, 0.0874317, 0.0879121, 0.0883978, 0.0888889, 0.0893855, 0.0898876, 0.0903955,
  0.0909091, 0.0914286, 0.091954, 0.0924855, 0.0930233, 0.0935673, 0.0941176, 0.0946746, 0.0952381, 0.0958084, 0.0963855, 0.0969697, 0.097561, 0.0981595, 0.0987654, 0.0993789,
  0.1, 0.100629, 0.101266, 0.101911, 0.102564, 0.103226, 0.103896, 0.104575, 0.105263, 0.10596, 0.106667, 0.107383, 0.108108, 0.108844, 0.109589, 0.110345,
  0.111111, 0.111888, 0.112676, 0.113475, 0.114286, 0.115108, 0.115942, 0.116788, 0.117647, 0.118519, 0.119403, 0.120301, 0.121212, 0.122137, 0.123077, 0.124031,
  0.125, 0.125984, 0.126984, 0.128, 0.129032, 0.130081, 0.131148, 0.132231, 0.133333, 0.134454, 0.135593, 0.136752, 0.137931, 0.13913, 0.140351, 0.141593,
  0.142857, 0.144144, 0.145455, 0.146789, 0.148148, 0.149533, 0.150943, 0.152381, 0.153846, 0.15534, 0.156863, 0.158416, 0.16, 0.161616, 0.163265, 0.164948,
  0.166667, 0.168421, 0.170213, 0.172043, 0.173913, 0.175824, 0.177778, 0.179775, 0.181818, 0.183908, 0.186047, 0.188235, 0.190476, 0.192771, 0.195122, 0.197531,
  0.2, 0.202532, 0.205128, 0.207792, 0.210526, 0.213333, 0.216216, 0.219178, 0.222222, 0.225352, 0.228571, 0.231884, 0.235294, 0.238806, 0.242424, 0.246154,
  0.25, 0.253968, 0.258065, 0.262295, 0.266667, 0.271186, 0.275862, 0.280702, 0.285714, 0.290909, 0.296296, 0.301887, 0.307692, 0.313725, 0.32, 0.326531,
  0.333333, 0.340426, 0.347826, 0.355556, 0.363636, 0.372093, 0.380952, 0.390244, 0.4, 0.410256, 0.421053, 0.432432, 0.444444, 0.457143, 0.470588, 0.484848,
  0.5, 0.516129, 0.533333, 0.551724, 0.571429, 0.592593, 0.615385, 0.64, 0.666667, 0.695652, 0.727273, 0.761905, 0.8, 0.842105, 0.888889, 0.941176,
  1, 1.06667, 1.14286, 1.23077, 1.33333, 1.45455, 1.6, 1.77778, 2, 2.28571, 2.66667, 3.2, 4, 5.33333, 8, 16
};

const SQ15x16 scalarTable[256] =
{
  0.125, 0.12549, 0.125984, 0.126482, 0.126984, 0.12749, 0.128, 0.128514, 0.129032, 0.129555, 0.130081, 0.130612, 0.131148, 0.131687, 0.132231, 0.13278,
  0.133333, 0.133891, 0.134454, 0.135021, 0.135593, 0.13617, 0.136752, 0.137339, 0.137931, 0.138528, 0.13913, 0.139738, 0.140351, 0.140969, 0.141593, 0.142222,
  0.142857, 0.143498, 0.144144, 0.144796, 0.145455, 0.146119, 0.146789, 0.147465, 0.148148, 0.148837, 0.149533, 0.150235, 0.150943, 0.151659, 0.152381, 0.15311,
  0.153846, 0.154589, 0.15534, 0.156098, 0.156863, 0.157635, 0.158416, 0.159204, 0.16, 0.160804, 0.161616, 0.162437, 0.163265, 0.164103, 0.164948, 0.165803,
  0.166667, 0.167539, 0.168421, 0.169312, 0.170213, 0.171123, 0.172043, 0.172973, 0.173913, 0.174863, 0.175824, 0.176796, 0.177778, 0.178771, 0.179775, 0.180791,
  0.181818, 0.182857, 0.183908, 0.184971, 0.186047, 0.187135, 0.188235, 0.189349, 0.190476, 0.191617, 0.192771, 0.193939, 0.195122, 0.196319, 0.197531, 0.198758,
  0.2, 0.201258, 0.202532, 0.203822, 0.205128, 0.206452, 0.207792, 0.20915, 0.210526, 0.211921, 0.213333, 0.214765, 0.216216, 0.217687, 0.219178, 0.22069,
  0.222222, 0.223776, 0.225352, 0.22695, 0.228571, 0.230216, 0.231884, 0.233577, 0.235294, 0.237037, 0.238806, 0.240602, 0.242424, 0.244275, 0.246154, 0.248062,
  0.25, 0.251969, 0.253968, 0.256, 0.258065, 0.260163, 0.262295, 0.264463, 0.266667, 0.268908, 0.271186, 0.273504, 0.275862, 0.278261, 0.280702, 0.283186,
  0.285714, 0.288288, 0.290909, 0.293578, 0.296296, 0.299065, 0.301887, 0.304762, 0.307692, 0.31068, 0.313725, 0.316832, 0.32, 0.323232, 0.326531, 0.329897,
  0.333333, 0.336842, 0.340426, 0.344086, 0.347826, 0.351648, 0.355556, 0.359551, 0.363636, 0.367816, 0.372093, 0.376471, 0.380952, 0.385542, 0.390244, 0.395062,
  0.4, 0.405063, 0.410256, 0.415584, 0.421053, 0.426667, 0.432432, 0.438356, 0.444444, 0.450704, 0.457143, 0.463768, 0.470588, 0.477612, 0.484848, 0.492308,
  0.5, 0.507937, 0.516129, 0.52459, 0.533333, 0.542373, 0.551724, 0.561404, 0.571429, 0.581818, 0.592593, 0.603774, 0.615385, 0.627451, 0.64, 0.653061,
  0.666667, 0.680851, 0.695652, 0.711111, 0.727273, 0.744186, 0.761905, 0.780488, 0.8, 0.820513, 0.842105, 0.864865, 0.888889, 0.914286, 0.941176, 0.969697,
  1, 1.03226, 1.06667, 1.10345, 1.14286, 1.18519, 1.23077, 1.28, 1.33333, 1.3913, 1.45455, 1.52381, 1.6, 1.68421, 1.77778, 1.88235,
  2, 2.13333, 2.28571, 2.46154, 2.66667, 2.90909, 3.2, 3.55556, 4, 4.57143, 5.33333, 6.4, 8, 10.6667, 16, 32
};

#include "textures.h"

const uint8_t mapWidth = 20;
const uint8_t mapHeight = 20;

const uint8_t mapData[mapWidth * mapHeight] =
{
  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
  1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1,
  1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1,
  1, 0, 0, 1, 1, 2, 1, 1, 0, 0, 0, 0, 1, 1, 2, 1, 1, 0, 0, 1,
  1, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 1,
  1, 0, 0, 2, 0, 0, 0, 2, 0, 0, 0, 0, 2, 0, 0, 0, 2, 0, 0, 1,
  1, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 1,
  1, 0, 0, 1, 1, 2, 1, 1, 0, 0, 0, 0, 1, 1, 2, 1, 1, 0, 0, 1,
  1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1,
  1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1,
  1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1,
  1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1,
  1, 0, 0, 1, 1, 2, 1, 1, 0, 0, 0, 0, 1, 1, 2, 1, 1, 0, 0, 1,
  1, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 1,
  1, 0, 0, 2, 0, 0, 0, 2, 0, 0, 0, 0, 2, 0, 0, 0, 2, 0, 0, 1,
  1, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 1,
  1, 0, 0, 1, 1, 2, 1, 1, 0, 0, 0, 0, 1, 1, 2, 1, 1, 0, 0, 1,
  1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1,
  1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1,
  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1
};

Door doors[64] =
{
  { -1, 0, 64, 0 }, { -1, 0, 64, 0 }, { -1, 0, 64, 0 }, { -1, 0, 64, 0 }, { -1, 0, 64, 0 }, { -1, 0, 64, 0 }, { -1, 0, 64, 0 }, { -1, 0, 64, 0 },
  { -1, 0, 64, 0 }, { -1, 0, 64, 0 }, { -1, 0, 64, 0 }, { -1, 0, 64, 0 }, { -1, 0, 64, 0 }, { -1, 0, 64, 0 }, { -1, 0, 64, 0 }, { -1, 0, 64, 0 },
  { -1, 0, 64, 0 }, { -1, 0, 64, 0 }, { -1, 0, 64, 0 }, { -1, 0, 64, 0 }, { -1, 0, 64, 0 }, { -1, 0, 64, 0 }, { -1, 0, 64, 0 }, { -1, 0, 64, 0 },
  { -1, 0, 64, 0 }, { -1, 0, 64, 0 }, { -1, 0, 64, 0 }, { -1, 0, 64, 0 }, { -1, 0, 64, 0 }, { -1, 0, 64, 0 }, { -1, 0, 64, 0 }, { -1, 0, 64, 0 },
  { -1, 0, 64, 0 }, { -1, 0, 64, 0 }, { -1, 0, 64, 0 }, { -1, 0, 64, 0 }, { -1, 0, 64, 0 }, { -1, 0, 64, 0 }, { -1, 0, 64, 0 }, { -1, 0, 64, 0 },
  { -1, 0, 64, 0 }, { -1, 0, 64, 0 }, { -1, 0, 64, 0 }, { -1, 0, 64, 0 }, { -1, 0, 64, 0 }, { -1, 0, 64, 0 }, { -1, 0, 64, 0 }, { -1, 0, 64, 0 },
  { -1, 0, 64, 0 }, { -1, 0, 64, 0 }, { -1, 0, 64, 0 }, { -1, 0, 64, 0 }, { -1, 0, 64, 0 }, { -1, 0, 64, 0 }, { -1, 0, 64, 0 }, { -1, 0, 64, 0 },
  { -1, 0, 64, 0 }, { -1, 0, 64, 0 }, { -1, 0, 64, 0 }, { -1, 0, 64, 0 }, { -1, 0, 64, 0 }, { -1, 0, 64, 0 }, { -1, 0, 64, 0 }, { -1, 0, 64, 0 }
};

SQ15x16 cameraX = 5.5;
SQ15x16 cameraY = 10.5;
uint8_t cameraAngle = 64;
SQ15x16 oldCameraX;
SQ15x16 oldCameraY;

bool showText = true;

void setup()
{
  gb.begin();
  gb.setFrameRate(30);
}

SQ15x16 SinLookup(uint_fast8_t index)
{
  return sinTable[index];
}

SQ15x16 Sin(uint8_t brads)
{
  const uint8_t quadrant = ((brads & 0xC0) >> 6);
  const uint8_t index = ((brads & 0x3F) >> 0);
  switch (quadrant)
  {
  case 0: return SinLookup(index);
  case 1: return SinLookup(63 - index);
  case 2: return -SinLookup(index);
  case 3: return -SinLookup(63 - index);
  default: return 0;
  }
}

SQ15x16 Cos(uint16_t brads)
{
  const uint8_t quadrant = ((brads & 0xC0) >> 6);
  const uint8_t index = ((brads & 0x3F) >> 0);
  switch (quadrant)
  {
  case 0: return SinLookup(63 - index);
  case 1: return -SinLookup(index);
  case 2: return -SinLookup(63 - index);
  case 3: return SinLookup(index);
  default: return 0;
  }
}

SQ15x16 TanLookup(uint_fast8_t index)
{
  return tanTable[index];
}

SQ15x16 Tan(uint8_t brads)
{
  const uint8_t quadrant = ((brads & 0xC0) >> 6);
  const uint8_t index = ((brads & 0x3F) >> 0);
  switch (quadrant)
  {
  case 0: case 2: return TanLookup(index);
  case 1: case 3: return -TanLookup(63 - index);
  default: return 0;
  }
}

SQ15x16 Cot(uint16_t brads)
{
  const uint8_t quadrant = ((brads & 0xC0) >> 6);
  const uint8_t index = ((brads & 0x3F) >> 0);
  switch (quadrant)
  {
  case 0: case 2: return TanLookup(63 - index);
  case 1: case 3: return -TanLookup(index);
  default: return 0;
  }
}

uint16_t FindHeight(SQ15x16 d)
{
  int16_t l = 0;
  int16_t r = 255;

  while (l <= r)
  {
    int16_t m = l + (r - l) / 2;

    if (heightToDistanceTable[m] == d)
      return 512 - 2 * m;

    if (heightToDistanceTable[m] < d)
      l = m + 1;
    else
      r = m - 1;
  }

  if (r < 0)
    return 512;

  return 512 - 2 * r;
}

void DrawWallSlice(uint16_t *p, const uint16_t *texture, uint8_t textureOffsetX, uint16_t wallHeight)
{
  int16_t wallStart = (32 - wallHeight) / 2;
  uint16_t count;
  SQ15x16 textureOffsetY;
  SQ15x16 scalar = scalarTable[(512 - wallHeight) / 2];
  texture = &texture[textureOffsetX * 64];

  if (wallHeight < 32)
  {
    count = wallStart;
    
    do
    {
      *p = static_cast<uint16_t>(DARKGRAY);
      p += gb.display.width();
    } while (--count);
  }
  
  if (wallStart < 0)
  {
    count = 32;
    textureOffsetY = -wallStart * scalar;
  }
  else
  {
    count = wallHeight;
    textureOffsetY = 0;
  }

  do
  {
    *p = texture[textureOffsetY.getInteger()];
    p += gb.display.width();
    textureOffsetY += scalar;
  } while (--count);

  if (wallHeight < 32)
  {
    count = wallStart;

    do
    {
      *p = static_cast<uint16_t>(GRAY);
      p += gb.display.width();
    } while(--count);
  }
}

void Update()
{
  oldCameraX = cameraX;
  oldCameraY = cameraY;

  if (gb.buttons.repeat(BUTTON_UP, 0))
  {
    cameraX += 0.133333 * Cos(cameraAngle);
    cameraY -= 0.133333 * Sin(cameraAngle);
  }

  if (gb.buttons.repeat(BUTTON_DOWN, 0))
  {
    cameraX -= 0.133333 * Cos(cameraAngle);
    cameraY += 0.133333 * Sin(cameraAngle);
  }

  if (gb.buttons.repeat(BUTTON_LEFT, 0) && gb.buttons.repeat(BUTTON_B, 0))
  {
    cameraX += 0.133333 * Cos(cameraAngle + 64);
    cameraY -= 0.133333 * Sin(cameraAngle + 64);
  }
  else if (gb.buttons.repeat(BUTTON_LEFT, 0))
  {
    cameraAngle += 4;
  }

  if (gb.buttons.repeat(BUTTON_RIGHT, 0) && gb.buttons.repeat(BUTTON_B, 0))
  {
    cameraX -= 0.133333 * Cos(cameraAngle + 64);
    cameraY += 0.133333 * Sin(cameraAngle + 64);
  }
  else if (gb.buttons.repeat(BUTTON_RIGHT, 0))
  {
    cameraAngle -= 4;
  }
  
  uint8_t tx = cameraX.getInteger();
  uint8_t txm = (cameraX - 0.140625).getInteger();
  uint8_t txp = (cameraX + 0.140625).getInteger();
  uint8_t ty = cameraY.getInteger();
  uint8_t tym = (cameraY - 0.140625).getInteger();
  uint8_t typ = (cameraY + 0.140625).getInteger();
  
  if (cameraX - 0.140625 < 0 || (cameraX + 0.140625).getInteger() >= mapWidth)
    cameraX = oldCameraX;
  else if (mapData[ty * mapWidth + txp] == 1 || mapData[ty * mapWidth + txm] == 1)
    cameraX = oldCameraX;
  else
  {
    if (mapData[typ * mapWidth + tx] == 1)
      cameraY = typ - 0.140625;

    if (mapData[tym * mapWidth + tx] == 1)
      cameraY = tym + 1.140625;
  }

  if (cameraY - 0.140625 < 0 || (cameraY + 0.140625).getInteger() >= mapHeight)
    cameraY = oldCameraY;
  else if (mapData[typ * mapWidth + tx] == 1 || mapData[tym * mapWidth + tx] == 1)
    cameraY = oldCameraY;
  else
  {
    if (mapData[ty * mapWidth + txp] == 1)
      cameraX = txp - 0.140625;

    if (mapData[ty * mapWidth + txm] == 1)
      cameraX = txm + 1.140625;
  }

  int16_t mapIndex = (ty - 1) * mapWidth + tx;

  if (mapData[mapIndex] == 2)
  {
    Door *door = &doors[((ty - 1) % 8) * 8 + (tx % 8)];

    if (door->mapIndex != mapIndex)
    {
      door->mapIndex = mapIndex;
      door->state = 0;
      door->offset = 64;
      door->tics = 0;
    }

    if (door->state == 0 || door->state == 1)
    {
      if (door->mapIndex == (ty * mapWidth + txp) || door->mapIndex == (ty * mapWidth + txm))
        cameraX = oldCameraX;

      if (door->mapIndex == (typ * mapWidth + tx) || door->mapIndex == (tym * mapWidth + tx))
        cameraY = oldCameraY;
    }
  }

  mapIndex = (ty + 1) * mapWidth + tx;

  if (mapData[mapIndex] == 2)
  {
    Door *door = &doors[((ty + 1) % 8) * 8 + (tx % 8)];

    if (door->mapIndex != mapIndex)
    {
      door->mapIndex = mapIndex;
      door->state = 0;
      door->offset = 64;
      door->tics = 0;
    }

    if (door->state == 0 || door->state == 1)
    {
      if (door->mapIndex == (ty * mapWidth + txp) || door->mapIndex == (ty * mapWidth + txm))
        cameraX = oldCameraX;

      if (door->mapIndex == (typ * mapWidth + tx) || door->mapIndex == (tym * mapWidth + tx))
        cameraY = oldCameraY;
    }
  }

  mapIndex = ty * mapWidth + (tx - 1);

  if (mapData[mapIndex] == 2)
  {
    Door *door = &doors[(ty % 8) * 8 + ((tx - 1) % 8)];

    if (door->mapIndex != mapIndex)
    {
      door->mapIndex = mapIndex;
      door->state = 0;
      door->offset = 64;
      door->tics = 0;
    }

    if (door->state == 0 || door->state == 1)
    {
      if (door->mapIndex == (ty * mapWidth + txp) || door->mapIndex == (ty * mapWidth + txm))
        cameraX = oldCameraX;

      if (door->mapIndex == (typ * mapWidth + tx) || door->mapIndex == (tym * mapWidth + tx))
        cameraY = oldCameraY;
    }
  }

  mapIndex = ty * mapWidth + (tx + 1);

  if (mapData[mapIndex] == 2)
  {
    Door *door = &doors[(ty % 8) * 8 + ((tx + 1) % 8)];

    if (door->mapIndex != mapIndex)
    {
      door->mapIndex = mapIndex;
      door->state = 0;
      door->offset = 64;
      door->tics = 0;
    }

    if (door->state == 0 || door->state == 1)
    {
      if (door->mapIndex == (ty * mapWidth + txp) || door->mapIndex == (ty * mapWidth + txm))
        cameraX = oldCameraX;

      if (door->mapIndex == (typ * mapWidth + tx) || door->mapIndex == (tym * mapWidth + tx))
        cameraY = oldCameraY;
    }
  }

  for (uint8_t i = 0; i < 64; i++)
  {
    Door *door = &doors[i];

    if (door->mapIndex != -1)
    {
      if (door->state == 0)
      {
        door->tics++;

        if (door->tics == 30)
        {
          door->state = 1;
          door->tics = 0;
        }
      }
      else if (door->state == 1)
      {
        door->offset -= 4.266667;

        if (door->offset < 0)
        {
          door->state = 2;
          door->offset = 0;
        }
      }
      else if (door->state == 2)
      {
        if (door->mapIndex != (ty * mapWidth + tx))
        {
          door->tics++;

          if (door->tics == 30)
          {
            door->state = 3;
            door->tics = 0;
          }
        }
      }
      else if (door->state == 3)
      {
        door->offset += 4.266667;

        if (door->offset > 64)
        {
          door->mapIndex = -1;
          door->state = 0;
          door->offset = 64;
        }
      }
    }
  }
}

void Render()
{
  uint16_t *p = &gb.display._buffer[(gb.display.height() - 32) / 2 * gb.display.width() + (gb.display.width() - 64) / 2];

  uint8_t rayAngle = cameraAngle + 31;

  for (uint8_t i = 0; i < 64; i++)
  {
    SQ15x16 horizontalIntersectionY;
    SQ15x16 stepY;

    if (rayAngle < 128)
    {
      horizontalIntersectionY = floorFixed(cameraY);
      stepY = -1;
    }
    else
    {
      horizontalIntersectionY = floorFixed(cameraY) + 1;
      stepY = 1;
    }

    SQ15x16 horizontalIntersectionX = cameraX - (horizontalIntersectionY - cameraY) * Cot(rayAngle);
    SQ15x16 stepX = -stepY * Cot(rayAngle);
    SQ15x16 horizontalIntersectionDistance;
    uint8_t horizontalIntersectionType;
    uint8_t horizontalDoorOffset;

    if (rayAngle == 0 || rayAngle == 128)
      horizontalIntersectionDistance = SQ15x16::MaxValue;
    else
    {
      while (1)
      {
        int8_t gridX = horizontalIntersectionX.getInteger();
        int8_t gridY = horizontalIntersectionY.getInteger() - (stepY < 0 ? 1 : 0);

        if (gridX < 0 || gridY < 0 || gridX >= mapWidth || gridY >= mapHeight)
        {
          horizontalIntersectionDistance = SQ15x16::MaxValue;
          break;
        }

        horizontalIntersectionType = mapData[gridY * mapWidth + gridX];
        
        if (horizontalIntersectionType == 1)
        {
          horizontalIntersectionDistance = (horizontalIntersectionX - cameraX) * Cos(cameraAngle) - (horizontalIntersectionY - cameraY) * Sin(cameraAngle);
          break;
        }
        else if (horizontalIntersectionType == 2 && ((64 * (horizontalIntersectionX + 0.5 * stepX)).getInteger() % 64) < (horizontalDoorOffset = (doors[(gridY % 8) * 8 + (gridX % 8)].mapIndex == (gridY * mapWidth + gridX) ? doors[(gridY % 8) * 8 + (gridX % 8)].offset.getInteger() : 64)))
        {
          horizontalIntersectionX += 0.5 * stepX;
          horizontalIntersectionY += 0.5 * stepY;
          horizontalIntersectionDistance = (horizontalIntersectionX - cameraX) * Cos(cameraAngle) - (horizontalIntersectionY - cameraY) * Sin(cameraAngle);
          break;
        }

        horizontalIntersectionX += stepX;
        horizontalIntersectionY += stepY;
      }
    }

    SQ15x16 verticalIntersectionX;

    if (rayAngle >= 64 && rayAngle < 192)
    {
      verticalIntersectionX = floorFixed(cameraX);
      stepX = -1;
    }
    else
    {
      verticalIntersectionX = floorFixed(cameraX) + 1;
      stepX = 1;
    }

    SQ15x16 verticalIntersectionY = cameraY - (verticalIntersectionX - cameraX) * Tan(rayAngle);
    stepY = -stepX * Tan(rayAngle);
    SQ15x16 verticalIntersectionDistance;
    uint8_t verticalIntersectionType;
    uint8_t verticalDoorOffset;

    if (rayAngle == 64 || rayAngle == 192)
      verticalIntersectionDistance = SQ15x16::MaxValue;
    else
    {
      while (1)
      {
        int8_t gridX = verticalIntersectionX.getInteger() - (stepX < 0 ? 1 : 0);
        int8_t gridY = verticalIntersectionY.getInteger();

        if (gridX < 0 || gridY < 0 || gridX >= mapWidth || gridY >= mapHeight)
        {
          verticalIntersectionDistance = SQ15x16::MaxValue;
          break;
        }

        verticalIntersectionType = mapData[gridY * mapWidth + gridX];
        
        if (verticalIntersectionType == 1)
        {
          verticalIntersectionDistance = (verticalIntersectionX - cameraX) * Cos(cameraAngle) - (verticalIntersectionY - cameraY) * Sin(cameraAngle);
          break;
        }
        else if (verticalIntersectionType == 2 && ((64 * (verticalIntersectionY + 0.5 * stepY)).getInteger() % 64) < (verticalDoorOffset = (doors[(gridY % 8) * 8 + (gridX % 8)].mapIndex == (gridY * mapWidth + gridX) ? doors[(gridY % 8) * 8 + (gridX % 8)].offset.getInteger() : 64)))
        {
          verticalIntersectionX += 0.5 * stepX;
          verticalIntersectionY += 0.5 * stepY;
          verticalIntersectionDistance = (verticalIntersectionX - cameraX) * Cos(cameraAngle) - (verticalIntersectionY - cameraY) * Sin(cameraAngle);
          break;
        }

        verticalIntersectionX += stepX;
        verticalIntersectionY += stepY;
      }
    }

    SQ15x16 distance;
    const uint16_t *texture;
    uint8_t textureOffsetX;

    if (horizontalIntersectionDistance < verticalIntersectionDistance)
    {
      distance = horizontalIntersectionDistance;
      texture = wall1Data;
      textureOffsetX = (64 * horizontalIntersectionX).getInteger() % 64;

      if (horizontalIntersectionType == 2)
      {
        texture = door1Data;
        textureOffsetX += 64 - horizontalDoorOffset;
      }

      if (horizontalIntersectionType != 2 && rayAngle > 128)
        textureOffsetX = 63 - textureOffsetX;
    }
    else
    {
      distance = verticalIntersectionDistance;
      texture = wall2Data;
      textureOffsetX = (64 * verticalIntersectionY).getInteger() % 64;

      if (verticalIntersectionType == 2)
      {
        texture = door2Data;
        textureOffsetX += 64 - verticalDoorOffset;
      }

      if (verticalIntersectionType != 2 && rayAngle >= 64 && rayAngle < 192)
        textureOffsetX = 63 - textureOffsetX;
    }

    uint16_t wallHeight = FindHeight(distance);
    
    DrawWallSlice(p, texture, textureOffsetX, wallHeight);

    rayAngle--;

    p++;
  }
}

void ShowText()
{
  gb.display.clear();
  gb.display.setCursor((gb.display.width() - 9 * gb.display.getFontWidth()) / 2, (gb.display.height() - 3 * gb.display.getFontHeight()) / 2);
  gb.display.print("Raycaster");
  gb.display.setCursor((gb.display.width() - 18 * gb.display.getFontWidth()) / 2, gb.display.getCursorY() + gb.display.getFontHeight());
  gb.display.print("by John D. Corrado");
  gb.display.setCursor((gb.display.width() - 19 * gb.display.getFontWidth()) / 2, gb.display.getCursorY() + gb.display.getFontHeight());
  gb.display.print("Press A to continue");
}

void loop()
{
  while (!gb.update());

  if (showText)
  {
    ShowText();
    
    if (gb.buttons.pressed(BUTTON_A))
    {
      gb.display.clear();
      showText = false;
    }
  }
  else
  {
    Update();
    Render();
  }
}
