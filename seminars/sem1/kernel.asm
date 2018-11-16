global start

section .text
bits 32
start:
      mov dword [0xb8000], 0x5f615f53          ; print 'Sa' to screen
      mov dword [0xb8004], 0x5f695f62          ; print 'bi' to screen
      mov dword [0xb8008], 0x5f615f6E          ; print 'na' to screen
      mov dword [0xb800C], 0x5f205f73          ; print 's ' to screen
      mov dword [0xb8010], 0x5f535f4F          ; print 'OS' to screen
      hlt
