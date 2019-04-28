                .386
                .MODEL flat
                .CODE
                LOCALS
                PUBLIC _ReadAnalogJoyPos,_ReadAnalogJoyButtons

_ReadAnalogJoyPos        PROC
                ARG     POS:DWORD,RES:DWORD


                push    ebp
                mov     ebp,esp
                push    ebx
                push    edi
                push    esi


                xor     eax,eax
                xor     esi,esi
                
                mov    ebx,POS
                mov    [ebx],eax
                mov    [ebx+4],eax
                mov    [ebx+8],eax
                mov    [ebx+12],eax
                

                mov     edx,201h
                cli
                out     dx,al        ;start up count
                mov     ecx,RES
                



@@LOOP1:        in      al,dx

                test  al,1111b
                jz    @@DONE
                
                shr   al,1
                adc   [ebx],esi
                shr   al,1
                adc   [ebx+4],esi
                shr   al,1
                adc   [ebx+8],esi
                shr   al,1
                adc   [ebx+12],esi
                
                loop  @@LOOP1


@@DONE:         sti

                pop     esi
                pop     edi
                pop     ebx
                pop     ebp
                ret
_ReadAnalogJoyPos        ENDP


_ReadAnalogJoyButtons    PROC
                push ebp
                mov  ebp,esp
                mov  edx,201h
                xor  eax,eax
                in   al,dx

                shr  eax,4
                xor  al,1111b
                pop  ebp
                ret
_ReadAnalogJoyButtons    ENDP


                END
