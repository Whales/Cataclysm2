#ifndef _SOUND_H_
#define _SOUND_H_

struct Sound
{
  Sound(std::string D = "", int V = 0) : description (D), volume (V) {}
  std::string description;
  int volume;
};

#endif
