#ifndef AE83458B_E680_4E83_B483_C27B8AA4412F
#define AE83458B_E680_4E83_B483_C27B8AA4412F

extern unsigned long get_systick( void );

#define UNITY_CLOCK_MS() get_systick()

#endif /* AE83458B_E680_4E83_B483_C27B8AA4412F */
