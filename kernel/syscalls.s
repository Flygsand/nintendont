  .section ".text"   
  .align   4         
  .arm    
  .global  thread_create
thread_create:
  .long    0xe6000010
  bx       lr        

  .global  syscall_04
syscall_04:
  .long    0xe6000090
  bx       lr        

  .global  syscall_09
syscall_09:
  .long    0xe6000130
  bx       lr        

  .global  syscall_0a
syscall_0a:
  .long    0xe6000150
  bx       lr        

  .global  syscall_0b
syscall_0b:
  .long    0xe6000170
  bx       lr        

  .global  syscall_0e
syscall_0e:
  .long    0xe60001d0
  bx       lr        

  .global  TimerCreate
TimerCreate:
  .long    0xe6000230
  bx       lr        

  .global  TimerDestroy
TimerDestroy:
  .long    0xe6000290
  bx       lr        

  .global  syscall_18
syscall_18:
  .long    0xe6000310
  bx       lr        

  .global  heap_alloc_aligned
heap_alloc_aligned:
  .long    0xe6000330
  bx       lr        

  .global  syscall_1a
syscall_1a:
  .long    0xe6000350
  bx       lr        

  .global  syscall_1b
syscall_1b:
  .long    0xe6000370
  bx       lr        

  .global  IOS_Open  
IOS_Open: 
  .long    0xe6000390
  bx       lr        

  .global  IOS_Close 
IOS_Close:
  .long    0xe60003b0
  bx       lr        

  .global  IOS_Read  
IOS_Read: 
  .long    0xe60003d0
  bx       lr        

  .global  IOS_Write 
IOS_Write:
  .long    0xe60003f0
  bx       lr        

  .global  IOS_Seek  
IOS_Seek: 
  .long    0xe6000410
  bx       lr        

  .global  IOS_Ioctl 
IOS_Ioctl:
  .long    0xe6000430
  bx       lr        

  .global  IOS_Ioctlv
IOS_Ioctlv:
  .long    0xe6000450
  bx       lr        

  .global  syscall_2b
syscall_2b:
  .long    0xe6000570
  bx       lr        

  .global  syscall_2d
syscall_2d:
  .long    0xe60005b0
  bx       lr        

  .global  syscall_2f
syscall_2f:
  .long    0xe60005f0
  bx       lr        

  .global  syscall_30
syscall_30:
  .long    0xe6000610
  bx       lr        

  .global  sync_before_read
sync_before_read:
  .long    0xe60007f0
  bx       lr        

  .global  sync_after_write
sync_after_write:
  .long    0xe6000810
  bx       lr        

  .global  syscall_41
syscall_41:
  .long    0xE6000830
  bx       lr        

  .global  syscall_4c
syscall_4c:
  .long    0xe6000990
  bx       lr        

  .global  syscall_4d
syscall_4d:
  .long    0xe60009b0
  bx       lr        
          
  .global  syscall_54
syscall_54:
  .long    0xe6000a90
  bx       lr        

  .global  syscall_59
syscall_59:
  .long    0xe6000b30
  bx       lr        

  .global  syscall_5a
syscall_5a:
  .long    0xe6000b50
  bx       lr        

  .global  sha1      
sha1:     
  .long    0xe6000cf0
  bx       lr        

  .global  svc_write 
svc_write:
  mov      r1,        r0        
  mov      r0,        #4
  svc      #0xab
  bx       lr        

  .pool   
  .end    
