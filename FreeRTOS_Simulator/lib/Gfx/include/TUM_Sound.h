#ifndef __TUM_SOUND_H__
#define __TUM_SOUND_H__

#define GEN_ENUM(ENUM)  ENUM,

#define FOR_EACH_SAMPLE(SAMPLE)      \
                        SAMPLE(a3) \
                        SAMPLE(a4) \
                        SAMPLE(a5) \
                        SAMPLE(b3) \
                        SAMPLE(b4) \
                        SAMPLE(c3) \
                        SAMPLE(c4) \
                        SAMPLE(c5) \
                        SAMPLE(d3) \
                        SAMPLE(d4) \
                        SAMPLE(d5) \
                        SAMPLE(e3) \
                        SAMPLE(e4) \
                        SAMPLE(e5) \
                        SAMPLE(f3) \
                        SAMPLE(f4) \
                        SAMPLE(f5) \
                        SAMPLE(g3) \
                        SAMPLE(g4) \
                        SAMPLE(g5) \

enum samples_enum{
    FOR_EACH_SAMPLE(GEN_ENUM)
};

void vInitAudio(char *bin_dir_str);

void vPlaySample(unsigned char index);

#endif
