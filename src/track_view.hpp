#pragma once
#include <cstdint>

void    enter_track_select();
void    enter_track_select(uint8_t& dst);

void    sprint_track_id(char* dst, int nr);
void    sprint_inst_effect_id(char* dst, int nr);

void    select_track(uint8_t t);
uint8_t selected_track();
void    select_instrument(int i);
int     selected_instrument();
void    select_effect(int e);
int     selected_effect();

void    draw_instrument_cache();
void    draw_effect_cache();
void    draw_track_view();
