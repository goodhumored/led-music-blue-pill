#define MS_TICKS 3600

void wait(int time) {
  for (int i = 0; i < MS_TICKS * time; i++) __asm__("nop");
}

