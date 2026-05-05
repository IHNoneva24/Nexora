#pragma once

struct CharacterData {
    int userId   = 0;
    int gender   = 0;  // 0=male, 1=female
    int skinIdx  = 0;
    int hairIdx  = 0;
    int topIdx   = 0;
    int pantsIdx = 0;  // male only; stored but ignored for female
    int feetIdx  = 0;
};
