#include <fastnoise/fastnoise.h>

float z_init = (float)rand() / (float)RAND_MAX;

void scaleData(float* data, int width, int height) {
  float min = NULL;
  float max = NULL;
  for (int i = 0; i < width * height; i++) {
    if (data[i] < min || min == NULL) {
      min = data[i];
    }
    if (data[i] > max || max == NULL) {
      max = data[i];
    }
  }
  for (int i = 0; i < width * height; i++) {
    data[i] = (data[i] - min) / (max - min);
  }
}

float* perlinCrossSection(int width, int height, float scale, float z) {
  FastNoiseLite noise;
  noise.SetNoiseType(FastNoiseLite::NoiseType_OpenSimplex2);

  float* noiseData = new float[width * height];
  int index = 0;

  for (int y = 0; y < height; y++) {
    for (int x = 0; x < width; x++) {
      float x_ = (float)x * scale;
      float y_ = (float)y * scale;
      noiseData[index++] = noise.GetNoise(x_, y_, (float)z + z_init);
    }
  }
  scaleData(noiseData, width, height);
  return noiseData;
}
