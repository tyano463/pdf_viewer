#ifndef __MC_MIDI_H__
#define __MC_MIDI_H__

#include "mc_component.h"

int open_midi(win_attr_t *attr, void *arg, int (*callback)(win_attr_t *, void *));
#endif