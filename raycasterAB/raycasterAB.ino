// raycasterAB
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

#include <Arduboy2.h>
#include <FixedPoints.h>
#include <FixedPointsCommon.h>

struct Door
{
  int16_t mapIndex;
  uint8_t state;
  SQ15x16 offset;
  uint8_t tics;
};

Arduboy2 arduboy;

const SQ15x16 PROGMEM sinTable[64] =
{
  0.0122715, 0.0368072, 0.0613207, 0.0857973, 0.110222, 0.134581, 0.158858, 0.18304, 0.207111, 0.231058, 0.254866, 0.27852, 0.302006, 0.32531, 0.348419, 0.371317,
  0.393992, 0.41643, 0.438616, 0.460539, 0.482184, 0.503538, 0.52459, 0.545325, 0.565732, 0.585798, 0.605511, 0.624859, 0.643832, 0.662416, 0.680601, 0.698376,
  0.715731, 0.732654, 0.749136, 0.765167, 0.780737, 0.795837, 0.810457, 0.824589, 0.838225, 0.851355, 0.863973, 0.87607, 0.88764, 0.898674, 0.909168, 0.919114,
  0.928506, 0.937339, 0.945607, 0.953306, 0.960431, 0.966976, 0.97294, 0.978317, 0.983105, 0.987301, 0.990903, 0.993907, 0.996313, 0.998118, 0.999322, 0.999925
};

const SQ15x16 PROGMEM tanTable[64] =
{
  0.0122725, 0.0368322, 0.0614364, 0.0861149, 0.110898, 0.135816, 0.160901, 0.186185, 0.211702, 0.237484, 0.26357, 0.289995, 0.316799, 0.344023, 0.37171, 0.399908,
  0.428665, 0.458034, 0.48807, 0.518835, 0.550394, 0.582817, 0.616182, 0.650571, 0.686077, 0.722799, 0.760848, 0.800345, 0.841426, 0.884239, 0.928952, 0.975753,
  1.02485, 1.07648, 1.13092, 1.18846, 1.24946, 1.31432, 1.38351, 1.45756, 1.53711, 1.6229, 1.7158, 1.81688, 1.92739, 2.04889, 2.18325, 2.33282,
  2.50057, 2.69027, 2.90679, 3.15658, 3.44834, 3.79406, 4.2108, 4.72363, 5.37099, 6.21499, 7.36289, 9.0173, 11.6124, 16.277, 27.1502, 81.4832
};

const SQ15x16 heightToDistanceTable[32] =
{
  1, 1.03226, 1.06667, 1.10345, 1.14286, 1.18519, 1.23077, 1.28, 1.33333, 1.3913, 1.45455, 1.52381, 1.6, 1.68421, 1.77778, 1.88235,
  2, 2.13333, 2.28571, 2.46154, 2.66667, 2.90909, 3.2, 3.55556, 4, 4.57143, 5.33333, 6.4, 8, 10.6667, 16, 32
};

const uint8_t wallData[16] =
{
  0xFF, 0x80, 0xC0, 0xE0, 0xF0, 0xF8, 0xFC, 0xFE, 0xFF, 0x01, 0x03, 0x07, 0x0F, 0x1F, 0x3F, 0x7F
};

const uint8_t mapWidth = 20;
const uint8_t mapHeight = 20;

const uint8_t PROGMEM mapData[mapWidth * mapHeight] =
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
  arduboy.begin();
  arduboy.setFrameRate(30);
}

SQ15x16 SinLookup(uint_fast8_t index)
{
  SQ15x16 value;
  memcpy_P(&value, &sinTable[index], sizeof(SQ15x16));
  return value;
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
  SQ15x16 value;
  memcpy_P(&value, &tanTable[index], sizeof(SQ15x16));
  return value;
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

uint8_t FindHeight(SQ15x16 d)
{
  int8_t l = 0;
  int8_t r = 31;

  while (l <= r)
  {
    int8_t m = l + (r - l) / 2;

    if (heightToDistanceTable[m] == d)
      return 64 - 2 * m;

    if (heightToDistanceTable[m] < d)
      l = m + 1;
    else
      r = m - 1;
  }

  if (r < 0)
    return 64;

  return 64 - 2 * r;
}

void DrawWallSlice(uint8_t *p, uint8_t h, bool ic)
{
  if (h > 64)
    h = 64;

  uint8_t hh = h / 2;
  uint8_t c = hh - 1;
  c /= 8;
  uint8_t s = 3 - c;
  uint8_t i = hh % 8;
  uint8_t u;
  uint8_t m;
  uint8_t l;
  uint8_t u2;
  uint8_t l2;
  uint8_t bg = 0xAA;
  uint8_t bg2 = 0x55;

  if (ic)
  {
    u = bg & ~wallData[i];
    m = ~0xFF;
    l = bg & ~wallData[i + 8];
    u2 = bg2 & ~wallData[i];
    l2 = bg2 & ~wallData[i + 8];
  }
  else
  {
    u = bg | wallData[i];
    m = 0xFF;
    l = bg | wallData[i + 8];
    u2 = bg2 | wallData[i];
    l2 = bg2 | wallData[i + 8];
  }
  
  switch (s)
  {
    case 3:
      *p = bg;
      *(p + 1) = bg2;
      p += WIDTH;
    case 2:
      *p = bg;
      *(p + 1) = bg2;
      p += WIDTH;
    case 1:
      *p = bg;
      *(p + 1) = bg2;
      p += WIDTH;
    default:
      break;
  }

  *p = u;
  *(p + 1) = u2;
  p += WIDTH;

  switch (c)
  {
    case 3:
      *p = *(p + 1) = m;
      p += WIDTH;
      *p = *(p + 1) = m;
      p += WIDTH;
    case 2:
      *p = *(p + 1) = m;
      p += WIDTH;
      *p = *(p + 1) = m;
      p += WIDTH;
    case 1:
      *p = *(p + 1) = m;
      p += WIDTH;
      *p = *(p + 1) = m;
      p += WIDTH;
    default:
      break;
  }

  *p = l;
  *(p + 1) = l2;
  p += WIDTH;

  switch (s)
  {
    case 3:
      *p = bg;
      *(p + 1) = bg2;
      p += WIDTH;
    case 2:
      *p = bg;
      *(p + 1) = bg2;
      p += WIDTH;
    case 1:
      *p = bg;
      *(p + 1) = bg2;
      p += WIDTH;
    default:
      break;
  }
}

void Update()
{
  oldCameraX = cameraX;
  oldCameraY = cameraY;

  if (arduboy.pressed(UP_BUTTON))
  {
    cameraX += 0.133333 * Cos(cameraAngle);
    cameraY -= 0.133333 * Sin(cameraAngle);
  }

  if (arduboy.pressed(DOWN_BUTTON))
  {
    cameraX -= 0.133333 * Cos(cameraAngle);
    cameraY += 0.133333 * Sin(cameraAngle);
  }

  if (arduboy.pressed(LEFT_BUTTON + B_BUTTON))
  {
    cameraX += 0.133333 * Cos(cameraAngle + 64);
    cameraY -= 0.133333 * Sin(cameraAngle + 64);
  }
  else if (arduboy.pressed(LEFT_BUTTON))
  {
    cameraAngle += 4;
  }

  if (arduboy.pressed(RIGHT_BUTTON + B_BUTTON))
  {
    cameraX -= 0.133333 * Cos(cameraAngle + 64);
    cameraY += 0.133333 * Sin(cameraAngle + 64);
  }
  else if (arduboy.pressed(RIGHT_BUTTON))
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
  else if (pgm_read_byte(&mapData[ty * mapWidth + txp]) == 1 || pgm_read_byte(&mapData[ty * mapWidth + txm]) == 1)
    cameraX = oldCameraX;
  else
  {
    if (pgm_read_byte(&mapData[typ * mapWidth + tx]) == 1)
      cameraY = typ - 0.140625;

    if (pgm_read_byte(&mapData[tym * mapWidth + tx]) == 1)
      cameraY = tym + 1.140625;
  }

  if (cameraY - 0.140625 < 0 || (cameraY + 0.140625).getInteger() >= mapHeight)
    cameraY = oldCameraY;
  else if (pgm_read_byte(&mapData[typ * mapWidth + tx]) == 1 || pgm_read_byte(&mapData[tym * mapWidth + tx]) == 1)
    cameraY = oldCameraY;
  else
  {
    if (pgm_read_byte(&mapData[ty * mapWidth + txp]) == 1)
      cameraX = txp - 0.140625;

    if (pgm_read_byte(&mapData[ty * mapWidth + txm]) == 1)
      cameraX = txm + 1.140625;
  }

  int16_t mapIndex = (ty - 1) * mapWidth + tx;

  if (pgm_read_byte(&mapData[mapIndex]) == 2)
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

  if (pgm_read_byte(&mapData[mapIndex]) == 2)
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

  if (pgm_read_byte(&mapData[mapIndex]) == 2)
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

  if (pgm_read_byte(&mapData[mapIndex]) == 2)
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
  uint8_t *p = arduboy.sBuffer;

  uint8_t rayAngle = cameraAngle + 31;

  for (uint8_t i = 0; i < 128; i += 2)
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

        horizontalIntersectionType = pgm_read_byte(&mapData[gridY * mapWidth + gridX]);
        
        if (horizontalIntersectionType == 1)
        {
          horizontalIntersectionDistance = (horizontalIntersectionX - cameraX) * Cos(cameraAngle) - (horizontalIntersectionY - cameraY) * Sin(cameraAngle);
          break;
        }
        else if (horizontalIntersectionType == 2 && ((64 * (horizontalIntersectionX + 0.5 * stepX)).getInteger() % 64) < (doors[(gridY % 8) * 8 + (gridX % 8)].mapIndex == (gridY * mapWidth + gridX) ? doors[(gridY % 8) * 8 + (gridX % 8)].offset.getInteger() : 64))
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

        verticalIntersectionType = pgm_read_byte(&mapData[gridY * mapWidth + gridX]);
        
        if (verticalIntersectionType == 1)
        {
          verticalIntersectionDistance = (verticalIntersectionX - cameraX) * Cos(cameraAngle) - (verticalIntersectionY - cameraY) * Sin(cameraAngle);
          break;
        }
        else if (verticalIntersectionType == 2 && ((64 * (verticalIntersectionY + 0.5 * stepY)).getInteger() % 64) < (doors[(gridY % 8) * 8 + (gridX % 8)].mapIndex == (gridY * mapWidth + gridX) ? doors[(gridY % 8) * 8 + (gridX % 8)].offset.getInteger() : 64))
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
    bool invertColor;

    if (horizontalIntersectionDistance < verticalIntersectionDistance)
    {
      distance = horizontalIntersectionDistance;
      invertColor = false;
    }
    else
    {
      distance = verticalIntersectionDistance;
      invertColor = true;
    }

    uint8_t wallHeight = FindHeight(distance);
    
    DrawWallSlice(p, wallHeight, invertColor);
 
    rayAngle--;

    p += 2;
  }
}

void ShowText()
{
  arduboy.clear();
  arduboy.setCursor(37, 20);
  arduboy.print(F("Raycaster"));
  arduboy.setCursor(10, 28);
  arduboy.print(F("by John D. Corrado"));
  arduboy.setCursor(7, 36);
  arduboy.print(F("Press A to continue"));
}

void loop()
{
  if (!(arduboy.nextFrame()))
    return;

  if (showText)
  {
    ShowText();

    arduboy.pollButtons();

    if (arduboy.justPressed(A_BUTTON))
      showText = false;
  }
  else
  {
    Update();
    Render();
  }

  arduboy.display();
}
