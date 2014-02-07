#include <asm.h>

#
#
#  r3  channel

SIGetType:

  cmpwi    %r3,       0         
  bne      NoPad     
  lis      %r3,       0x0900    
  b        end       

NoPad:    
  li       %r3,       0x80      
  b        end       

end:      
  blr     
