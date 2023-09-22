/* Copyright (C)
* 2015 - John Melton, G0ORX/N6LYT,  2016 - Steve Wilson, KA6S
*
*   This program is free software: you can redistribute it and/or modify
*   it under the terms of the GNU General Public License as published by
*   the Free Software Foundation, either version 3 of the License, or
*   (at your option) any later version.
*
*   This program is distributed in the hope that it will be useful,
*   but WITHOUT ANY WARRANTY; without even the implied warranty of
*   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*   GNU General Public License for more details.
*
*   You should have received a copy of the GNU General Public License
*   along with this program.  If not, see <https://www.gnu.org/licenses/>.
*
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "bandstack.h"
#include "band.h"
#include "filter.h"
#include "mode.h"
#include "alex.h"
#include "property.h"
#include "store.h"
#include "store_menu.h"
#include "radio.h"
#include "ext.h"
#include "vfo.h"
#include "message.h"

MEM mem[NUM_OF_MEMORYS];  // This makes it a compile time option

void memSaveState() {
  char name[128];
  char value[128];

  for (int b = 0; b < NUM_OF_MEMORYS; b++) {
    SetPropI1("mem.%d.freqA", b,           mem[b].frequency);
    SetPropI1("mem.%d.mode", b,            mem[b].mode);
    SetPropI1("mem.%d.filter", b,          mem[b].filter);
    SetPropI1("mem.%d.ctcss_enabled", b,   mem[b].ctcss_enabled);
    SetPropI1("mem.%d.ctcss", b,           mem[b].ctcss);
  }
}

void memRestoreState() {
  char name[128];
  char *value;
  t_print("memRestoreState: restore memory\n");

  for (int b = 0; b < NUM_OF_MEMORYS; b++) {
    //
    // Set defaults
    //
    mem[b].frequency = 28010000LL;
    mem[b].mode = modeCWU;
    mem[b].filter = filterF0;
    mem[b].ctcss_enabled = 0;
    mem[b].ctcss = 0;
    //
    // Read from props
    //
    GetPropI1("mem.%d.freqA", b,           mem[b].frequency);
    GetPropI1("mem.%d.mode", b,            mem[b].mode);
    GetPropI1("mem.%d.filter", b,          mem[b].filter);
    GetPropI1("mem.%d.ctcss_enabled", b,   mem[b].ctcss_enabled);
    GetPropI1("mem.%d.ctcss", b,           mem[b].ctcss);
  }
}

void recall_memory_slot(int index) {
  long long new_freq;
  new_freq = mem[index].frequency;
  //
  // Recalling a memory slot is equivalent to the following actions
  //
  // a) set the new frequency via the "Freq" menu
  // b) set the new mode via the "Mode" menu
  // c) set the new filter via the "Filter" menu
  // d) set CTCSS via the "TX" menu
  //
  // Step b) automatically restores the filter, noise reduction, and
  // equalizer settings stored with that mode
  //
  // Step c) will not only change the filter but also store the new setting
  // with that mode.
  //
  // Step d) will only be done if the active receiver controls the TX
  //
  vfo_set_frequency(active_receiver->id, new_freq);
  vfo_mode_changed(mem[index].mode);
  vfo_filter_changed(mem[index].filter);
  if (active_receiver->id == get_tx_vfo() && can_transmit) {
    transmitter_set_ctcss(transmitter, mem[index].ctcss_enabled, mem[index].ctcss);
  }
  g_idle_add(ext_vfo_update, NULL);
}

void store_memory_slot(int index) {
  int id = active_receiver->id;

  //
  // Store current frequency, mode, and filter in slot #index
  //
  if (vfo[id].ctun) {
    mem[index].frequency = vfo[id].ctun_frequency;
  } else {
    mem[index].frequency = vfo[id].frequency;
  }

  mem[index].mode = vfo[id].mode;
  mem[index].filter = vfo[id].filter;
  if (id == get_tx_vfo() && can_transmit) {
    mem[index].ctcss_enabled = transmitter->ctcss_enabled;
    mem[index].ctcss = transmitter->ctcss;
  }
  //memSaveState();
}
