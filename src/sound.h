/* date = June 22nd 2026 7:47 pm */

#ifndef SOUND_H
#define SOUND_H

#define MAX_SOUNDS 10


enum SoundType
{
    PLAYER_MOVE_SOUND,
    PLAYER_PUSH_SOUND,
    PLAYER_PROJ_SOUND,
    BLOCK_MOVE_SOUND,
    BLOCK_PROJ_SOUND,
    DOOR_OPEN_SOUND,
    STAR_COLLECT_SOUND,
    SOUND_COUNT,
};

char * audioSources[SOUND_COUNT] = 
{
    "Assets/Sounds/SlimeWalk.wav",
    "",
    "",
    "",
    "",
    "",
    ""
};

struct SoundData
{
     uint32 currentSound;
    Sound soundArray[MAX_SOUNDS];
};

struct AudioData
{
    SoundData soundDatas[SOUND_COUNT];
    };

   SoundData LoadSoundData(SoundType soundType)
{
    SoundData sound = { 0 };
    
    sound.currentSound = 0;
    sound.soundArray[0] = LoadSound(audioSources[soundType]);
    for (uint32 i = 1; i < MAX_SOUNDS; i++) sound.soundArray[i] = LoadSoundAlias(sound.soundArray[0]);
    
    return sound;
    }

AudioData LoadAudioData()
{
    AudioData data = { 0 };
    
    // TODO: Find rest of the sound
    data.soundDatas[PLAYER_MOVE_SOUND] = LoadSoundData(PLAYER_MOVE_SOUND);
    
    return data;
    
}

void PlayClip(AudioData * audio, SoundType soundType, real32 pitchVariation = 0.0f)
{
    SoundData & sound = audio->soundDatas[soundType];
    
    real32 min = 1 - pitchVariation;
    real32 max = 1 + pitchVariation;
    
    real32 rand = (real32)GetRandomValue(0, RAND_MAX) / RAND_MAX;
    real32 pitch = min + (max - min) * rand;
    SM_TRACE("rand %.2f, pitch %.2f", rand, pitch);
    
    SetSoundPitch(sound.soundArray[sound.currentSound], pitch);
    PlaySound(sound.soundArray[sound.currentSound]);
    sound.currentSound++;
    if (sound.currentSound >= MAX_SOUNDS) sound.currentSound = 0;
}

#endif //SOUND_H
