.ORIG x0200 ;Double check for NOP for next line
NOP
.FILL x1200 ; Timer interrupt
.FILL x1600; Protection exception 
.FILL x1A00; unaligned access exception
.FILL x1C00; unknown opcode exception
.END